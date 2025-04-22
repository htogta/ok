#include "okvm.h"
#include <stdio.h>
#include <assert.h>

void test_multibyte();
void test_jmp();
void test_skip();
void test_skip_lit();
// void test_syn(); TODO

int main() {
  printf("Testing vm...\n");
  
  test_multibyte();
  test_jmp();
  test_skip();
  test_skip_lit();

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

// instructions for test_jmp
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

// new instructions for test_skip
#define ASB1_SKIP (0b11000000)
#define MXR1 (0b10000011)
#define DRP1 (0b10001001)

void test_skip() {
  unsigned char program[10] = {
    LIT1,
    0x09,
    LIT1,
    0x03, // stack is [9, 3] <- top
    LIT1,
    33, // stack is now [9, 3, 33]
    ASB1_SKIP, // pop 33, since 33 != 255, skip ASB1
    MXR1, // run MXR instead, stack is now [3^9, 3*9]
    DRP1, // drop 3*9, top should be 3^9
    0
  };
  
  OkVM vm;
  vm_init(&vm, program, 10);

  vm.status = VM_RUNNING;
  while (vm.status == VM_RUNNING) {
    vm_tick(&vm);
  }

  assert(stack_pop(&(vm.dst)) == (3 ^ 9));

  vm_free(&vm);

  printf("...test_skip PASSED\n");
}

#define LIT2_SKIP (0b11011101)

// gave it its own test case, because lit is a bit weird
void test_skip_lit() {
  // lit2_skip should skip both the "lit" instruction AND its byte args 
  // when the top is not 255

  unsigned char program[8] = {
    LIT1, // push 69
    69,
    LIT1, // push 2 
    2,
    LIT2_SKIP, // all this gets skipped, straight to halt
    0xcd,
    0xab,
    0 // should halt with 69 on the stack top
  };

  OkVM vm;
  vm_init(&vm, program, 8);

  vm.status = VM_RUNNING;
  while (vm.status == VM_RUNNING) {
    vm_tick(&vm);
  }

  unsigned char top = stack_pop(&(vm.dst));
  assert(top == 69);

  vm_free(&vm);

  printf("...test_skip_lit PASSED\n");
}
