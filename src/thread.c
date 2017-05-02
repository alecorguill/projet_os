#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>
#include <valgrind/valgrind.h>
#include <unistd.h>
#include "thread.h"

static int mutex_id = 0;
static struct List thread_pool;
static Thread * thread_current = NULL;

/*
  Debug function
*/
void print_list(struct List * l){
  Thread * t = thread_current;  
  fprintf(stderr, "thread_pool: \n");
  for(t = l->head.cqh_first; t != (void *)&l->head; t = t->pointers.cqe_next){
    fprintf(stderr, "element: %p\n", t);           
  }
 
  /* while( CIRCLEQ_LOOP_NEXT(&l->head, t, pointers) != thread_current ){ */
  /* //while( CIRCLEQ_NEXT(t, pointers) != thread_current ){ */
  /*   fprintf(stderr, "element: %p\n", t); */
  /*   t = CIRCLEQ_LOOP_NEXT(&l->head, t, pointers); */
  /* }  */
}

void free_thread(Thread * t){
  if(t->valgrind_stackid != MAIN_STACK){//do not free the main stack 
    VALGRIND_STACK_DEREGISTER(t->valgrind_stackid);
    free(t->uc.uc_stack.ss_sp);
  }  
  if(t != NULL){
    free(t);
  }
}


__attribute__ ((__constructor__)) 
void pre_func(void){
	// init thread_current
	thread_current = malloc(sizeof(Thread));
	getcontext(&thread_current->uc);
	thread_current->thread_waiting_for_me = NULL;
	thread_current->is_done = 0;
	thread_current->retval = NULL; 
	thread_current->valgrind_stackid = MAIN_STACK;
	 
	// point to head (beacon)
	thread_current->pointers.cqe_next = (void *)(&thread_pool.head);
	thread_current->pointers.cqe_prev = (void *)(&thread_pool.head);

	// set thread_current to first
	CIRCLEQ_INIT(&(thread_pool.head));
	CIRCLEQ_INSERT_HEAD(&(thread_pool.head), thread_current, pointers);       
}

__attribute__ ((__destructor__)) 
void post_func(void){
  Thread * next;
  while( (next = CIRCLEQ_LOOP_NEXT(&thread_pool.head, thread_current, pointers)) != thread_current ){
    CIRCLEQ_REMOVE(&thread_pool.head, next, pointers);
    free_thread(next);
  }
  free(thread_current);
}

void exec_and_save(void *(*func)(void*), void *funcarg){
  // To save func retval in thread structure
  thread_exit(func(funcarg));
}

thread_t thread_self(void){
  return thread_current;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){

	Thread * t = malloc(sizeof(Thread));
	// Create context 
	getcontext(&t->uc);//avoid valgrind errors
	t->uc.uc_stack.ss_size = 64*1024;
	t->uc.uc_stack.ss_sp = malloc(t->uc.uc_stack.ss_size);
	int valgrind_stackid = VALGRIND_STACK_REGISTER(t->uc.uc_stack.ss_sp, t->uc.uc_stack.ss_sp + t->uc.uc_stack.ss_size);
	makecontext(&t->uc, (void (*)(void)) exec_and_save, 2, func, funcarg);
	
	// Create thread corresponding to context
	t->thread_waiting_for_me = NULL;
	t->is_done = 0;
	t->retval = NULL;
	t->valgrind_stackid = valgrind_stackid;
	*newthread = t;

	// Insert element at the end of the list
	CIRCLEQ_INSERT_BEFORE(&(thread_pool.head), thread_current, t, pointers);
	return 0;
}
    
int thread_yield(void){  

  Thread * new = CIRCLEQ_LOOP_NEXT(&(thread_pool.head), thread_current, pointers);
  Thread * old = thread_current;
  thread_current = new;
  if(new != old){//do not swap on same context
    return swapcontext(&(old->uc), &(new->uc));
  }
  return 0;
}

int thread_join(thread_t thread, void **retval){
  
  //prevent a waiting thread from being yield to
  thread->thread_waiting_for_me = thread_current;
  //CIRCLEQ_REMOVE(&(thread_pool.head), thread_current, pointers);
  while(!thread->is_done){
    //yield until thread is done
    thread_yield();
  }
  if(retval != NULL){
    //if result is not ignored    
    *retval = thread->retval;
  }
  //the current thread does not have to wait anymore, give it priority
  //CIRCLEQ_INSERT_AFTER(&(thread_pool.head), thread_current, thread->thread_waiting_for_me, pointers);
  //free the joined thread
  free_thread(thread);
  return 0;
}


void thread_exit(void *retval) {
  thread_current->is_done = 1;
  thread_current->retval = retval;
  Thread * next = CIRCLEQ_LOOP_NEXT(&thread_pool.head, thread_current, pointers);

  /* if(thread_current->thread_waiting_for_me != NULL){ */
  /*   CIRCLEQ_INSERT_AFTER(&(thread_pool.head), thread_current, thread_current->thread_waiting_for_me, pointers); */
  /*   next = thread_current->thread_waiting_for_me; */
  /* }else{ */
  /*   next = CIRCLEQ_LOOP_NEXT(&thread_pool.head, thread_current, pointers); */
  /* } */
  if(thread_current == next){    
    //if 1 element in thread pool, exit program
    exit(EXIT_SUCCESS);
  }

  //remove current thread from queue. It will be freed only if it's joined
  CIRCLEQ_REMOVE(&(thread_pool.head), thread_current, pointers);  
  //equivalent of a yield but with a setcontext instead of a swapcontext
  thread_current = next;
  setcontext(&(next->uc));
  fprintf(stderr, "thread_exit error\n");
  exit(EXIT_FAILURE);//avoid no_return related warning
}




int thread_mutex_init(thread_mutex_t *mutex){
	if(mutex == NULL)
		return 1;
	
	mutex->id = mutex_id++;
	mutex->is_locked = 0;

	return 0;
}





int thread_mutex_destroy(thread_mutex_t *mutex){	
	if(mutex == NULL)
		return 1;
		
	return 0;
}

int thread_mutex_lock(thread_mutex_t *mutex){
	if(mutex == NULL)
		return 1;
	
	while(mutex->is_locked){
		thread_yield();
	}
	
	/// BEWARE OF PREEMPTION HERE!
	
	mutex->is_locked = 1;
	
	return 0;
}

int thread_mutex_unlock(thread_mutex_t *mutex){
	if(mutex == NULL)
		return 1;
		
	mutex->is_locked = 0;
		
	return 0;
}





