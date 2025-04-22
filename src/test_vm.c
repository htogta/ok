#include "okvm.h"
#include <stdio.h>
#include <assert.h>


void test_multibyte();
void test_jmp();
// void test_syn(); TODO
// void test_skip(); TODO

int main() {
  printf("Testing vm...\n");
  
  test_multibyte();
  test_jmp();

  printf("Done.\n");
  
  return 0;
}

// TESTS BELOW:

// defining some opcodes for test_multibyte
#define LIT2 (0b10011101)
#define JMP2 (0b10011100)
#define ASB2 (0b10010000)

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
  vm_init(&vm, program, 8);

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

#define LIT1 (0b10001101)
#define JMP (0b10001100)

void test_jmp() {
  unsigned char program[4] = {
    LIT1,
    0x55, // 85 in decimal
    JMP,
    0
  };

  OkVM vm;
  vm_init(&vm, program, 4);

  vm.status = VM_RUNNING;
  while (vm.status == VM_RUNNING) {
    vm_tick(&vm);
  }
  
  assert(vm.pc == 0x56); // should halt one after 

  vm_free(&vm);
  
  printf("...test_jmp PASSED\n");
}
