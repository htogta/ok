#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>

void test_multibyte();
void test_be_stack();
void test_shf();
void test_jmp();
void test_skip();
void test_skip_lit();
void test_nop();
void test_lod();
void test_str();
void test_int_stdout();

int main() {
  printf("Testing vm...\n");
  
  test_multibyte();
  test_be_stack();
  test_shf();
  test_jmp();
  test_skip();
  test_skip_lit();
  test_nop();
  test_lod();
  test_str();
  test_int_stdout();
  // TODO test return stack?

  printf("Done.\n");
  
  return 0;
}

// TESTS BELOW:

// defining some opcodes for test_multibyte
#define LIT2 (0b10011101)
#define JMP2 (0b10011100)
#define ADD2 (0b10010000)

// testing asb for multi-byte ints
void test_multibyte() {
  uint8_t program[8] = {
    LIT2, // lit2
    0x09, // 0915 or 2325
    0x15, 
    LIT2, // lit2
    0x0b, // 0bab or 2987  
    0xab, 
    ADD2, // add2
    0 // halt
  };

  OkVM vm;
  okvm_init(&vm, program, 8);

  vm.status = OK_RUNNING;
  while (vm.status == OK_RUNNING) {
    okvm_tick(&vm);
  }
  
  // 5312 should now be on the stack top:
  assert(stack_popn(&vm, 2) == 5312);
  //printf("Stack top: %d\n", stack_popn(&(vm.dst), 2));
  
  okvm_free(&vm);

  printf("...test_multibyte PASSED\n");
}

// we doin big-endian now, coz then values are stored on the stack 
// in a way that makes more sense.
void test_be_stack() {
  uint8_t program[] = {
    LIT2,
    0x01, // 0x0102 is on the stack, i.e. 258
    0x02,
    LIT2,
    0x01,
    0x02,
    0
  };

  OkVM vm;
  okvm_init(&vm, program, sizeof(program));
  
  vm.status = OK_RUNNING;
  while (vm.status == OK_RUNNING) {
    okvm_tick(&vm);
  }

  // unsigned short top = stack_popn(&(vm.dst), 2);
  assert(stack_popn(&vm, 2) == 0x102); // make sure we got the right number
  
  // popping one byte of that value should give me the LEAST significant byte,
  // i.e. 0x02
  assert(stack_pop(&vm) == 0x02);
  // then 0x01
  assert(stack_pop(&vm) == 0x01);

  okvm_free(&vm);

  printf("...test_big_endian_stack PASSED\n");
}

// instructions for test_jmp
#define LIT1 (0b10001101)
#define JMP (0b10001100)

void test_jmp() {
  uint8_t program[4] = {
    LIT1,
    0x55, // 85 in decimal
    JMP,
    0
  };

  OkVM vm;
  okvm_init(&vm, program, 4);

  vm.status = OK_RUNNING;
  while (vm.status == OK_RUNNING) {
    okvm_tick(&vm);
  }
  
  assert(vm.pc == 0x56); // should halt one after 

  okvm_free(&vm);
  
  printf("...test_jmp PASSED\n");
}

// new instructions for test_skip
#define ADD1_SKIP (0b11000000)
#define AND1 (0b10000001)

void test_skip() {
  uint8_t program[10] = {
    LIT1,
    0, // stack is now [0]
    LIT1,
    0x09,
    LIT1,
    0x03, // stack is [0, 9, 3] <- top
    ADD1_SKIP, // pop 9 and 3, then pop 0, since 0, restore 9 and 3 and skip add1
    AND1, // run AND1 instead, stack is now [3&9]
    0
  };
  
  OkVM vm;
  okvm_init(&vm, program, 10);

  vm.status = OK_RUNNING;
  while (vm.status == OK_RUNNING) {
    okvm_tick(&vm);
  }

  assert(stack_pop(&vm) == (3 & 9));

  okvm_free(&vm);

  printf("...test_skip PASSED\n");
}

#define LIT2_SKIP (0b11011101)

// gave it its own test case, because lit is a bit weird
void test_skip_lit() {
  // lit2_skip should skip both the "lit" instruction AND its byte args 
  // when the top is not 255

  uint8_t program[8] = {
    LIT1, // push 69
    69,
    LIT1, // push 0 
    0, 
    LIT2_SKIP, // all this gets skipped, straight to halt
    0xcd,
    0xab,
    0 // should halt with 69 on the stack top
  }; // "#69 #00 #abcd?"

  OkVM vm;
  okvm_init(&vm, program, 8);

  vm.status = OK_RUNNING;
  while (vm.status == OK_RUNNING) {
    okvm_tick(&vm);
  }

  uint8_t top = stack_pop(&vm);
  assert(top == 69);

  okvm_free(&vm);

  printf("...test_skip_lit PASSED\n");
}

#define NOP (0b10001111)

void test_nop() {
  uint8_t program[] = {
    NOP, 
    NOP,
    0
  };

  OkVM vm;
  okvm_init(&vm, program, sizeof(program));

  vm.status = OK_RUNNING;
  while (vm.status == OK_RUNNING) {
    okvm_tick(&vm);
  }

  assert(vm.dsp == 0);
  assert(vm.pc == 3);

  okvm_free(&vm);

  printf("...test_nop PASSED\n");
}

#define STR2 (0b10010110)
#define LOD2 (0b10010111)
#define LIT3 (0b10101101)

void test_lod() {
  uint8_t program[6] = {
    LIT3,
    0,
    0x03,
    0xe7,
    LOD2,
    0
  };

  OkVM vm;
  okvm_init(&vm, program, 6);

  // pre-set RAM[999] to 0xdead
  vm.ram[0x3e7] = 0xde; // note 0x03e7 is 999 in hex 
  vm.ram[1000] = 0xad;

  vm.status = OK_RUNNING;
  while (vm.status == OK_RUNNING) {
    okvm_tick(&vm);
  }

  uint32_t top = stack_popn(&vm, 2);
  // printf("top = %4x\n", top); fflush(stdout);
  assert(top == 0xdead);

  okvm_free(&vm);

  printf("...test_lod PASSED\n");
}

void test_str() {
  uint8_t program[9] = {
    LIT2,
    0xab,
    0xcd,
    LIT3,
    0,
    0,
    69,
    STR2, // store 0xabcd at RAM[69]
    0
  };

  OkVM vm;
  okvm_init(&vm, program, 9);

  vm.status = OK_RUNNING;
  while (vm.status == OK_RUNNING) {
    okvm_tick(&vm);
  }

  // printf("ram[69] = %4x\n", vm.ram[69]); fflush(stdout);
  assert(vm.ram[69] == 0xab);
  assert(vm.ram[70] == 0xcd); // NOTE big-endian in RAM

  okvm_free(&vm);

  printf("...test_str PASSED\n");
}

#define SHF2 (0b10010011)
#define RIGHT4_LEFT3 (0b00110100)

void test_shf() {
  uint8_t program[] = {
    LIT2,
    0x01, // 258 on the stack (0x102)
    0x02,
    LIT1,
    RIGHT4_LEFT3,
    SHF2,
    0
  };

  OkVM vm;
  okvm_init(&vm, program, sizeof(program));

  vm.status = OK_RUNNING;
  while (vm.status == OK_RUNNING) {
    okvm_tick(&vm);
  }

  unsigned short top = (unsigned short) stack_popn(&vm, 2);
  // printf("TOP = %d\n", top); fflush(stdout);
  assert(top == ((258 >> 4) << 3));

  okvm_free(&vm);

  printf("...test_shf PASSED\n");
}

#define STR1 (0b10000110)
#define INT1 (0b10001110)
#define SERIAL_OUT_ID (0b00000000)

// serial output device function:
uint8_t serial_output(OkVM* vm, uint8_t op) {
  assert(op < 16);
  
  if (op == 0) {
    char ch = vm->ram[0x00babe];
    putchar(ch);
    return (uint8_t) ch;
  } else {
    return 255;
  }
}

void test_int_stdout() {
  printf("Here is an \"at\" symbol: <");
  
  uint8_t program[13] = {
    LIT1,
    0x40, // @ char
    LIT3,
    0x00,
    0xba,
    0xbe,
    STR1, // write to magic number
    LIT1,
    SERIAL_OUT_ID,
    INT1, // flush to output
    0
  };

  OkVM vm;
  okvm_init(&vm, program, 13);

  // registering serial device
  int reg_success = okvm_register_device(&vm, serial_output);
  assert(reg_success == 0);

  vm.status = OK_RUNNING;
  while (vm.status == OK_RUNNING) {
    okvm_tick(&vm);
  }
  
  assert(vm.status == OK_HALTED);
  
  uint8_t top = stack_pop(&vm);
  // '@' should have been pushed onto the stack
  assert(top == 0x40);

  okvm_free(&vm);

  // no need for assert, just look in the standard output.
  printf(">\n...test_int_stdout PASSED\n");
}
