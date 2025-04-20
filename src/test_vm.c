#include "okvm.h"
#include <stdio.h>
#include <assert.h>

// defining some opcodes for debug purposes
#define LIT2 (0b10011101)
#define ASB2 (0b10010000)

unsigned int stack_popn(OkStack* stack, unsigned char n) {
  assert(n >= 1); assert(n <= 4); // 1-4 bytes ONLY for pop

  unsigned int out = 0;
  for (size_t i = 0; i < n; i++) {
    out = (out << 8) | stack_pop(stack);
  }
  return out;
}

int main() {
  printf("Testing vm...\n");
  
  unsigned char program[8] = {
    LIT2, // lit2
    0x15, 
    0x09, // 0915 or 2325
    LIT2, // lit2
    0xab, 
    0x0b, // 0bab or 2987  
    ASB2, // asb
    0 // halt
  };

  OkVM vm;
  vm_init(&vm, program);

  vm.status = VM_RUNNING;
  while (vm.status == VM_RUNNING) {
    vm_tick(&vm);
  }

  // 5312 should now be on the stack top:
  printf("Stack top: %d\n", stack_popn(&(vm.dst), 2));

  // and then 662 should be on the stack top:
  printf("Stack top: %d\n", stack_popn(&(vm.dst), 2));
  
  vm_free(&vm);

  printf("Done.\n");
  
  return 0;
}
