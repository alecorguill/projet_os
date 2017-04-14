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
	pE->pointers.cqe_next = pE;
	pE->pointers.cqe_prev = pE;
	
	// Insert element at the head of the list
	CIRCLEQ_INSERT_HEAD(&(thread_pool.head), pE, pointers);
	
	return 0;
}
    
int thread_yield(void){
	if(CIRCLEQ_FIRST(&(thread_pool.head)) != CIRCLEQ_LAST(&(thread_pool.head))){ // More than one element
		Element *thread_next = CIRCLEQ_NEXT(thread_current, pointers);
		thread_current = thread_next;
		return swapcontext(&(thread_current->thread.uc), &(thread_next->thread.uc));
	}
	return 0;
}

//void thread_exit(void *retval





