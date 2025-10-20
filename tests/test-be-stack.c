#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// instruction defines go here
#define LIT2 (0b10011101)

// program mem goes here
static uint8_t program[] = {
  LIT2,
  0x01, // 0x0102 is on the stack, i.e. 258
  0x02,
  LIT2,
  0x01,
  0x02,
  0
};

static uint8_t* ram;

// externally defined memory functions
uint8_t ok_mem_read(size_t address) {
  return ram[address];
}

void ok_mem_write(size_t address, uint8_t val) {
  ram[address] = val;
}

uint8_t ok_fetch(size_t address) {
  return program[address];
}

int main() {
  // allocate RAM
  ram = calloc(OK_MEM_SIZE, 1);

  OkState vm;
  ok_init(&vm);
  while (vm.status == OK_RUNNING) ok_tick(&vm);

  // test assertions go here
  // unsigned short top = stack_popn(&(vm.dst), 2);
  assert(ok_dst_pop(&vm, 2) == 0x102); // make sure we got the right number
  
  // popping one byte of that value should give me the LEAST significant byte,
  // i.e. 0x02
  assert(ok_dst_pop(&vm, 1) == 0x02);
  // then 0x01
  assert(ok_dst_pop(&vm, 1) == 0x01);

  printf("...test-be-stack PASSED\n");
  free(ram);
  return 0;
}
