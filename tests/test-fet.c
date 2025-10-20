#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// instruction defines go here
#define FET1 (0b10001110)
#define LIT3 (0b10101101)

// program mem goes here
static uint8_t program[] = {
  LIT3,
  0,
  0,
  6,
  FET1,
  0,
  69, // we'll be reading this one
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
  assert(ok_dst_pop(&vm, 1) == 69);

  printf("...test-fet PASSED\n");
  free(ram);
  return 0;
}
