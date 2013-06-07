
#include <sched.h>
#include <linux/sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#include <errno.h>

int check_pri()
{
   // Since -1 is a legitimate return value, must clear and check "errno"
   errno = 0;
   int priority = getpriority(PRIO_PROCESS, 0);
   if (errno != 0)
      perror("getpriority"), exit(-1);
   return priority;
}

void set_the_priority()
{
   int new_priority = 1;
   if (setpriority(PRIO_PROCESS, 0, new_priority) != 0)
      perror("setpriority"), exit(-1);
}


int check_scheduler()
{
   int sched = sched_getscheduler(0);
   if (sched < 0)
      perror("sched_getscheduler"), exit(-1);
   return sched;
}

void
fix_sched()
{
   struct sched_param param = { 0 };
   param.sched_priority = 1;

   if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) 
      perror("sched_setscheduler"), exit(-1);
}

void
display_sched_and_pri()
{
   printf("Priority currently set to %d\n", check_pri());

   int sched = check_scheduler();
   char *sched_name = " *** ERROR ***";
   switch (sched) {
      case SCHED_OTHER: sched_name = "SCHED_OTHER"; break;
      case SCHED_BATCH: sched_name = "SCHED_BATCH"; break;
      case SCHED_IDLE:  sched_name = "SCHED_IDLE" ; break;
      case SCHED_RR:    sched_name = "SCHED_RR"   ; break;
      case SCHED_FIFO:  sched_name = "SCHED_FIFO" ; break;
   }
   printf("Scheduler currently set to %s\n", sched_name);
}

int
main(int argc, char* argv[])
{
   // I had trouble setting the priority via sched_setscheduler()
   // So, instead, I set it *first*.  Can't seem to change it later.
    set_the_priority();
    fix_sched();
    display_sched_and_pri();
    long length = argc>1 ? atol(argv[1]) : 400000000;
    long i,counter=0;
    long minor_length = length/10;
    for (i=0; i<length; i++) {
        if (counter % minor_length == 0) {
            struct timeval tv;
            if (gettimeofday(&tv, 0) != 0){
                perror("gettimeofday failed");
                exit(-1);
            }
            printf("time = %9ld.%06ld pid: %d - counter: %ld\n ", (long)tv.tv_sec, (long)tv.tv_usec, getpid(), counter);
        }
        counter++;
    }
   
   return 0;
}

