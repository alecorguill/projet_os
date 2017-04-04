#include <stdlib.h>
#include <stdio.h>
#include "list.h"


struct Element{
  int val;
  LIST_ENTRY(Element) pointers;
};

struct List{
  LIST_HEAD(list, Element) head;
};

int main(){
  struct List l;
  LIST_INIT(&l.head);

  struct Element * item = malloc(sizeof(struct Element));
  LIST_INSERT_HEAD(&l.head, item, pointers);
  fprintf(stderr, "is_empty: %d\n", LIST_EMPTY(&l.head));
  return 0;
}
