#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "thread.h"


int MAX;
int sum;

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

static void * sum_long(__attribute__((__unused__)) void *_value){
  while(sum < MAX){
    sum += 1;
    cpu_burn();
  }
  return NULL; 
}

static void * sum_fast(__attribute__((__unused__)) void *_value)
{
  while(sum < MAX){
    sum += 10000;
    thread_yield();
  }
  return NULL; 
}

int main(int argc, char** argv)
{
  sum=0;
  if (argc < 2) {
    printf("argument manquant: entier à atteindre\n");
    return -1;
  }
  MAX = atoi(argv[1]);
  thread_t t1, t2;
  thread_create(&t1, sum_long, NULL);
  thread_create(&t2, sum_fast, NULL);
  
  thread_join(t1, NULL);
  thread_join(t2, NULL);
  printf("La valeur a été dépassée\n");
  return 0;
}
