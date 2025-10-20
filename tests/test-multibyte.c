#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>

// instruction defines go here
#define LIT2 (0b10011101)
#define ADD2 (0b10010000)

// program mem goes here
static uint8_t program[] = {
  LIT2, // lit2
  0x09, // 0915 or 2325
  0x15, 
  LIT2, // lit2
  0x0b, // 0bab or 2987  
  0xab, 
  ADD2, // add2
  0 // halt
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
  OkState vm;
  ok_init(&vm);
  while (vm.status == OK_RUNNING) ok_tick(&vm);

  // 5312 should now be on the stack top:
  uint32_t val = ok_dst_pop(&vm, 2);
  assert(val == 5312);

  printf("...test-multibyte PASSED\n");
  return 0;
}
