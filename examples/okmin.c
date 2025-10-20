#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <stdlib.h>

// defining the buffers for the VM to use
static uint8_t* ram;
static uint8_t* program;

uint8_t ok_mem_read(size_t address) {
  return ram[address];
}

void ok_mem_write(size_t address, uint8_t val) {
  if (address == 0x00babe) putchar(val); // memory-mapped putchar
  ram[address] = val;
}

uint8_t ok_fetch(size_t address) {
  return program[address];
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("usage: okmin file.rom\n");
    return 1;
  }

  // allocate the ram and program buffers
  ram = calloc(OK_MEM_SIZE, 1);
  program = calloc(OK_MEM_SIZE, 1);
  if (!ram || !program) {
    free(ram);
    free(program);
    return 1;
  };

  // load program file into the program buffer
  if (!ok_load_file(program, 0, argv[1])) {
    free(ram);
    free(program);
    return 1; 
  }

  OkState vm;
  ok_init(&vm);
  while (vm.status == OK_RUNNING) ok_tick(&vm);

  free(ram);
  free(program);
  return vm.status != OK_HALTED;
}
