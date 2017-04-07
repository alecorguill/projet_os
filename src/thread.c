#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>

thread_t thread_self(void){
  struct Element e;
  LIST_FOREACH(e, thread_pool.head, pointers){
    if(e.thread.uc == thread_current.uc){
      return e.thread.uc;
    }
  }
  return NULL;
}
    
