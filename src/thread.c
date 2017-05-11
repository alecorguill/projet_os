#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>
#include <valgrind/valgrind.h>
#include <unistd.h>
#include <sys/time.h>
#include "thread.h"

#define TIMEOUT 4000//4ms before preemption
#define PREEMPTION

static int mutex_id = 0;
static struct List thread_pool;
static Thread * thread_current = NULL;

#ifdef PREEMPTION
char preemption;
struct timeval last_yield;


void preempter(__attribute__((__unused__)) int signo){
  if(preemption){
    fprintf(stderr, "preemption\n");
    gettimeofday(&last_yield, NULL);
    thread_yield();
  }
}

unsigned long get_duration(struct timeval* t1, struct timeval* t2){
  //return time in micro_s
  return (t2->tv_sec - t1->tv_sec) * 1000000 + (t2->tv_usec - t1->tv_usec);
}

void set_alarm(){
  struct timeval now;
  gettimeofday(&now, NULL);
  unsigned long diff = get_duration(&last_yield, &now);
  //fprintf(stderr, "prochaine alarme dans %lu micro secondes\n", TIMEOUT - diff);
  if(diff >= TIMEOUT){
    //TIMEOUT over
    //(no need to reactivate preemption for the yield)
    preempter(0);
  }else{
    preemption = 1;
    ualarm(TIMEOUT - diff, 0);
  }
}
#endif


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
  #ifdef PREEMPTION
  signal(SIGALRM, preempter);
  #endif
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
  gettimeofday(&last_yield, NULL);
}

__attribute__ ((__destructor__)) 
void post_func(void){
  preemption = 0;
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
  #ifdef PREEMPTION
  preemption = 0;
  #endif

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

  #ifdef PREEMPTION
  set_alarm();
  #endif

  return 0;

}
    
int thread_yield(void){  
  #ifdef PREEMPTION
  preemption = 0;
  #endif

  Thread * new = CIRCLEQ_LOOP_NEXT(&(thread_pool.head), thread_current, pointers);
  Thread * old = thread_current;
  thread_current = new;
  #ifdef PREEMPTION
  preemption = 1;
  #endif
  if(new != old){//do not swap on same context
    gettimeofday(&last_yield, NULL);    
    #ifdef PREEMPTION
    ualarm(TIMEOUT, 0);        
    #endif
    return swapcontext(&(old->uc), &(new->uc));
  }
  return 0;
}

int thread_join(thread_t thread, void **retval){
  #ifdef PREEMPTION
  preemption = 0;
  #endif
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
  #ifdef PREEMPTION
  set_alarm();
  #endif
  return 0;
}


void thread_exit(void *retval) {
  #ifdef PREEMPTION
  preemption = 0;
  #endif

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
  #ifdef PREEMPTION
  set_alarm();
  #endif
  setcontext(&(next->uc));
  fprintf(stderr, "thread_exit error\n");
  exit(EXIT_FAILURE);//avoid no_return related warning
}




int thread_mutex_init(thread_mutex_t *mutex){
	preemption = 0;
	
	if(mutex == NULL)
		return 1;
	
	mutex->id = mutex_id++;
	mutex->is_locked = 0;
	CIRCLEQ_INIT(&(mutex->threads_waiting_for_unlock.head));

	return 0;
}





int thread_mutex_destroy(thread_mutex_t *mutex){	
	if(mutex == NULL)
		return 1;
		
	return 0;
}

int thread_mutex_lock(thread_mutex_t *mutex){
	preemption = 0;
	
	if(mutex == NULL)
		return 1;
	
	//printf("LOCK ATTEMPT %p...", thread_current);
	
	if(mutex->is_locked){
		Thread* old = thread_current;
		Thread* new = CIRCLEQ_LOOP_NEXT(&(thread_pool.head), thread_current, pointers);
		thread_current = new;

		//printf("  WAITING %p -> %p\n", old, new);
		
		// Put the thread into the waiting List
		CIRCLEQ_REMOVE(&(thread_pool.head), old, pointers);
		CIRCLEQ_INSERT_TAIL(&(mutex->threads_waiting_for_unlock.head), old, pointers);  
		
		int rt = swapcontext(&(old->uc), &(new->uc));
		if(rt != 0) return rt;
	}
	
	//printf("  LOCKING %p\n", thread_current);
	__sync_lock_test_and_set(&(mutex->is_locked), 1);
	
	return 0;
}

int thread_mutex_unlock(thread_mutex_t *mutex){
	preemption = 0;
	
	if(mutex == NULL)
		return 1;
	
	mutex->is_locked = 0;
	
	if(!CIRCLEQ_EMPTY(&(mutex->threads_waiting_for_unlock.head))){
		Thread* new = CIRCLEQ_FIRST(&(mutex->threads_waiting_for_unlock.head));
		CIRCLEQ_REMOVE(&(mutex->threads_waiting_for_unlock.head), new, pointers);
		CIRCLEQ_INSERT_AFTER(&(thread_pool.head), thread_current, new, pointers);
		
		//printf("  NEW %p\n", new); 
	}
	
	thread_yield(); // nice gesture
		
	return 0;
}





