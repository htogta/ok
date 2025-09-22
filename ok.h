#ifndef OK_H
#define OK_H

#include <stdint.h>
#include <stddef.h>

#define OK_WORD_SIZE (3)

#define OK_MAX_DEVICES (16)

#define OK_VERSION ("0.1.0")

typedef enum {
  OK_RUNNING,
  OK_HALTED,
  OK_PANIC, // fatal error, eg divide by 0
} OkVM_status;

// type for VM device function pointers
typedef uint8_t (*OkDevice)(uint8_t*, uint8_t*);

// forward-declaring OkVM 
typedef struct OkVM {
  uint8_t dsp; // data stack pointer
  uint8_t dst[256]; // data stack
  uint8_t rsp; // return stack pointer
  uint8_t rst[256]; // return stack
  size_t pc; // program counter
  size_t num_devices; // track number of registered devices
  uint8_t device_ports[OK_MAX_DEVICES];
  OkDevice devices[OK_MAX_DEVICES]; // dev fn ptrs
  uint8_t* ram;
  uint8_t* rom;
  OkVM_status status;
} OkVM;

// NOTE: vm_init functions allocate memory!
int okvm_init(OkVM* vm, uint8_t* program, size_t rom_size);
int okvm_init_from_file(OkVM* vm, const char* filepath);

// "devices" are just callback functions that mutate the state of the VM
int okvm_register_device(OkVM* vm, OkDevice device_fn, uint8_t port);
OkVM_status okvm_tick(OkVM* vm);

// this just frees the VM RAM and ROM, everything else (should) just be on
// the stack.
void okvm_free(OkVM* vm);

#ifdef OK_IMPLEMENTATION

// main (data) stack handling
static inline void stack_push(OkVM* vm, uint8_t i) {
  vm->dst[vm->dsp] = i;
  vm->dsp++;
}

static inline uint8_t stack_pop(OkVM* vm) {
  uint8_t out = vm->dst[vm->dsp - 1];
  vm->dst[vm->dsp--] = 0;
  return out;
}

// pop 1-4 bytes as a 32-bit int, top of stack is the low byte
static inline uint32_t stack_popn(OkVM* vm, uint8_t n) {
  uint32_t out = 0;
  for (int i = 0; i < n; i++) {
    out |= (uint32_t)stack_pop(vm) << (8 * i);
  }
  
  return out;
}

// push 1-4 bytes of a 32-bit int
static inline void stack_pushn(OkVM* vm, uint8_t n, uint32_t val) {
  for (int i = n - 1; i >= 0; i--) {
    uint8_t byte = (uint8_t) ((val >> (8 * i)) & 0xFF);
    stack_push(vm, byte);
  }
}

// return stack handling
static inline void rstack_push(OkVM* vm, uint8_t i) {
  vm->rst[vm->rsp] = i;
  vm->rsp++;
}

static inline uint8_t rstack_pop(OkVM* vm) {
  uint8_t out = vm->rst[vm->rsp - 1];
  vm->rst[vm->rsp--] = 0;
  return out;
}

// pop 1-4 bytes as a 32-bit int, top of stack is the low byte
static inline uint32_t rstack_popn(OkVM* vm, uint8_t n) {
  uint32_t out = 0;
  for (int i = 0; i < n; i++) {
    out |= (uint32_t)rstack_pop(vm) << (8 * i);
  }
  
  return out;
}

// push 1-4 bytes of a 32-bit int
static inline void rstack_pushn(OkVM* vm, uint8_t n, uint32_t val) {
  for (int i = n - 1; i >= 0; i--) {
    uint8_t byte = (uint8_t) ((val >> (8 * i)) & 0xFF);
    rstack_push(vm, byte);
  }
}

// vm implementation

#include <stdlib.h>
#include <string.h>
// stdlib for calloc, string for memcpy

#include <stdio.h>
// for file handling in okvm_init_from_file

#define OK_MEM_SIZE (1 << (OK_WORD_SIZE * 8))

// initialize an instance of the VM (ALLOCATES MEMORY !!!)
// return nonzero if failed
int okvm_init(OkVM* vm, uint8_t* program, size_t rom_size) { // NOTE: allocates memory!
  for (size_t i = 0; i < 256; i++) { // zero-init stacks
    vm->dst[i] = 0;
    vm->rst[i] = 0;
  }
  
  vm->pc = 0;
  vm->num_devices = 0;
  vm->status = OK_HALTED;

  // ensure devices array filled with nulls
  for (int i = 0; i < OK_MAX_DEVICES; i++) {
    vm->devices[i] = NULL;
  }

  // allocate vm ram (TODO don't allocate it all at once? It's a lot)
  vm->ram = (uint8_t*) calloc(OK_MEM_SIZE, sizeof(uint8_t));
  if (vm->ram == NULL) return 1;
  
  // allocate vm rom (TODO see note above)
  vm->rom = (uint8_t*) calloc(OK_MEM_SIZE, sizeof(uint8_t));
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
  if (file_size > (1 << (OK_WORD_SIZE * 8))) return 1;

  // file bytes are read directly into ROM
  vm->rom = (uint8_t*) calloc(OK_MEM_SIZE, sizeof(uint8_t));
  if (vm->rom == NULL) return 1;
  
  size_t bytes_read = fread(vm->rom, sizeof(uint8_t), file_size, fptr);
  if (bytes_read < file_size) return 1;

  fclose(fptr); // now we're done with the file stuff

  // now the other vm init stuff happens
  for (size_t i = 0; i < 256; i++) { // zero-init stacks
    vm->dst[i] = 0;
    vm->rst[i] = 0;
  }
  vm->pc = 0;
  vm->num_devices = 0;
  vm->status = OK_HALTED;

  // ensure devices array filled with nulls
  for (int i = 0; i < OK_MAX_DEVICES; i++) {
    vm->devices[i] = NULL;
  }

  // allocate vm ram (TODO don't allocate it all at once? It's a lot)
  vm->ram = (uint8_t*) calloc(OK_MEM_SIZE, sizeof(uint8_t));
  if (vm->ram == NULL) return 1;

  return 0; // success!
}

// NOTE: nonzero exit code means failure
int okvm_register_device(OkVM* vm, OkDevice device_fn, uint8_t port) {  
  // ensure that we have registered less than 16 devices
  if (vm->num_devices >= OK_MAX_DEVICES) return 1;

  // now register it!
  vm->device_ports[vm->num_devices] = port;
  vm->devices[vm->num_devices] = device_fn;
  vm->num_devices++;
  
  return 0; // return 0 upon success
}

static inline uint8_t fetch(OkVM* vm) {
  return vm->rom[vm->pc++];
}

static void execute(OkVM* vm, uint8_t instr);

// one clock cycle of the VM
OkVM_status okvm_tick(OkVM* vm) {
  // fetch and exec current instr
  execute(vm, fetch(vm));
  
  return vm->status;
}

// free heap memory of the vm
void okvm_free(OkVM* vm) {
  free(vm->ram);
  free(vm->rom);
}

// triggering a device with its port in the zero-page
static void trigger_device(OkVM* vm, uint8_t port) {
  int port_found = 0; // used as a bool to see if we find it
  size_t dev_index = 0;
  for (size_t i = 0; i < vm->num_devices; i++) {
    if (vm->device_ports[i] == port) {
      port_found = 1;
      dev_index = i;
      break;
    }
  }

  // if port not found, panic and return
  if (!port_found) {
    vm->status = OK_PANIC;
    return;
  }

  if (vm->devices[dev_index] == NULL) { // if it somehow is null
    vm->status = OK_PANIC;
    return;
  } else {
    OkDevice device = vm->devices[dev_index];
    uint8_t result = device(vm->ram, vm->rom);
    
    // push the result onto the stack
    stack_push(vm, result);
  }
}

static void handle_opcode(OkVM* vm, uint8_t opcode, uint8_t argsize, uint8_t skip_flag) {
  
  // pre-declaring these
  uint32_t a, b;
  size_t addr;
  uint32_t n;
  uint8_t byte;
  uint8_t buff[4] = {0}; // using this just for the `int` opcode

  switch (opcode) {
    case 0: // add
      b = stack_popn(vm, argsize + 1);
      a = stack_popn(vm, argsize + 1);
      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          stack_pushn(vm, argsize + 1, a + b);
        } else {
          stack_pushn(vm, argsize + 1, a);
          stack_pushn(vm, argsize + 1, b);
        }
      } else {
        stack_pushn(vm, argsize + 1, a + b);
      }
      break;
    case 1: // and
      b = stack_popn(vm, argsize + 1);
      a = stack_popn(vm, argsize + 1);
      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          stack_pushn(vm, argsize + 1, a & b);
        } else {
          stack_pushn(vm, argsize + 1, a);
          stack_pushn(vm, argsize + 1, b);
        }
      } else {
        stack_pushn(vm, argsize + 1, a & b);
      }
      break;
    case 2: // xor
      b = stack_popn(vm, argsize + 1);
      a = stack_popn(vm, argsize + 1);
      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          stack_pushn(vm, argsize + 1, a ^ b);
        } else {
          stack_pushn(vm, argsize + 1, a);
          stack_pushn(vm, argsize + 1, b);
        }
      } else {
        stack_pushn(vm, argsize + 1, a ^ b);
      }
      break;
    case 3: // shf
      byte = stack_pop(vm);
      n = stack_popn(vm, argsize + 1);

      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          n = n >> (byte & 0x0f); // right shift first
          n = n << ((byte & 0xf0) >> 4); // then left
          stack_pushn(vm, argsize + 1, n);
        } else {
          stack_pushn(vm, argsize + 1, n);
          stack_push(vm, byte);
        }
      } else {
        n = n >> (byte & 0x0f); // right shift first
        n = n << ((byte & 0xf0) >> 4); // then left
        stack_pushn(vm, argsize + 1, n);
      }
      break;
    case 4: // swp
      b = stack_popn(vm, argsize + 1);
      a = stack_popn(vm, argsize + 1);

      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          // i.e. push b, then push a
          stack_pushn(vm, argsize + 1, b);
          stack_pushn(vm, argsize + 1, a);
        } else {
          stack_pushn(vm, argsize + 1, a);
          stack_pushn(vm, argsize + 1, b);
        }
      } else {
        // i.e. push b, then push a
        stack_pushn(vm, argsize + 1, b);
        stack_pushn(vm, argsize + 1, a);
      }
      break;
    case 5: // cmp
      b = stack_popn(vm, argsize + 1);
      a = stack_popn(vm, argsize + 1);

      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          if (a > b) {
            stack_push(vm, 1);
          } else if (a < b) {
            stack_push(vm, 255);
          } else {
            stack_push(vm, 0);
          }
        } else { // restore args
          stack_pushn(vm, argsize + 1, a);
          stack_pushn(vm, argsize + 1, b);
        }
      } else {
        if (a > b) {
          stack_push(vm, 1);
        } else if (a < b) {
          stack_push(vm, 255);
        } else {
          stack_push(vm, 0);
        }
      }
      break;
    case 6: // str
      addr = (size_t) stack_popn(vm, OK_WORD_SIZE);
      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          for (int i = argsize; i >= 0; i--) {
            vm->ram[addr + i] = stack_pop(vm); 
          }
        } else { // restore
          stack_pushn(vm, OK_WORD_SIZE, addr);
        }
      } else {
        for (int i = argsize; i >= 0; i--) {
          vm->ram[addr + i] = stack_pop(vm); 
        }
      }
      break;
    case 7: // lod
      addr = (size_t) stack_popn(vm, OK_WORD_SIZE);
      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          for (int i = 0; i < argsize + 1; i++) {
            stack_push(vm, vm->ram[addr + i]);
          }
        } else { // restore
          stack_pushn(vm, OK_WORD_SIZE, addr);
        }
      } else {
        for (int i = 0; i < argsize + 1; i++) {
          stack_push(vm, vm->ram[addr + i]);
        }
      }
      break;
    case 8: // dup
      n = stack_popn(vm, argsize + 1);

      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          stack_pushn(vm, argsize + 1, n);
          stack_pushn(vm, argsize + 1, n);
        } else { // restore
          stack_pushn(vm, argsize + 1, n);
        }
      } else {
        stack_pushn(vm, argsize + 1, n);
        stack_pushn(vm, argsize + 1, n);
      }
      break;
    case 9: // drp (pop from stack)
      n = stack_popn(vm, argsize + 1);

      if (skip_flag) {
        if (stack_pop(vm) == 0) {
          stack_pushn(vm, argsize + 1, n);
        } // this one's a lot simpler
      }
      break;
    case 10: // psh (push onto return stack)
      n = stack_popn(vm, argsize + 1);

      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          rstack_pushn(vm, argsize + 1, n);
        } else { // restore
          stack_pushn(vm, argsize + 1, n);
        }
      } else {
        rstack_pushn(vm, argsize + 1, n);
      }
      break;
    case 11: // pop (off of return stack)
      n = rstack_popn(vm, argsize + 1);

      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          stack_pushn(vm, argsize + 1, n);
        } else { // restore
          rstack_pushn(vm, argsize + 1, n);
        }
      } else {
        stack_pushn(vm, argsize + 1, n);
      }
      break;
    case 12: // jmp
      addr = (size_t) stack_popn(vm, argsize + 1);
      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          vm->pc = addr;
        } else { // restore
          stack_pushn(vm, argsize + 1, addr);
        }
      } else {
        vm->pc = addr;
      }
      break;
    case 13: // lit
      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          for (int i = 0; i < argsize + 1; i++) {
            stack_push(vm, fetch(vm));
          }
        } else {
          // we gotta skip the args in the ROM as well
          for (int i = 0; i < argsize + 1; i++) { fetch(vm); }
        }
      } else {
        for (int i = 0; i < argsize + 1; i++) {
          stack_push(vm, fetch(vm));
        }
      }
      break;
    case 14: // int
      for (int i = 0; i < argsize + 1; i++) {
        buff[i] = stack_pop(vm);
      }
    
      if (skip_flag) {
        if (stack_pop(vm) != 0) {
          for (int i = 0; i < argsize + 1; i++) {
            trigger_device(vm, buff[i]);
          }
        } else { // restore
          for (int i = argsize; i >= 0; i--) {
            stack_push(vm, buff[i]);
          }
        }
      } else {
        for (int i = 0; i < argsize + 1; i++) {
          trigger_device(vm, buff[i]);
        }
      }
    
      break;
    case 15: // nop
      if (skip_flag) {
        // even tho it's meaningless, skip flag here can still pop a flag byte
        stack_pop(vm);
      }
      break;
  }
}

static void execute(OkVM* vm, uint8_t instr) {
  // instruction binary format: 1baaoooo

  // handling 1 at start
  if ((instr & 0b10000000) == 0) {
    vm->status = OK_HALTED;
    return;
  }
  
  // decoding "a" -> the argument size flag
  uint8_t argsize = (instr & 0b00110000) >> 4;
  
  // decoding opcodes
  uint8_t opcode = instr & 0x000f;

  // split this off for brevity
  handle_opcode(vm, opcode, argsize, (instr & 0b01000000) != 0); 
}


#endif // OKVM_IMPLEMENTATION

#endif // OKVM_H
