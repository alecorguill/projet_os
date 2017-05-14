#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "thread.h"

#define TIMESLICE 4000
#define DELAY 100 //approximative max execution time of the thread functions
#define LOOP 100000

struct timeval t1, t2;
int preempted;

void cpu_burn(){
  double useless;
  int i;
  for(i = 0; i < 1000; i++){
    useless += i;
  }
}

unsigned long duration(struct timeval * t1, struct timeval * t2){
  return (t2->tv_sec - t1->tv_sec) * 1000000 + (t2->tv_usec - t1->tv_usec);
}

static void * too_long(__attribute__((__unused__)) void *_value)
{
  unsigned long i, diff;
  //ualarm(4 * TIMESLICE + DELAY, 0); 
  gettimeofday(&t1, NULL);
  //loop too long to be handled within a couple timeslice
  for(i = 0; i < LOOP; i++){
    //make sure loop is long with cpu waisting computation
    cpu_burn();
    //check if preempted
    if(preempted){
      preempted = 0; //init for other threads
      break;
    }
  }
  gettimeofday(&t2, NULL);
  diff = duration(&t1, &t2);
  if(diff > 2 * (TIMESLICE + DELAY)){
    //if thread has not been preempted, it ends after 2 timeslices
    fprintf(stderr, "test_62 \033[31mKO\033[0m\n");
  }else{
    fprintf(stderr, "test_62 \033[32mOK\033[0m\n");
  }
  return NULL;
}

static void * nothing_special(__attribute__((__unused__)) void *_value)
{
  int i;
  for(i = 0; i < LOOP; i++){
    cpu_burn();
    preempted = 1;
  }
  return NULL;
}

int main()
{
  preempted = 0; 
  srand(time(NULL));
  thread_t t1, t2;
  //thread 1 is looping for a duration slightly superior to the timeslice
  //if preemption went OK, t2 should be given a timeslice before t1 ends
  //so t1 should run a bit more than 2 timeslices
  thread_create(&t1, too_long, NULL);
  thread_create(&t2, nothing_special, NULL);
	  
  thread_join(t1, NULL);
  thread_join(t2, NULL);

  return 0;
}
