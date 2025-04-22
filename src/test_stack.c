#include <stdio.h>
#include "okstack.h"
#include <assert.h>

int main() {
  printf("Testing stack... ");

  OkStack s = stack_init();

  assert(s.sp == 0);

  // pushing 2, 3, 4...
  stack_push(&s, 2);
  stack_push(&s, 3);
  stack_push(&s, 4);

  assert(s.sp == 3);

  // now for popping:
  assert(4 == stack_pop(&s));
  assert(3 == stack_pop(&s));
  assert(2 == stack_pop(&s));

  assert(s.sp == 0);

  // check underflow
  assert(stack_pop(&s) == 0);
  assert(s.sp == 255);

  printf("done.\n");
  
  return 0;
}
