#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>
#include "thread.h"


#define MAIN_ID		0
/// Gérer le Main_id



__attribute__ ((__constructor__)) 
void pre_func(void){
	thread_current = malloc(sizeof(Element));
	
	// init thread_current
	thread_current->thread.id = MAIN_ID;
	//thread_current->thread.uc = ???;
	thread_current->thread.thread_waiting = NULL;
	thread_current->thread.retval = NULL;
	
	// point to head (beacon)
	thread_current->pointers.cqe_next = (void *)(&thread_pool.head);
	thread_current->pointers.cqe_prev = (void *)(&thread_pool.head);
	
	// set thread_current to first
	thread_pool.head.cqh_first = thread_current;
	thread_pool.head.cqh_last = thread_current;
}


void exec_and_save(void *(*func)(void*), void *funcarg){

  // To save func retval in thread structure
  thread_exit(func(funcarg));

  // Set the next context
  struct Element *next = CIRCLEQ_NEXT(thread_current, pointers);
  if(CIRCLEQ_FIRST(&(thread_pool.head)) == CIRCLEQ_LAST(&(thread_pool.head))) // one element
    return;
  
  setcontext(&(next->thread.uc));

  // Remove current thread from the list
  CIRCLEQ_REMOVE(&(thread_pool.head), thread_current, pointers);
  thread_current = next;
}

thread_t thread_self(void){
  return thread_current->thread.id;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){
	// Create context
	ucontext_t uc;
	makecontext(&uc, (void (*)(void)) exec_and_save, 2, func, funcarg);
	///if(uc == NULL) return -1;
	
	// Create thread corresponding to context
	Thread t;
	t.id = *newthread;
	t.uc = uc;

	// create element to store thread
	Element* pE = malloc(sizeof(Element));
	if(pE == NULL) return -1;
	pE->thread = t;
	pE->pointers.cqe_next = pE;
	pE->pointers.cqe_prev = pE;
	
	// Insert element at the head of the list
	CIRCLEQ_INSERT_HEAD(&(thread_pool.head), pE, pointers);
	
	return 0;
}
    
int thread_yield(void){
  /*	
	if(CIRCLEQ_FIRST(&(thread_pool.head)) != CIRCLEQ_LAST(&(thread_pool.head))){ // More than one element
		Element *thread_next = CIRCLEQ_NEXT(thread_current, pointers);
		thread_current = thread_next;
		return swapcontext(&(thread_current->thread.uc), &(thread_next->thread.uc));
	}
	return 0;
  */

  struct Element *e;
  
  // If thread is done
  CIRCLEQ_FOREACH(e, &(thread_pool.head), pointers){
    if(!(e->thread.is_waiting)){
      thread_current = e;
      return swapcontext(&(thread_current->thread.uc), &(e->thread.uc));
    }
  }
  
  return 1; // Deadlock
}


int thread_join(thread_t thread, void **retval){
  struct Element *e;
  
  // If thread is done
  CIRCLEQ_FOREACH(e, &(thread_done.head), pointers){
    if(e->thread.id == thread)
      return 0;
  }
  
  CIRCLEQ_FOREACH(e, &(thread_pool.head), pointers){
    if(e->thread.id == thread){
      
      if(e->thread.thread_waiting == thread_current->thread.id)
		return 2;//thread joined twice

      if(e->thread.thread_waiting != NULL)
		return 1;//thread already joined by another thread
      
      e->thread.thread_waiting = thread_current;
      e->thread.is_waiting = 1;
      thread_current = e;
      return swapcontext(&(thread_current->thread.uc), &(e->thread.uc));
    }
  }
  
  return -1; // Should not get to this point (thread is in thread_pool)
}

void thread_exit(void *retval) {
  thread_current->thread.retval = retval;
}


