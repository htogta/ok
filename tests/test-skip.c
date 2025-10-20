#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// instruction defines go here
#define LIT1 (0b10001101)
#define ADD1_SKIP (0b11000000)
#define AND1 (0b10000001)

// program mem goes here
static uint8_t program[] = {
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
  assert(ok_dst_pop(&vm, 1) == (3 & 9));

  printf("...test-skip PASSED\n");
  free(ram);
  return 0;
}
