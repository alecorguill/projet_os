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
#define LOOP 100000000

int preempted;

void click_clock(__attribute__((__unused__)) int signo){
  preempted = 1;
}

unsigned long duration(struct timeval * t1, struct timeval * t2){
  return (t2->tv_sec - t1->tv_sec) * 1000000 + (t2->tv_usec - t1->tv_usec);
}

static void * too_long(__attribute__((__unused__)) void *_value)
{
  struct timeval t1, t2;
  unsigned long i, diff;
  ualarm(TIMESLICE + DELAY, 0); 
  gettimeofday(&t1, NULL);
  //loop too long to be handled on a single timeslice
  for(i = 0; i < LOOP; i++){
    //check if preempted
    if(preempted){
      preempted = 0; //init for other threads
      break;
    }
  }
  gettimeofday(&t2, NULL);
  diff = duration(&t1, &t2);
  //fprintf(stderr, "%lu\n", diff);
  if(diff < TIMESLICE + 2 * DELAY){
    //if thread has not been preempted, it ends before 2 timeslices
    fprintf(stderr, "\033[31mKO\033[0m\n");
  }else{
    fprintf(stderr, "\033[32mOK\033[0m\n");
  }
  fprintf(stderr, "too_long ended\n");
  return NULL;
}

static void * nothing_special(__attribute__((__unused__)) void *_value)
{
  int i;
  for(i = 0; i < LOOP; i++){
    preempted = 1;
  }
  fprintf(stderr, "nothing_special ended\n");
  return NULL;
}

int main()
{
  signal(SIGALRM, click_clock);
  preempted = 0; 
  srand(time(NULL));
  thread_t t1, t2;
  //thread 1 is looping for a duration slightly superior to the timeslice
  //if preemption went OK, t2 should be given a timeslice before t1 ends
  //so t1 should run a bit more than 2 timeslices
  thread_create(&t1, too_long, NULL);
  thread_create(&t2, nothing_special, NULL);
  fprintf(stderr, "too_long: %p\nnothing_special: %p\n", t1, t2);
	  
  thread_join(t1, NULL);
  thread_join(t2, NULL);

  return 0;
}
