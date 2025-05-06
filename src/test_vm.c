#define OKVM_IMPLEMENTATION
#include "okvm.h"
#include <stdio.h>
#include <assert.h>

void test_multibyte();
void test_jmp();
void test_skip();
void test_skip_lit();
void test_dbg();
void test_lod();
void test_str(); 
void test_syn_stdout();

int main() {
  printf("Testing vm...\n");
  
  test_multibyte();
  test_jmp();
  test_skip();
  test_skip_lit();
  test_dbg();
  test_lod();
  test_str();
  test_syn_stdout();

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
  okvm_init(&vm, program, 8);

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) {
    okvm_tick(&vm);
  }
  
  // 5312 should now be on the stack top:
  assert(stack_popn(&(vm.dst), 2) == 5312);
  //printf("Stack top: %d\n", stack_popn(&(vm.dst), 2));

  // and then 662 should be on the stack top:
  assert(stack_popn(&(vm.dst), 2) == 662);
  //printf("Stack top: %d\n", stack_popn(&(vm.dst), 2));
  
  okvm_free(&vm);

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
  okvm_init(&vm, program, 4);

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) {
    okvm_tick(&vm);
  }
  
  assert(vm.pc == 0x56); // should halt one after 

  okvm_free(&vm);
  
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
    0, // stack is now [9, 3, 0]
    ASB1_SKIP, // pop, since 0, skip ASB1
    MXR1, // run MXR instead, stack is now [3^9, 3*9]
    DRP1, // drop 3*9, top should be 3^9
    0
  };
  
  OkVM vm;
  okvm_init(&vm, program, 10);

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) {
    okvm_tick(&vm);
  }

  assert(stack_pop(&(vm.dst)) == (3 ^ 9));

  okvm_free(&vm);

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
    0,
    LIT2_SKIP, // all this gets skipped, straight to halt
    0xcd,
    0xab,
    0 // should halt with 69 on the stack top
  };

  OkVM vm;
  okvm_init(&vm, program, 8);

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) {
    okvm_tick(&vm);
  }

  unsigned char top = stack_pop(&(vm.dst));
  assert(top == 69);

  okvm_free(&vm);

  printf("...test_skip_lit PASSED\n");
}

#define DBG_SP (0b10001111)
#define DBG_RP (0b10011111)
#define DBG_PC (0b10101111)
#define DBG_WORDSIZE (0b10111111)
#define PSH1 (0b10001010)

void test_dbg() {
  unsigned char program[8] = {
    DBG_WORDSIZE, // should push "3"
    DBG_PC, // should push "1" as a WORD
    LIT1,
    69,
    PSH1, // 69 on top of return stack
    DBG_RP, // should push "1"
    DBG_SP, // should push "1 + 1 + (OKVM_WORD_SIZE)" I think
    0
  };

  // so after running, stack should be (descending, so top is top):
  // 5 (or 2 + OKVM_WORD_SIZE)
  // 1
  // 0x010000
  // 3

  OkVM vm;
  okvm_init(&vm, program, 8);

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) {
    okvm_tick(&vm);
  }

  unsigned char maybe_sp = stack_pop(&(vm.dst));
  unsigned char maybe_rp = stack_pop(&(vm.dst));
  unsigned int maybe_pc = stack_popn(&(vm.dst), OKVM_WORD_SIZE);
  unsigned char maybe_wordsize = stack_pop(&(vm.dst));
  
  assert(maybe_sp == (2 + OKVM_WORD_SIZE));
  assert(maybe_rp == vm.rst.sp);
  assert(maybe_rp == 1); // 1 should also work, testing just in case
  assert(((size_t) maybe_pc) == 1);
  assert(maybe_wordsize = OKVM_WORD_SIZE);
  
  okvm_free(&vm);

  printf("...test_dbg PASSED\n");
}

#define STR2 (0b10010110)
#define LOD2 (0b10010111)
#define LIT3 (0b10101101)

void test_lod() {
  unsigned char program[6] = {
    LIT3,
    0xe7,
    0x03,
    0,
    LOD2,
    0
  };

  OkVM vm;
  okvm_init(&vm, program, 6);

  // pre-set RAM[999] to 0xdead
  vm.ram[0x03e7] = 0xad; // note 0x03e7 is 999 in hex 
  vm.ram[1000] = 0xde;

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) {
    okvm_tick(&vm);
  }

  unsigned int top = stack_popn(&(vm.dst), 2);
  assert(top == 0xdead);

  okvm_free(&vm);

  printf("...test_lod PASSED\n");
}

void test_str() {
  unsigned char program[9] = {
    LIT2,
    0xcd,
    0xab,
    LIT3,
    69,
    0,
    0,
    STR2, // store 0xabcd at RAM[69]
    0
  };

  OkVM vm;
  okvm_init(&vm, program, 9);

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) {
    okvm_tick(&vm);
  }

  // printf("RAM[69] = 0x%02x\n", vm.ram[69]);
  assert(vm.ram[69] == 0xcd);
  assert(vm.ram[70] == 0xab); // NOTE little-endian in RAM ofc

  okvm_free(&vm);

  printf("...test_str PASSED\n");
}

#define STR1 (0b10000110)
#define SYN1 (0b10001110)
#define SERIAL_OUT_ID (0b10001000)

// serial output device function:
unsigned char serial_output(OkVM* vm) {
  char ch = vm->ram[0x00babe];
  putchar(ch);
  return (unsigned char) ch;
}

void test_syn_stdout() {
  printf("Here is an \"at\" symbol: <");
  
  unsigned char program[13] = {
    LIT1,
    0x40, // @ char
    LIT3,
    0xbe,
    0xba,
    0x00,
    STR1, // write to magic number
    LIT1,
    SERIAL_OUT_ID,
    SYN1, // flush to output
    0
  };

  OkVM vm;
  okvm_init(&vm, program, 13);

  // registering serial device
  int reg_success = okvm_register_device(&vm, SERIAL_OUT_ID, serial_output);
  assert(reg_success == 0);

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) {
    okvm_tick(&vm);
  }
  
  assert(vm.status == OKVM_HALTED);
  
  unsigned char top = stack_pop(&(vm.dst));
  // '@' should have been pushed onto the stack
  assert(top == 0x40);

  okvm_free(&vm);

  // no need for assert, just look in the standard output.
  printf(">\n...test_syn_stdout PASSED\n");
}
