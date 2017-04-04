#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h> /* ne compile pas avec -std=c89 ou -std=c99 */

void func(int numero)
{
  printf("j'affiche le numero %d\n", numero);
}

void test1()
{
  ucontext_t uc, previous;

  getcontext(&uc); /* initialisation de uc avec valeurs coherentes
		    * (pour eviter de tout remplir a la main ci-dessous) */
  
  uc.uc_stack.ss_size = 64*1024;//modif taille de la pile (1024 est une puissance de 2)
  uc.uc_stack.ss_sp = malloc(uc.uc_stack.ss_size);//alloue la pile sur le tas
  uc.uc_link = &previous;//etat de retour
  makecontext(&uc, (void (*)(void)) func, 1, 34);//definit nouveau contexte comme execution de func

  printf("je suis dans le main\n");
  swapcontext(&previous, &uc);//save context courant dans previous et execute le context uc
  printf("je suis revenu dans le main\n");

  uc.uc_stack.ss_size = 64*1024;
  uc.uc_stack.ss_sp = malloc(uc.uc_stack.ss_size);
  uc.uc_link = NULL;//arrete le programme lorsque le contexte uc est termine
  makecontext(&uc, (void (*)(void)) func, 1, 57);

  //free(uc.uc_stack.ss_sp);

  printf("je suis dans le main\n");
  setcontext(&uc);
  printf("je ne reviens jamais ici\n");
}

void test2()
{
  ucontext_t uc, previous, mid;

  getcontext(&uc); /* initialisation de uc avec valeurs coherentes
		    * (pour eviter de tout remplir a la main ci-dessous) */
  getcontext(&mid);

  mid.uc_stack.ss_size = 64*1024;
  mid.uc_stack.ss_sp = malloc(mid.uc_stack.ss_size);
  mid.uc_link = &previous;  
  makecontext(&mid, (void (*)(void)) func, 1, 42);

  uc.uc_stack.ss_size = 64*1024;//modif taille de la pile (1024 est une puissance de 2)
  uc.uc_stack.ss_sp = malloc(uc.uc_stack.ss_size);//alloue la pile sur le tas
  uc.uc_link = &mid;//etat de retour
  makecontext(&uc, (void (*)(void)) func, 1, 34);//definit nouveau contexte comme execution de func

  printf("je suis dans le main\n");
  swapcontext(&previous, &uc);//save context courant dans previous et execute le context uc
  printf("je suis revenu dans le main\n");

  uc.uc_stack.ss_size = 64*1024;
  uc.uc_stack.ss_sp = malloc(uc.uc_stack.ss_size);
  uc.uc_link = NULL;//arrete le programme lorsque le contexte uc est termine
  makecontext(&uc, (void (*)(void)) func, 1, 57);

  printf("je suis dans le main\n");
  setcontext(&uc);//lance le contexte uc (et ne retourne JAMAIS)
  printf("je ne reviens jamais ici\n");
}


int main(){
  //test1();
  test2();
  return 0;
}
