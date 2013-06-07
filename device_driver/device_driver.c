/*
 * TEAM8 Encrypting Disk Driver | CS411 - Project 3
 * 
 * Based on sample simple block driver found here:
 * http://blog.superpat.com/2010/05/04/a-simple-block-driver-for-linux-kernel-2-6-31/
 *
 * Encryption based heavily on implementation here:
 * http://www.emerham.com/media/class/CS/CS%20411/proj4/osurd.c
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h> 
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>

/*
 * Cryptography
 */
#include <linux/crypto.h>
#include <linux/scatterlist.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TEAM8 Project 3");
MODULE_AUTHOR("Ryley Herrington, Steven Reid, William Leslie");
MODULE_ALIAS("team8");

static int major_num = 0;
module_param(major_num, int, 0);

/*
 * Size of disk to be created consists of logical_block_size * nsectors
 */
static int logical_block_size = 512;
module_param(logical_block_size, int, 0);
static int nsectors = 1024; 
module_param(nsectors, int, 0);

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SIZE 512

/*
 * Our request queue.
 */
static struct request_queue *Queue;

/*
 * Struct to hold our device info.
 */
static struct team8_device {
	unsigned long size;
	spinlock_t lock;
	u8 *data;
	struct gendisk *gd;
} Device;

/*
 * Handle an I/O request.
 */
static void team8_transfer(struct team8_device *dev, sector_t sector,
	unsigned long nsect, char *buffer, int write) {
	unsigned long offset = sector * logical_block_size;
	unsigned long nbytes = nsect * logical_block_size;
	
	if ((offset + nbytes) > dev->size) {
		printk ("TEAM 8 : Beyond-end write -- this should not happen\n");
		return;
	}

	if (write) 
		memcpy(dev->data + offset, buffer, nbytes);
	else
		memcpy(buffer, dev->data + offset, nbytes);
}

static int team8_encrypt(char *input, int input_length, int write_flag) {
	char*  algorithm = "ecb(aes)";
	char   keyval[32];	

	/* 
	 * Setting Key
	 */
	int i;
	for (i = 0; i<32; i++){
		keyval[i] = 0;
	}
	keyval[0] = 'O';
	keyval[1] = 'S';
	keyval[2] = 'U';
	
	struct crypto_ablkcipher *tfm;
	struct ablkcipher_request *req;
	struct completion full_struct;
	struct scatterlist sg[8];
	int    ret;

	/*
	 * This is the seed value for encryption
	 */
	char crypt_initial_val = 0x44;

	init_completion(&full_struct);

	tfm = crypto_alloc_ablkcipher(algorithm, 0, 0);
	if (IS_ERR(tfm)){
		printk("TEAM8: Failed allocating tfm\n");
		goto out;
	}

	req = ablkcipher_request_alloc(tfm, GFP_KERNEL);
	if (IS_ERR(req)){
		printk("TEAM8: Failed allocating req\n");
		goto out;
	}	
	
	ret = crypto_ablkcipher_setkey(tfm, keyval, 32);
	if (ret < 0) {
		printk("TEAM8: Failed to set key\n");
		goto out;
	}

	sg_set_buf(&sg[0], input, input_length);
	ablkcipher_request_set_crypt(req, sg, sg, input_length, crypt_initial_val);

	if (write_flag)
		ret = crypto_ablkcipher_encrypt(req);
	else
		ret = crypto_ablkcipher_decrypt(req);

	/*
	 * We're turning a asynchronous function back into synchronous...
	 */
	switch (ret) {
	case 0:
		break;
	case -EINPROGRESS:
	case -EBUSY:
		ret = wait_for_completion_interruptible(&full_struct);
		if (!ret) {
			INIT_COMPLETION(full_struct);
			break;
		}
	default:
		printk("TEAM8: failed to complete: %d\n", ret);
		ret = -1;
		goto out;
	}
	ret = 0;
out:
	crypto_free_ablkcipher(tfm);
	ablkcipher_request_free(req);

	return ret;
}

static void team8_request(struct request_queue *q) {
	struct request *req;
	char* total_area=NULL;
	int total_size=0;

	req = blk_fetch_request(q);
	while (req != NULL) {

		/*
		 * Don't handle non-command requests
		 */
		if (req == NULL || (req->cmd_type != REQ_TYPE_FS)) {
			__blk_end_request_all(req, -EIO);
			continue;
		}
		
		/*
		 * Make room for encryption
		 */	
		total_size = blk_rq_cur_sectors(req) * KERNEL_SECTOR_SIZE;
		total_area = kmalloc(total_size, GFP_KERNEL | GFP_ATOMIC);

		/*
		 * Writing when rq_data_dir = 1
		 */
		if (rq_data_dir(req)){ 
			printk("TEAM8 : Writing in request\n");
			/*
			 * copy data to a new place for encryption
			 */
			printk("TEAM8: First 3 bytes raw: %x%x%x\n",req->buffer[0],req->buffer[1], req->buffer[2]);
			memcpy(total_area, req->buffer, total_size);
			/* 
			 * encrypt
			 */
			team8_encrypt(total_area, total_size,1);
			printk("TEAM8: First 3 bytes enc: %x%x%x\n",total_area[0],total_area[1], total_area[2]);
			/*
			 * transfer to disk
			 */
			team8_transfer(&Device, 
				       blk_rq_pos(req),
				       blk_rq_cur_sectors(req),
				       total_area,
				       1);
		}	
		else {
			/*
			 * transfer from disk
			 */
			printk("TEAM8 : Reading in request\n");
			team8_transfer(&Device,
				       blk_rq_pos(req),
				       blk_rq_cur_sectors(req),
				       total_area,
				       0);
			/*
			 * decrypt
			 */
			printk("TEAM8: First 3 bytes enc: %x%x%x\n",total_area[0],total_area[1],total_area[2]);
			team8_encrypt(total_area, total_size,0);
			memcpy(req->buffer, total_area, total_size);
			printk("TEAM8: First 3 bytes raw: %x%x%x\n",req->buffer[0],req->buffer[1], req->buffer[2]);
		}
		kfree(total_area);	

		if ( ! __blk_end_request_cur(req, 0) ) {
			req = blk_fetch_request(q);
		}
	}
}

/*
 * The HDIO_GETGEO ioctl is handled in blkdev_ioctl(), which
 * calls this. We need to implement getgeo, since we can't
 * use tools such as fdisk to partition the drive otherwise.
 */
int team8_getgeo(struct block_device * block_device, struct hd_geometry * geo) {
	long size;

	/* 
	 * We have no real geometry, of course, so make something up.
	 */
	size = Device.size * (logical_block_size / KERNEL_SECTOR_SIZE);
	geo->cylinders = (size & ~0x3f) >> 6;
	geo->heads = 4;
	geo->sectors = 16;
	geo->start = 0;
	return 0;
}

/*
 * The device operations structure.
 */
static struct block_device_operations team8_ops = {
		.owner  = THIS_MODULE,
		.getgeo = team8_getgeo
};

static int __init team8_init(void) {
	/*
	 * Set up our internal device.
	 */
	Device.size = nsectors * logical_block_size;
	spin_lock_init(&Device.lock);
	Device.data = vmalloc(Device.size);
	if (Device.data == NULL)
		return -ENOMEM;
	/*
	 * Get a request queue.
	 */
	Queue = blk_init_queue(team8_request, &Device.lock);
	if (Queue == NULL)
		goto out;
	blk_queue_logical_block_size(Queue, logical_block_size);
	/*
	 * Get registered.
	 */
	major_num = register_blkdev(major_num, "team8");
	if (major_num < 0) {
		printk("TEAM8 : unable to get major number\n");
		goto out;
	}
	/*
	 * And the gendisk structure.
	 */
	Device.gd = alloc_disk(16);
	if (!Device.gd)
		goto out_unregister;
	Device.gd->major = major_num;
	Device.gd->first_minor = 0;
	Device.gd->fops = &team8_ops;
	Device.gd->private_data = &Device;
	strcpy(Device.gd->disk_name, "team80");
	set_capacity(Device.gd, nsectors);
	Device.gd->queue = Queue;
	add_disk(Device.gd);

	return 0;

out_unregister:
	unregister_blkdev(major_num, "team8");
out:
	vfree(Device.data);
	return -ENOMEM;
}

static void __exit team8_exit(void)
{
	del_gendisk(Device.gd);
	put_disk(Device.gd);
	unregister_blkdev(major_num, "team8");
	blk_cleanup_queue(Queue);
	vfree(Device.data);
}

module_init(team8_init);
module_exit(team8_exit);
