#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// instruction defines go here
#define LIT1 (0b10001101)
#define LIT2_SKIP (0b11011101)

// program mem goes here
static uint8_t program[] = {
  LIT1, // push 69
  69,
  LIT1, // push 0 
  0, 
  LIT2_SKIP, // all this gets skipped, straight to halt
  0xcd,
  0xab,
  0 // should halt with 69 on the stack top
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
  uint8_t top = ok_dst_pop(&vm, 1);
  assert(top == 69);

  printf("...test-skip-lit PASSED\n");
  free(ram);
  return 0;
}
