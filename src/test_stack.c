#include <stdio.h>
#include "okstack.h"

int main() {
  printf("Testing stack...\n");

  OkStack s = stack_init();

  printf("current sp (should be 0): %d\n", s.sp);

  printf("pushing 2, 3, 4...\n");
  
  stack_push(&s, 2);
  stack_push(&s, 3);
  stack_push(&s, 4);
  
  printf("current sp (should be 3): %d\n", s.sp);

  printf("should pop 4: %d\n", stack_pop(&s));
  printf("should pop 3: %d\n", stack_pop(&s));
  printf("should pop 2: %d\n", stack_pop(&s));

  printf("current sp (should be 0): %d\n", s.sp);

  printf("should pop 0: %d\n", stack_pop(&s));
  printf("current sp (should be 255): %d\n", s.sp);
  
  return 0;
}
