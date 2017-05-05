#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "thread.h"

 
typedef struct Bigtab{
  unsigned long* tab;
  unsigned len;
} Bigtab;


static void * fusionsort(void *_bigtab)
{
  thread_t th, th2;
  int err;
  void *res = NULL, *res2 = NULL;
  unsigned i, j1, j2;
  Bigtab* bigtab = (Bigtab*) _bigtab;

  /* on passe un peu la main aux autres pour eviter de faire uniquement la partie gauche de l'arbre */
  thread_yield();

  if (bigtab->len < 2){
    return (void*) bigtab;
  }
  
  Bigtab bigtab1, bigtab2;
  bigtab1.len = (bigtab->len)/2;
  bigtab2.len = bigtab->len - bigtab1.len;
  bigtab1.tab = &(bigtab->tab[0]);
  bigtab2.tab = &(bigtab->tab[bigtab1.len]);

  err = thread_create(&th, fusionsort, (void*)(&bigtab1));
  assert(!err);
  err = thread_create(&th2, fusionsort, (void*)(&bigtab2));
  assert(!err);
  
  err = thread_join(th, &res);
  assert(!err);
  err = thread_join(th2, &res2);
  assert(!err);

  j1=0; j2=0;
  
  Bigtab bigtabTmp;
  bigtabTmp.tab = malloc(bigtab->len * sizeof(unsigned long));
  
  for(i=0; i<bigtab->len; i++){
	if(j1 == bigtab1.len){
	  bigtabTmp.tab[i] = bigtab2.tab[j2++];
	  continue;
	}
	if(j2 == bigtab2.len){
	  bigtabTmp.tab[i] = bigtab1.tab[j1++];
	  continue;
	}
	if(bigtab1.tab[j1] < bigtab2.tab[j2]){
	  bigtabTmp.tab[i] = bigtab1.tab[j1++];
	  continue;
	}
	bigtabTmp.tab[i] = bigtab2.tab[j2++];
  }
  
  for(i=0; i<bigtab->len; i++) bigtab->tab[i] = bigtabTmp.tab[i];
  free(bigtabTmp.tab);
  
  return (void*) bigtab;
}

int main(int argc, char *argv[])
{
  srand(time(NULL));
  unsigned long n;

  if (argc < 2) {
    printf("argument manquant: entier x pour lequel calculer bigsum(x)\n");
    return -1;
  }
  n = atoi(argv[1]);
  
  Bigtab bigtab;
  bigtab.tab = malloc(n*sizeof(unsigned long));
  bigtab.len = n;
  unsigned i;
  for(i=0; i<n; i++){
	  //bigtab.tab[i] = (unsigned long)(rand()%n); // Random tab
	  bigtab.tab[i] = (unsigned long)(n-i); // Decreasing tab
  }
  
  fusionsort((void *)(&bigtab));
  printf("fusion sort de %ld:\n", n);
  
  for(i=0; i<bigtab.len; i++){
	printf("%ld ", bigtab.tab[i]);
  }
  printf("\n");
  
  free(bigtab.tab);

  return 0;
}
