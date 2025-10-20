#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// instruction defines go here

// program mem goes here
static uint8_t program[] = {

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

  printf("...test PASSED\n");
  free(ram);
  return 0;
}
