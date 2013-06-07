/*
 * sstf 
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct sstf_data {
	struct list_head queue;
	sector_t last_sector; 
	struct list_head* next_to_dispatch;
	
	int queue_count;
};

static int sstf_dispatch(struct request_queue *q, int force)
{
	printk("In dispatch.\n");
	struct sstf_data *nd = q->elevator->elevator_data;

	if (!list_empty(&nd->queue)) {
		struct request *rq;
		rq = list_entry(nd->next_to_dispatch, struct request, queuelist);

		if (rq == 0) {
			printk("failed");
			return 0;
		}

		if (nd->queue_count == 1) {
			printk("Only one item in queue, no calcs needed.\n");
			list_del_init(&rq->queuelist);
			nd->queue_count--;
		}

		else {
			printk("More than one item in queue to dispatch. \n");
			//Gets the pointers to the requests that we want
			struct request* curr_req = list_entry(nd->next_to_dispatch, struct request, queuelist);
			struct request* prev_req = list_entry(nd->next_to_dispatch->prev, struct request, queuelist);
			struct request* next_req = list_entry(nd->next_to_dispatch->next, struct request, queuelist);
		
			//Gets the sectors	
			unsigned long the_curr = (unsigned long)blk_rq_pos(curr_req);
			unsigned long the_prev = (unsigned long)blk_rq_pos(prev_req);
			unsigned long the_next = (unsigned long)blk_rq_pos(next_req);
		
			//Gets the differences	
			unsigned long diff_prev = 0;
			unsigned long diff_next = 0;

			if(the_prev > the_curr) {
				diff_prev = the_prev - the_curr;
			} else if (the_prev < the_curr) {
				diff_prev = the_curr - the_prev;
			} else {
				// equal
				diff_prev = 0;
			}

			if(the_next > the_curr) {
				diff_next = the_next - the_curr;
			} else if (the_next < the_curr) {
				diff_next = the_curr - the_next;
			} else {
				diff_next = 0;
			}

			printk("diff_prev = %lu, diff_next = %lu\n", diff_prev, diff_next);

			//If prev is closer to current, then dispatch prev next
			if (diff_prev < diff_next) {
				printk("Diff_prev < diff_next\n");
				nd->next_to_dispatch = nd->next_to_dispatch->prev;
			}
			//else choose next for the next to dispatch
			else { //(diff_prev >= diff_next) {
				printk("Diff_next <= diff_prev\n");
				nd->next_to_dispatch = nd->next_to_dispatch->next;
			}	
			
			//Delete currently dispatched request because it's finished
        	list_del_init(&rq->queuelist);
			nd->queue_count--;
		}

        printk("TEAM8 : Dispatching :%lu\n", (unsigned long)blk_rq_pos(rq));
		elv_dispatch_sort(q, rq);
        printk("TEAM8 : Done dispatching. Queue count at %d", nd->queue_count);
        return 1;
    }
    return 0;
}

static void sstf_print_list(struct request_queue *q)
{
	struct sstf_data *nd = q->elevator->elevator_data;

	struct list_head* position_print;
	struct request* print_node;
	printk("TEAM8 : Printing List: ");
	list_for_each(position_print, &nd->queue) {
		print_node = list_entry(position_print, struct request, queuelist);
		printk("%lu,", (unsigned long)blk_rq_pos(print_node));
	}
	printk("\n");
}

static void sstf_add_request(struct request_queue *q, struct request *rq)
{
	
	struct sstf_data *nd = q->elevator->elevator_data;
	int added = 0;
	
	printk("TEAM8 : Adding :%lu\n", (unsigned long)blk_rq_pos(rq));

	sstf_print_list(q);

	//If the list is empty, do a basic add and return
	if (list_empty(&nd->queue)){
		list_add(&rq->queuelist, &nd->queue);
		nd->next_to_dispatch = nd->queue.next;  
		nd->queue_count++;
        printk ("TEAM8 : List Was Empty\n");
		sstf_print_list(q);
		return;
	}

	struct list_head* position;
	list_for_each(position , &nd->queue) { 
		
		struct request* curr_req = list_entry(position, struct request, queuelist);
		struct request* curr_req_next = list_entry(position->next, struct request, queuelist);
		
		sector_t curr_req_sector = blk_rq_pos(curr_req);
		sector_t next_req_sector = blk_rq_pos(curr_req_next);
		sector_t new_req_sector = blk_rq_pos(rq);
	
		//This is for one item in the queue	
		if (nd->queue_count == 1){
			list_add(&rq->queuelist, position);
			nd->queue_count++;
			added = 1;
        	printk ("TEAM8 : Adding second item; next= %p and prev = %p\n" , position->next, position->prev );
			break;
		}	

		//This is for general queue addition	
		if (next_req_sector >= new_req_sector && curr_req_sector <=new_req_sector ){
			list_add(&rq->queuelist, position);
			nd->queue_count++;
			added = 1;
        	printk ("TEAM8 : Adding 3rd or higher item; next= %p and prev = %p\n" , position->next, position->prev);
			break;
		}
    }

	if(added != 1) {
		// Our new request must be bigger than all current, add it to the end
		list_add_tail(&rq->queuelist, &nd->queue);
		nd->queue_count++;
		printk("Added to tail!\n");
	}

	printk("TEAM8 : After adding. Queue count at %d\n", nd->queue_count);
	sstf_print_list(q);
}

static void *sstf_init_queue(struct request_queue *q)
{
	struct sstf_data *nd;
	printk("This is TEAM8's queue");
	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd)
		return NULL;
	INIT_LIST_HEAD(&nd->queue);
	nd->queue_count = 0;
	return nd;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static int sstf_deny_merge(struct request_queue *req_q, struct request *req,
			struct bio *bio)
{
	return ELEVATOR_NO_MERGE;
}

static struct elevator_type elevator_sstf = {
	.ops = {
		.elevator_allow_merge_fn 	= sstf_deny_merge,
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_init_fn			= sstf_init_queue,
		.elevator_exit_fn			= sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

static int __init sstf_init(void)
{
	elv_register(&elevator_sstf);

	return 0;
}

static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);


MODULE_AUTHOR("CS411 - Group 8");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");
