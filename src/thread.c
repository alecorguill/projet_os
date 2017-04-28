#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>
#include <valgrind/valgrind.h>

#include "thread.h"

static struct List thread_pool;
static struct List thread_done;
static struct Element * thread_current = NULL;

/*
  Debug function
*/
void print_list(struct List * l, char * name){
  struct Element * e;
  fprintf(stderr, "list %s: \n", name);
  CIRCLEQ_FOREACH(e, &l->head, pointers){
    fprintf(stderr, "element: %p\n", &e->thread);
  }
}

void free_element(Element * e){
	if(e != NULL && e != thread_current){
		if(e->thread->uc.uc_stack.ss_sp != NULL){ /// Valgrind grogne un peu car il pense que c'est pas initialisé
			VALGRIND_STACK_DEREGISTER(e->thread->valgrind_stackid);
			free(e->thread->uc.uc_stack.ss_sp);
		}
		free(e->thread);
		free(e);
	}
}


__attribute__ ((__constructor__)) 
void pre_func(void){
	ucontext_t uc;
	getcontext(&uc);
	uc.uc_stack.ss_size = 64*1024;
	uc.uc_stack.ss_sp = malloc(uc.uc_stack.ss_size);
	int valgrind_stackid = VALGRIND_STACK_REGISTER(uc.uc_stack.ss_sp, uc.uc_stack.ss_sp + uc.uc_stack.ss_size);
	uc.uc_link = NULL;
	
	// init thread_current
	thread_current = malloc(sizeof(Element));
	thread_current->thread = malloc(sizeof(Thread));
	thread_current->thread->uc = uc;
	thread_current->thread->thread_waiting_for_me = NULL;
	/// thread_current->thread->is_waited = ???;
	thread_current->thread->is_waiting = 0;
	thread_current->thread->retval = NULL;
	thread_current->thread->valgrind_stackid = valgrind_stackid;
	
	// point to head (beacon)
	thread_current->pointers.cqe_next = (void *)(&thread_pool.head);
	thread_current->pointers.cqe_prev = (void *)(&thread_pool.head);

	// set thread_current to first
	CIRCLEQ_INIT(&(thread_pool.head));
	CIRCLEQ_INSERT_HEAD(&(thread_pool.head), thread_current, pointers);
	
	CIRCLEQ_INIT(&(thread_done.head));
}

__attribute__ ((__destructor__)) 
void post_func(void){
  Element * e;
  /*
  CIRCLEQ_FOREACH(e, &(thread_done.head),pointers){
    fprintf(stderr, "%p\n",e);
  }  
  fprintf(stderr, "\n");
  */

  while(!CIRCLEQ_EMPTY(&(thread_done.head))){
	e = CIRCLEQ_FIRST(&(thread_done.head));
	CIRCLEQ_REMOVE(&(thread_done.head), e, pointers);
    free_element(e);
  }

  if(thread_current != NULL)
    free_element(thread_current);
}

void exec_and_save(void *(*func)(void*), void *funcarg){

  // To save func retval in thread structure
  thread_exit(func(funcarg));
}

thread_t thread_self(void){
  return thread_current->thread;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){
	// Create context 
	ucontext_t uc;
	getcontext(&uc);
	uc.uc_stack.ss_size = 64*1024;
	uc.uc_stack.ss_sp = malloc(uc.uc_stack.ss_size);
	int valgrind_stackid = VALGRIND_STACK_REGISTER(uc.uc_stack.ss_sp, uc.uc_stack.ss_sp + uc.uc_stack.ss_size);
	uc.uc_link = NULL;
	makecontext(&uc, (void (*)(void)) exec_and_save, 2, func, funcarg);
	
	// Create thread corresponding to context
	Thread * t = malloc(sizeof(Thread));
	t->uc = uc;
	t->thread_waiting_for_me = NULL;
	t->is_waiting = 0;
	t->retval = NULL;
	t->valgrind_stackid = valgrind_stackid;
	*newthread = t;

	// create element to store thread
	Element * e = malloc(sizeof(Element));
	if(e == NULL) return -1;
	e->thread = t;
	e->pointers.cqe_next = e;
	e->pointers.cqe_prev = e;
	
	// Insert element at the head of the list
	CIRCLEQ_INSERT_HEAD(&(thread_pool.head), e, pointers);

	return 0;
}
    
int thread_yield(void){
  
  Element * new = CIRCLEQ_LOOP_NEXT(&(thread_pool.head), thread_current, pointers);
  Element * old;

  while(new != thread_current){ // check (at most) all thread_pool
	if(!(new->thread->is_waiting)) // thread ready to go
      break;
	  
    new = CIRCLEQ_LOOP_NEXT(&(thread_pool.head), new, pointers);
  }
  if(new->thread->is_waiting)
    return 1; // Deadlock: all threads are waiting for one another

  old = thread_current;
  thread_current = new;
  return swapcontext(&(old->thread->uc), &(new->thread->uc));
}

/* Thread_join return values:
 * 0 - All went well
 * 	  > Either the return value is already in retval (if thread is already done)
 *    > Either thread_current is set to be waiting and will be woken up when thread is done
 * -----
 * Error codes:
 * 1 - thread was not found in thread_pool nor in thread_done
 * 2 - Deadlock situation: thread is already waiting for thread_current, so thread_current cannot wait back for thread
 * 3 - thread is already being waited for by another thread
 */
int thread_join(thread_t thread, void **retval){
  Element * e;

  /* fprintf(stderr, "join\n"); */
  /* print_list(&thread_pool, "thread_pool"); */
  /* print_list(&thread_done, "thread_done"); */
  // A) If thread is already in thread_done, we can immediately transfer the return value  
  CIRCLEQ_FOREACH(e, &(thread_done.head), pointers){
    if(e->thread == thread){
      if(retval != NULL)
	    *retval = e->thread->retval;
		
      return 0;
    }
  }
  
  // B) Else thread is in thread_pool and has to be run first
  CIRCLEQ_FOREACH(e, &(thread_pool.head), pointers){
    if(e->thread == thread)
		break;
  }
  if(e->thread != thread)
	return 1;

  if(e->thread->thread_waiting_for_me != NULL){
	if(e->thread->thread_waiting_for_me == thread_current){
	  return 2; 
    }
    return 3; 
  }


  e->thread->thread_waiting_for_me = thread_current;
  thread_current->thread->is_waiting = 1;

  Element * old = thread_current;
  thread_current = e;
  int rt = swapcontext(&(old->thread->uc), &(e->thread->uc));
  
  *retval = e->thread->retval;
  return rt;
}


void thread_exit(void *retval) {
  thread_current->thread->retval = retval;
  
  if(CIRCLEQ_FIRST(&(thread_pool.head)) != CIRCLEQ_LAST(&(thread_pool.head))){ // at least two elements (ie, more than just the main)
    Element * next = CIRCLEQ_LOOP_NEXT(&(thread_pool.head), thread_current, pointers);
    
    // Put thread_current in the thread_done List
    CIRCLEQ_REMOVE(&(thread_pool.head), thread_current, pointers);
    CIRCLEQ_INSERT_HEAD(&(thread_done.head), thread_current, pointers);

    // Wake up the thread which was waiting for thread_current
    Element * e = thread_current->thread->thread_waiting_for_me;
    if(e != NULL){
      e->thread->is_waiting = 0;
      next = e; /// Prioritize the thread which was waiting
    }
    
    Element * old = thread_current;
    thread_current = next;
    swapcontext(&(old->thread->uc), &(next->thread->uc));
  }
  
  exit(EXIT_SUCCESS);
}
