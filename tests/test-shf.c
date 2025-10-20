#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// instruction defines go here
#define SHF2 (0b10010011)
#define RIGHT4_LEFT3 (0b00110100)
#define LIT2 (0b10011101)
#define LIT1 (0b10001101)

// program mem goes here
static uint8_t program[] = {
  LIT2,
  0x01, // 258 on the stack (0x102)
  0x02,
  LIT1,
  RIGHT4_LEFT3,
  SHF2,
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
  unsigned short top = (unsigned short) ok_dst_pop(&vm, 2);
  // printf("TOP = %d\n", top); fflush(stdout);
  assert(top == ((258 >> 4) << 3));

  printf("...test-shf PASSED\n");
  free(ram);
  return 0;
}
