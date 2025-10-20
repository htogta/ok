#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// instruction defines go here
#define STR1 (0b10000110)
#define LIT1 (0b10001101)
#define LIT3 (0b10101101)

// program mem goes here
static uint8_t program[] = {
  LIT1,
  0x40, // @ char
  LIT3,
  0x00,
  0xba,
  0xbe,
  STR1, // write to magic number
  0
};

static uint8_t* ram;

// externally defined memory functions
uint8_t ok_mem_read(size_t address) {
  return ram[address];
}

void ok_mem_write(size_t address, uint8_t val) {
  if (address == 0x00babe) putchar(val); // a simple memory-mapped 
  ram[address] = val;
}

uint8_t ok_fetch(size_t address) {
  return program[address];
}

int main() {
  // allocate RAM
  ram = calloc(OK_MEM_SIZE, 1);

  printf("Here is an \"at\" symbol: <");

  OkState vm;
  ok_init(&vm);
  while (vm.status == OK_RUNNING) ok_tick(&vm);

  // test assertions go here
  assert(ram[0x00babe] == '@');

  printf(">\n...test-putchar PASSED\n");
  free(ram);
  return 0;
}
