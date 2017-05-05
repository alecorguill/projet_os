#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "thread.h"

 
typedef struct Bigtab{
  unsigned long* tab;
  unsigned len;
} Bigtab;


static void * bigsum(void *_bigtab)
{
  thread_t th, th2;
  int err;
  void *res = NULL, *res2 = NULL;
  Bigtab bigtab;
  bigtab.tab = ((Bigtab*) _bigtab)->tab;
  bigtab.len = ((Bigtab*) _bigtab)->len;

  /* on passe un peu la main aux autres pour eviter de faire uniquement la partie gauche de l'arbre */
  thread_yield();

  if (bigtab.len < 2){
    return (void*) bigtab.tab[0];
  }
  
  Bigtab bigtab1, bigtab2;
  bigtab1.len = bigtab.len/2;
  bigtab2.len = bigtab.len - bigtab1.len;
  bigtab1.tab = bigtab.tab;
  bigtab2.tab = &(bigtab.tab[bigtab1.len]);

  err = thread_create(&th, bigsum, (void*)(&bigtab1));
  assert(!err);
  err = thread_create(&th2, bigsum, (void*)(&bigtab2));
  assert(!err);
  
  err = thread_join(th, &res);
  assert(!err);
  err = thread_join(th2, &res2);
  assert(!err);

  return (void*)((unsigned long) res + (unsigned long) res2);
}

int main(int argc, char *argv[])
{
  unsigned long n, res;

  if (argc < 2) {
    printf("argument manquant: entier x pour lequel calculer bigsum(x)\n");
    return -1;
  }
  n = atoi(argv[1]);
  
  Bigtab bigtab;
  bigtab.tab = malloc(n*sizeof(unsigned long));
  bigtab.len = n;
  unsigned long i;
  for(i=0; i<n; i++){
	  bigtab.tab[i] = i;
  }
  
  res = (unsigned long) bigsum((void *)(&bigtab));
  printf("bigsum de %ld = %ld\n", n, res);
  
  free(bigtab.tab);

  return 0;
}
