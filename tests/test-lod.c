#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// instruction defines go here
#define LOD2 (0b10010111)
#define LIT3 (0b10101101)

// program mem goes here
static uint8_t program[] = {
  LIT3,
  0,
  0x03,
  0xe7,
  LOD2,
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

  // pre-set RAM[999] to 0xdead
  ram[0x3e7] = 0xde; // note 0x03e7 is 999 in hex 
  ram[1000] = 0xad;
  
  OkState vm;
  ok_init(&vm);
  while (vm.status == OK_RUNNING) ok_tick(&vm);

  // test assertions go here
  uint32_t top = ok_dst_pop(&vm, 2);
  // printf("top = %4x\n", top); fflush(stdout);
  assert(top == 0xdead);

  printf("...test-lod PASSED\n");
  free(ram);
  return 0;
}
