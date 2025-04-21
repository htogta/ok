#include "okvm.h"
#include <stdio.h>
#include <assert.h>

// defining some opcodes for debug purposes
#define LIT2 (0b10011101)
#define ASB2 (0b10010000)
#define JMP2 (0b10011100)

void test_multibyte();
// void test_jmp(); TODO
// void test_syn(); TODO
// void test_skip(); TODO

int main() {
  printf("Testing vm...\n");
  
  test_multibyte();  

  printf("Done.\n");
  
  return 0;
}

// TESTS BELOW:

// testing asb for multi-byte ints
void test_multibyte() {
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
  assert(stack_popn(&(vm.dst), 2) == 5312);
  //printf("Stack top: %d\n", stack_popn(&(vm.dst), 2));

  // and then 662 should be on the stack top:
  assert(stack_popn(&(vm.dst), 2) == 662);
  //printf("Stack top: %d\n", stack_popn(&(vm.dst), 2));
  
  vm_free(&vm);

  printf("...test_multibyte PASSED\n");
}
