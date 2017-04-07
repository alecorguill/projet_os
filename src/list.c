#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>

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

  struct Element * item1 = malloc(sizeof(struct Element));
  struct Element * item2 = malloc(sizeof(struct Element));
  struct Element * item3 = malloc(sizeof(struct Element));
  LIST_INSERT_HEAD(&l.head, item1, pointers);
  LIST_INSERT_HEAD(&l.head, item2, pointers);
  LIST_INSERT_HEAD(&l.head, item3, pointers);
  fprintf(stderr, "is_empty: %d\n", LIST_EMPTY(&l.head));
  return 0;
}
