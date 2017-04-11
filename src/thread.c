#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>
#include "thread.h"

void exec_and_save(void *(*func)(void*)){
  
}

thread_t thread_self(void){
  
  /* Ca c'est pourri, l'autre groupe le modifie 
  struct Element e;
  LIST_FOREACH(e, thread_pool.head, pointers){
    if(e.thread.uc == thread_current.uc){
      return e.thread.uc;
    }
  }
  return NULL;
  */
}
    
int thread_yield(void){
  struct Element *thread_next = CIRCLEQ_NEXT(thread_current, pointers);
  thread_current = thread_next;
  return swapcontext(&thread_current.thread.uc, thread_next.thread.uc);
}

void thread_exit(void *retval
