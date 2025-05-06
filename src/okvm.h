#ifndef OKVM_H
#define OKVM_H

#include "okstack.h"
#include <stddef.h>

// TODO make this user-definable?
#define OKVM_WORD_SIZE (3)

#define OKVM_MAX_DEVICES (16)

typedef enum {
  OKVM_RUNNING,
  OKVM_HALTED,
  OKVM_PANIC, // fatal error, eg divide by 0
} OkVM_status;

// forward-declaring OkVM 
struct OkVM;

typedef struct OkVM {
  OkStack dst; // data stack
  OkStack rst; // return stack
  size_t pc; // program counter
  size_t num_devices; // track number of registered devices (TODO does this need to be size_t?)
  unsigned char device_ids[OKVM_MAX_DEVICES]; // device unique ids 
  unsigned char (*device_fns[OKVM_MAX_DEVICES]) (struct OkVM*); // device function pointers
  unsigned char* ram;
  unsigned char* rom;
  OkVM_status status;
} OkVM;

// NOTE: vm_init functions allocate memory!
int okvm_init(OkVM* vm, unsigned char* program, size_t rom_size);
int okvm_init_from_file(OkVM* vm, const char* filepath);

// "devices" are just callback functions that mutate the state of the VM
int okvm_register_device(OkVM* vm, unsigned char id, unsigned char (*fn)(OkVM*));
OkVM_status okvm_tick(OkVM* vm);

// this just frees the VM RAM and ROM, everything else (should) just be on
// the stack.
void okvm_free(OkVM* vm);

#ifdef OKVM_IMPLEMENTATION

// single-header time

#include <stdlib.h>
#include <string.h>
// stdlib for calloc, string for memcpy

#include <stdio.h>
// for file handling in okvm_init_from_file

// initialize an instance of the VM (ALLOCATES MEMORY !!!)
// return nonzero if failed
int okvm_init(OkVM* vm, unsigned char* program, size_t rom_size) { // NOTE: allocates memory!
  vm->dst = stack_init();
  vm->rst = stack_init();
  vm->pc = 0;
  vm->num_devices = 0;
  vm->status = OKVM_HALTED;

  // ensure device_ids filled with 0s
  for (int i = 0; i < OKVM_MAX_DEVICES; i++) {
    vm->device_ids[i] = 0;
  }

  // allocate vm ram (TODO don't allocate it all at once? It's a lot)
  vm->ram = (unsigned char*) calloc(1 << (OKVM_WORD_SIZE * 8), sizeof(unsigned char));
  if (vm->ram == NULL) return 1;
  
  // allocate vm rom (TODO see note above)
  vm->rom = (unsigned char*) calloc(1 << (OKVM_WORD_SIZE * 8), sizeof(unsigned char));
  if (vm->rom == NULL) return 1;

  // copy program to rom
  memcpy(vm->rom, program, rom_size);

  return 0; // success!
}

// initialize VM instance from a ROM file, returns nonzero if there's an issue
// (ALLOCATES MEMORY !!!)
int okvm_init_from_file(OkVM* vm, const char* filepath) {

  // file handling boilerplate
  FILE* fptr = fopen(filepath, "rb");
  if (fptr == NULL) return 1; // TODO exit codes?

  fseek(fptr, 0, SEEK_END);
  size_t file_size = ftell(fptr);
  rewind(fptr);

  // fail if the file's too big to fit into ROM
  if (file_size > (1 << (OKVM_WORD_SIZE * 8))) return 1;

  // file bytes are read directly into ROM
  vm->rom = (unsigned char*) calloc(1 << (OKVM_WORD_SIZE * 8), sizeof(unsigned char));
  if (vm->rom == NULL) return 1;
  
  size_t bytes_read = fread(vm->rom, sizeof(unsigned char), file_size, fptr);
  if (bytes_read < file_size) return 1;

  fclose(fptr); // now we're done with the file stuff

  // now the other vm init stuff happens
  vm->dst = stack_init();
  vm->rst = stack_init();
  vm->pc = 0;
  vm->num_devices = 0;
  vm->status = OKVM_HALTED;

  // ensure device_ids filled with 0s
  for (int i = 0; i < OKVM_MAX_DEVICES; i++) {
    vm->device_ids[i] = 0;
  }

  // allocate vm ram (TODO don't allocate it all at once? It's a lot)
  vm->ram = (unsigned char*) calloc(1 << (OKVM_WORD_SIZE * 8), sizeof(unsigned char));
  if (vm->ram == NULL) return 1;

  return 0; // success!
}

int okvm_register_device(OkVM* vm, unsigned char id, unsigned char (*fn) (OkVM*)) {
  // NOTE: nonzero exit code means failure

  // ensure that we have registered less than 16 devices
  if (vm->num_devices >= OKVM_MAX_DEVICES) return 1;

  // check if id is valid
  if ((id & 0b10000000) == 0) return 1;

  // check if id is already registered (TODO special error code?)
  int duplicate_id = 0;
  for (int i = 0; i < OKVM_MAX_DEVICES; i++) {
    if (vm->device_ids[i] == id) { duplicate_id = 1; break; }
  }
  if (duplicate_id) return 1;

  // now register it!
  // TODO ensure device_ids[] is filled with 0s at VM initialization
  vm->device_ids[vm->num_devices] = id;
  vm->device_fns[vm->num_devices] = fn;
  vm->num_devices++;
  
  return 0; // return 0 upon success
}

static unsigned char fetch(OkVM* vm);
static void execute(OkVM* vm, unsigned char instr);

// one clock cycle of the VM
OkVM_status okvm_tick(OkVM* vm) {
  // fetch and exec current instr
  unsigned char instr = fetch(vm);
  execute(vm, instr);
  
  return vm->status;
}

// free heap memory of the vm
void okvm_free(OkVM* vm) {
  free(vm->ram);
  free(vm->rom);
}

static unsigned char fetch(OkVM* vm) {
  unsigned char instr = 0;

  if (vm->pc < (1 << OKVM_WORD_SIZE * 8)) {
    instr = vm->rom[vm->pc];
  }
  
  vm->pc++;
  return instr;
}

static void trigger_device(OkVM* vm, unsigned char id) {
  // check if ID is in device_ids
  int device_index = -1;
  for (int i = 0; i < OKVM_MAX_DEVICES; i++) {
    if (vm->device_ids[i] == id) {
      device_index = i;
      break;
    }
  }

  // if still -1 (i.e. not found) panic and return
  if (device_index == -1) {
    vm->status = OKVM_PANIC;
    return;
  }
  
  // if we're here it's been found; grab its corresponding function and call it
  unsigned char (*fn)(OkVM*) = vm->device_fns[device_index];
  unsigned char result = fn(vm);

  // push the result onto the stack
  stack_push(&(vm->dst), result);
}

static void handle_opcode(OkVM* vm, unsigned char opcode, unsigned char argsize) {

  unsigned int a, b;
  if (opcode <= 5) {
    b = stack_popn(&(vm->dst), argsize + 1); // add 1 because it's 0-3
    a = stack_popn(&(vm->dst), argsize + 1);
  }
  
  // pre-declaring these
  size_t addr = 0;
  unsigned int n = 0;

  switch (opcode) {
    case 0: // asb
      stack_pushn(&(vm->dst), argsize + 1, b - a);
      stack_pushn(&(vm->dst), argsize + 1, b + a);
      break;
    case 1: // dmd
      if (a == 0) {
        stack_pushn(&(vm->dst), argsize + 1, 0);
        stack_pushn(&(vm->dst), argsize + 1, 0);
        vm->status = OKVM_PANIC; // should this panic?
      } else {
        stack_pushn(&(vm->dst), argsize + 1, b % a);
        stack_pushn(&(vm->dst), argsize + 1, b / a);
      }
      break;
    case 2: // aor
      stack_pushn(&(vm->dst), argsize + 1, b - a);
      stack_pushn(&(vm->dst), argsize + 1, b + a);
      break;
    case 3: // mxr
      stack_pushn(&(vm->dst), argsize + 1, b ^ a);
      stack_pushn(&(vm->dst), argsize + 1, b * a);
      break;
    case 4: // swp
      // i.e. push b, then push a
      stack_pushn(&(vm->dst), argsize + 1, b);
      stack_pushn(&(vm->dst), argsize + 1, a);
      break;
    case 5: // cmp
      // push gl, then push eq 
      stack_push(&(vm->dst), (b > a) ? 255 : 0);
      stack_push(&(vm->dst), (b == a) ? 255 : 0);
      break;
    case 6: // str
      addr = (size_t) stack_popn(&(vm->dst), OKVM_WORD_SIZE);
      for (int i = argsize; i >= 0; i--) {
        vm->ram[addr + i] = stack_pop(&(vm->dst)); 
      }
      break;
    case 7: // lod
      addr = (size_t) stack_popn(&(vm->dst), OKVM_WORD_SIZE);
      for (int i = 0; i < argsize + 1; i++) {
        stack_push(&(vm->dst), vm->ram[addr + i]);
      }
      break;
    case 8: // dup
      n = stack_popn(&(vm->dst), argsize + 1);
      stack_pushn(&(vm->dst), argsize + 1, n);
      stack_pushn(&(vm->dst), argsize + 1, n);
      break;
    case 9: // drp (pop from stack)
      stack_popn(&(vm->dst), argsize + 1);
      break;
    case 10: // psh (push onto return stack)
      n = stack_popn(&(vm->dst), argsize + 1);
      stack_pushn(&(vm->rst), argsize + 1, n);
      break;
    case 11: // pop (off of return stack)
      n = stack_popn(&(vm->rst), argsize + 1);
      stack_pushn(&(vm->dst), argsize + 1, n);
      break;
    case 12: // jmp
      vm->pc = (size_t) stack_popn(&(vm->dst), argsize + 1);
      break;
    case 13: // lit
      for (int i = 0; i < argsize + 1; i++) {
        stack_push(&(vm->dst), fetch(vm));
      }
      break;
    case 14: // syn
      for (int i = 0; i < argsize + 1; i++) {
        trigger_device(vm, stack_pop(&(vm->dst)));
      }
      break;
    case 15: // dbg
      switch (argsize) {
        case 0: // push stack pointer
          stack_push(&(vm->dst), (unsigned char) vm->dst.sp);
          break;
        case 1: // push return pointer
          stack_push(&(vm->dst), (unsigned char) vm->rst.sp);
          break;
        case 2:
          // push (CURRENT) program counter
          stack_pushn(&(vm->dst), OKVM_WORD_SIZE, (unsigned char) (vm->pc - 1));
          break;
        case 3:
          // push machine word size in bytes (1-4)
          stack_push(&(vm->dst), OKVM_WORD_SIZE);
          break;
      }
      break;
  }
}

static void execute(OkVM* vm, unsigned char instr) {
  // instruction binary format: 1baaoooo

  // handling 1 at start
  if ((instr & 0b10000000) == 0) {
    vm->status = OKVM_HALTED;
    return;
  }
  
  // decoding "a" -> the argument size flag
  unsigned char argsize = (instr & 0b00110000) >> 4;
  
  // decoding opcodes
  unsigned char opcode = instr & 0x000f;

  // handling "b" -> the skip flag
  if ((instr & 0b01000000) != 0) {
    unsigned char top = stack_pop(&(vm->dst));
    if (top == 0) { // if 0, skip
      if (opcode == 0b1101) { // if lit opcode, skip immediate byte args too
        for (int i = 0; i < argsize + 1; i++) { fetch(vm); }
      }
      return;
    }
  }

  handle_opcode(vm, opcode, argsize); // split this off for brevity
}


#endif // OKVM_IMPLEMENTATION

#endif // OKVM_H
