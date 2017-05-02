#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "thread.h"

/* fibonacci.
 *
 * la durée doit être proportionnel à la valeur du résultat.
 * valgrind doit être content.
 * jusqu'à quelle valeur cela fonctionne-t-il ?
 *
 * support nécessaire:
 * - thread_create()
 * - thread_join() avec récupération de la valeur de retour
 * - retour sans thread_exit()
 */

static void * fibo(void *_value)
{
  thread_t th, th2;
  /*
  thread_t th = (thread_t)(_value-1);
  thread_t th2 = (thread_t)(_value-2);
  */
  int err;
  void *res = NULL, *res2 = NULL;
  unsigned long value = (unsigned long) _value;

  /* on passe un peu la main aux autres pour eviter de faire uniquement la partie gauche de l'arbre */
  thread_yield();


  //printf("----------\n");
  if (value < 3){
	//printf("value = %d -> res = 1\n", value);
    return (void*) 1;
  }
/*
  printf("value = %d\n", _value);
  printf("th = %p\n", th);
  printf("th2 = %p\n", th2);
*/
  err = thread_create(&th, fibo, (void*)(value-1));
  assert(!err);
  err = thread_create(&th2, fibo, (void*)(value-2));
  assert(!err);
  
  
  //printf("%d waiting for %d (%p)\n", value, value-1, (void*)th);
  err = thread_join(th, &res);
  assert(!err);
  //printf("----------\n");
  //printf("%d waiting for %d (%p)\n", value, value-2, th2);
  err = thread_join(th2, &res2);
  assert(!err);
  
  //printf("fibo%d, res1 = %d, res2 = %d\n", value, res, res2);

  return (void*)((unsigned long) res + (unsigned long) res2);
}

int main(int argc, char *argv[])
{
  unsigned long value, res;

  if (argc < 2) {
    printf("argument manquant: entier x pour lequel calculer fibonacci(x)\n");
    return -1;
  }

  value = atoi(argv[1]);
  res = (unsigned long) fibo((void *)value);
  printf("fibo de %ld = %ld\n", value, res);

  return 0;
}
