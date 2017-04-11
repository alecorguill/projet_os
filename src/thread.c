#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>
#include "thread.h"

void exec_and_save(void *(*func)(void*)){
  
}

thread_t thread_self(void){
  return thread_current->thread.id;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){
	// Create context
	ucontext_t uc;
	makecontext(&uc, (void (*)(void)) func, 1, funcarg);
	///if(uc == NULL) return -1;
	
	// Create thread corresponding to context
	Thread t;
	t.id = *newthread;
	t.uc = uc;

	// create element to store thread
	Element* pE = malloc(sizeof(Element));
	if(pE == NULL) return -1;
	pE->thread = t;
	pE->pointers.cqe_next = NULL;
	pE->pointers.cqe_prev = NULL;
	
	// Insert element at the head of the list
	CIRCLEQ_INSERT_HEAD(&(thread_pool.head), pE, pointers);
	
	return 0;
}
    
int thread_yield(void){
  struct Element *thread_next = CIRCLEQ_NEXT(thread_current, pointers);
  thread_current = thread_next;
  return swapcontext(&(thread_current->thread.uc), &(thread_next->thread.uc));
}

//void thread_exit(void *retval
