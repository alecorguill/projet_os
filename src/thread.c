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
	///thread_current->thread.uc = ???;
	thread_current->thread.thread_waiting_for_me = NULL;
	/// thread_current->thread.is_waited = ???;
	thread_current->thread.is_waiting = 0;
	thread_current->thread.retval = NULL;
	
	// point to head (beacon)
	thread_current->pointers.cqe_next = (void *)(&thread_pool.head);
	thread_current->pointers.cqe_prev = (void *)(&thread_pool.head);
	
	// set thread_current to first
	thread_pool.head.cqh_first = thread_current;
	thread_pool.head.cqh_last = thread_current;
	
	thread_done.head.cqh_first = (void *) &(thread_done.head);
	thread_done.head.cqh_last = (void *) &(thread_done.head);
}


void exec_and_save(void *(*func)(void*), void *funcarg){

  // To save func retval in thread structure
  thread_exit(func(funcarg));

  // Set the next context
  struct Element *next = CIRCLEQ_NEXT(thread_current, pointers);
  if(CIRCLEQ_FIRST(&(thread_pool.head)) == CIRCLEQ_LAST(&(thread_pool.head))) // one element
    return;
  
  // Remove current thread from the list
  CIRCLEQ_REMOVE(&(thread_pool.head), thread_current, pointers);
  thread_current = next;
  
  setcontext(&(next->thread.uc));
}

thread_t thread_self(void){
  return thread_current->thread.id;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){
	// Create context
	ucontext_t uc;
	getcontext(&uc);
	uc.uc_stack.ss_size = 64*1024;
    uc.uc_stack.ss_sp = malloc(uc.uc_stack.ss_size);
    uc.uc_link = &(thread_current->thread.uc); // back to current when done
	
	makecontext(&uc, (void (*)(void)) exec_and_save, 2, func, funcarg);
	///if(uc == NULL) return -1;
	
	// Create thread corresponding to context
	Thread t;
	t.id = *newthread;
	t.uc = uc;
	t.thread_waiting_for_me = NULL;
	t.is_waiting = 0;
	t.retval = NULL;

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
  Element * pE;
  Element * tmp;
  
  CIRCLEQ_FOREACH(pE, &(thread_pool.head), pointers){
    if(!(pE->thread.is_waiting))
	  break;
  }
  if(pE->thread.is_waiting)
    return 1; // Deadlock: all threads are waiting for one another
  
  CIRCLEQ_REMOVE(&(thread_pool.head), thread_current, pointers);
  CIRCLEQ_INSERT_TAIL(&(thread_pool.head), thread_current, pointers);
  
  tmp = thread_current;
  thread_current = pE;
  
  return swapcontext(&(tmp->thread.uc), &(pE->thread.uc));
}


int thread_join(thread_t thread, void **retval){
  Element * pE;
  Element * tmp;
  
  // If thread is already in thread_done, we can transfer the return value  
  CIRCLEQ_FOREACH(pE, &(thread_done.head), pointers){
	printf("pE = %p\n", pE);
	  
    if(pE->thread.id == thread){
	  *retval = pE->thread.retval;		
      return 0;
	}
  }
  
  CIRCLEQ_FOREACH(pE, &(thread_pool.head), pointers){
    if(pE->thread.id == thread)
		break;
  }
  if(pE->thread.id != thread)
	return 3;

  
  if(pE->thread.thread_waiting_for_me != NULL){ // thread is already being awaited, so current cannot wait as well
	///*retval = NULL;
	if(pE->thread.thread_waiting_for_me == thread_current){
	  return 2; // Deadlock situation: current is waiting thread, which is waiting current
    }

    return 1; 
  }


  pE->thread.thread_waiting_for_me = malloc(sizeof(Element));
  pE->thread.thread_waiting_for_me = thread_current; // current is waiting for me
  thread_current->thread.is_waiting = 1;

  tmp = thread_current;
  thread_current = pE;
  
  int rt = swapcontext(&(tmp->thread.uc), &(pE->thread.uc));
  
  *retval = pE->thread.retval;
  
  return rt;
}







void thread_exit(void *retval) {
  thread_current->thread.retval = retval;
  
  CIRCLEQ_REMOVE(&(thread_pool.head), thread_current, pointers);
  CIRCLEQ_INSERT_HEAD(&(thread_done.head), thread_current, pointers);
  
  Element * pE = thread_current->thread.thread_waiting_for_me;
  
  if(pE != NULL){
	  pE->thread.is_waiting = 0;
  }
  
  thread_yield();
}


