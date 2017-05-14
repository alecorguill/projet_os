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
  char ok = 1;
  unsigned long i, diff, mean;
  //wait for 2 preemption to check timeslice change
  int j;
  for(j = 0; j < 2; j++){
    gettimeofday(&t1, NULL);
    //loop too long to be handled within a couple timeslice
    for(i = 0; i < LOOP; i++){
      //yield at TIMESLICE / 2 for the first j loop
      gettimeofday(&t2, NULL);
      if(j == 0 && (duration(&t1, &t2) > TIMESLICE / 2)){
	thread_yield();
      }
      //make sure loop is long with cpu wasting computation
      cpu_burn();
      //check if preempted twice
      if(preempted){
	preempted = 0;
	break;
      }
    }
    gettimeofday(&t2, NULL);
    diff = duration(&t1, &t2);    
    if(j == 0){
      //for first preemption, time must be < 2 * (TIMESLICE + DELAY)
      ok = ok && diff < 2 * (TIMESLICE + DELAY);
      mean = diff;
    }else{
      //for second preemption, time must > 2 * (TIMESLICE + DELAY)
      ok = ok && diff > 2 * (TIMESLICE + DELAY);
      mean += diff;
      mean /= 2;
      //mean of a timeslice must be TIMESLICE +/- DELAY
      ok = ok && mean > 2 * (TIMESLICE - DELAY) && mean < 2 * (TIMESLICE + DELAY);
    }
  }

  if(!ok){
    fprintf(stderr, "test_63 \033[31mKO\033[0m\n");
  }else{
    fprintf(stderr, "test_63 \033[32mOK\033[0m\n");
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
