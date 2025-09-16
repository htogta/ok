#ifndef OKVM_H
#define OKVM_H

#include <stdint.h>
#include <stddef.h>

// the okstack stuff
typedef struct {
  uint8_t sp;
  uint8_t data[256];
} OkStack; 

// stack handling functions (TODO rename to okstack?)
void stack_push(OkStack* s, uint8_t i);
uint8_t stack_pop(OkStack* s);
void stack_pushn(OkStack* s, uint8_t n, uint32_t val);
uint32_t stack_popn(OkStack* s, uint8_t n);
OkStack stack_init();

#define OKVM_WORD_SIZE (3)

#define OKVM_MAX_DEVICES (16)

#define OKVM_VERSION ("0.1.0")

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
  size_t num_devices; // track number of registered devices
  uint8_t (*devices[16])(struct OkVM*, uint8_t op); // dev fn ptrs
  uint8_t* ram;
  uint8_t* rom;
  OkVM_status status;
} OkVM;

// NOTE: vm_init functions allocate memory!
int okvm_init(OkVM* vm, uint8_t* program, size_t rom_size);
int okvm_init_from_file(OkVM* vm, const char* filepath);

// "devices" are just callback functions that mutate the state of the VM
int okvm_register_device(OkVM* vm, uint8_t (*fn)(OkVM*, uint8_t));
OkVM_status okvm_tick(OkVM* vm);

// this just frees the VM RAM and ROM, everything else (should) just be on
// the stack.
void okvm_free(OkVM* vm);

#ifdef OKVM_IMPLEMENTATION

// single-header time

// okstack implementation

OkStack stack_init() {
  return (OkStack){}; // pre-init values to 0s
}

inline void stack_push(OkStack* s, uint8_t i) {
  s->data[s->sp] = i;
  s->sp++;
}

inline uint8_t stack_pop(OkStack* s) {
  uint8_t out = s->data[s->sp - 1];
  s->data[s->sp--] = 0;
  return out;
}

// pop 1-4 bytes as a 32-bit int
// NOTE: the top of the stack is the LOW BYTE
inline uint32_t stack_popn(OkStack* s, uint8_t n) {
  uint32_t out = 0;
  for (int i = 0; i < n; i++) {
    out |= (uint32_t)stack_pop(s) << (8 * i);
  }
  
  return out;
}

// push 1-4 bytes of a 32-bit int
inline void stack_pushn(OkStack* s, uint8_t n, uint32_t val) {
  for (int i = n - 1; i >= 0; i--) {
    uint8_t byte = (uint8_t) ((val >> (8 * i)) & 0xFF);
    stack_push(s, byte);
  }
}

// vm implementation

#include <stdlib.h>
#include <string.h>
// stdlib for calloc, string for memcpy

#include <stdio.h>
// for file handling in okvm_init_from_file

#define OKVM_MEM_SIZE (1 << (OKVM_WORD_SIZE * 8))

// initialize an instance of the VM (ALLOCATES MEMORY !!!)
// return nonzero if failed
int okvm_init(OkVM* vm, uint8_t* program, size_t rom_size) { // NOTE: allocates memory!
  vm->dst = stack_init();
  vm->rst = stack_init();
  vm->pc = 0;
  vm->num_devices = 0;
  vm->status = OKVM_HALTED;

  // ensure devices array filled with nulls
  for (int i = 0; i < OKVM_MAX_DEVICES; i++) {
    vm->devices[i] = NULL;
  }

  // allocate vm ram (TODO don't allocate it all at once? It's a lot)
  vm->ram = (uint8_t*) calloc(OKVM_MEM_SIZE, sizeof(uint8_t));
  if (vm->ram == NULL) return 1;
  
  // allocate vm rom (TODO see note above)
  vm->rom = (uint8_t*) calloc(OKVM_MEM_SIZE, sizeof(uint8_t));
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
  vm->rom = (uint8_t*) calloc(OKVM_MEM_SIZE, sizeof(uint8_t));
  if (vm->rom == NULL) return 1;
  
  size_t bytes_read = fread(vm->rom, sizeof(uint8_t), file_size, fptr);
  if (bytes_read < file_size) return 1;

  fclose(fptr); // now we're done with the file stuff

  // now the other vm init stuff happens
  vm->dst = stack_init();
  vm->rst = stack_init();
  vm->pc = 0;
  vm->num_devices = 0;
  vm->status = OKVM_HALTED;

  // ensure devices array filled with nulls
  for (int i = 0; i < OKVM_MAX_DEVICES; i++) {
    vm->devices[i] = NULL;
  }

  // allocate vm ram (TODO don't allocate it all at once? It's a lot)
  vm->ram = (uint8_t*) calloc(OKVM_MEM_SIZE, sizeof(uint8_t));
  if (vm->ram == NULL) return 1;

  return 0; // success!
}

int okvm_register_device(OkVM* vm, uint8_t (*fn) (OkVM*, uint8_t)) {
  // NOTE: nonzero exit code means failure

  // ensure that we have registered less than 16 devices
  if (vm->num_devices >= OKVM_MAX_DEVICES) return 1;

  // now register it!
  vm->devices[vm->num_devices] = fn;
  vm->num_devices++;
  
  return 0; // return 0 upon success
}

// defining fetch as a macro, it's faster
#define FETCH(vm) ((vm)->rom[(vm)->pc++])

static void execute(OkVM* vm, uint8_t instr);

// one clock cycle of the VM
OkVM_status okvm_tick(OkVM* vm) {
  // fetch and exec current instr
  execute(vm, FETCH(vm));
  
  return vm->status;
}

// free heap memory of the vm
void okvm_free(OkVM* vm) {
  free(vm->ram);
  free(vm->rom);
}

// triggering a device with a byte id:
//   - high nibble is the device index (0-15)
//   - low nibble is the operation
static void trigger_device(OkVM* vm, uint8_t id) {
  uint8_t index = (id & 0xf0) >> 4;
  uint8_t op = (id & 0x0f);

  if (vm->devices[index] == NULL) {
    vm->status = OKVM_PANIC;
    return;
  } else {
    uint8_t (*fn)(OkVM*, uint8_t) = vm->devices[index];
    uint8_t result = fn(vm, op);
    
    // push the result onto the stack
    stack_push(&(vm->dst), result);
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
      b = stack_popn(&(vm->dst), argsize + 1);
      a = stack_popn(&(vm->dst), argsize + 1);
      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          stack_pushn(&(vm->dst), argsize + 1, a + b);
        } else {
          stack_pushn(&(vm->dst), argsize + 1, a);
          stack_pushn(&(vm->dst), argsize + 1, b);
        }
      } else {
        stack_pushn(&(vm->dst), argsize + 1, a + b);
      }
      break;
    case 1: // and
      b = stack_popn(&(vm->dst), argsize + 1);
      a = stack_popn(&(vm->dst), argsize + 1);
      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          stack_pushn(&(vm->dst), argsize + 1, a & b);
        } else {
          stack_pushn(&(vm->dst), argsize + 1, a);
          stack_pushn(&(vm->dst), argsize + 1, b);
        }
      } else {
        stack_pushn(&(vm->dst), argsize + 1, a & b);
      }
      break;
    case 2: // xor
      b = stack_popn(&(vm->dst), argsize + 1);
      a = stack_popn(&(vm->dst), argsize + 1);
      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          stack_pushn(&(vm->dst), argsize + 1, a ^ b);
        } else {
          stack_pushn(&(vm->dst), argsize + 1, a);
          stack_pushn(&(vm->dst), argsize + 1, b);
        }
      } else {
        stack_pushn(&(vm->dst), argsize + 1, a ^ b);
      }
      break;
    case 3: // shf
      byte = stack_pop(&(vm->dst));
      n = stack_popn(&(vm->dst), argsize + 1);

      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          n = n >> (byte & 0x0f); // right shift first
          n = n << ((byte & 0xf0) >> 4); // then left
          stack_pushn(&(vm->dst), argsize + 1, n);
        } else {
          stack_pushn(&(vm->dst), argsize + 1, n);
          stack_push(&(vm->dst), byte);
        }
      } else {
        n = n >> (byte & 0x0f); // right shift first
        n = n << ((byte & 0xf0) >> 4); // then left
        stack_pushn(&(vm->dst), argsize + 1, n);
      }
      break;
    case 4: // swp
      b = stack_popn(&(vm->dst), argsize + 1);
      a = stack_popn(&(vm->dst), argsize + 1);

      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          // i.e. push b, then push a
          stack_pushn(&(vm->dst), argsize + 1, b);
          stack_pushn(&(vm->dst), argsize + 1, a);
        } else {
          stack_pushn(&(vm->dst), argsize + 1, a);
          stack_pushn(&(vm->dst), argsize + 1, b);
        }
      } else {
        // i.e. push b, then push a
        stack_pushn(&(vm->dst), argsize + 1, b);
        stack_pushn(&(vm->dst), argsize + 1, a);
      }
      break;
    case 5: // cmp
      b = stack_popn(&(vm->dst), argsize + 1);
      a = stack_popn(&(vm->dst), argsize + 1);

      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          if (a > b) {
            stack_push(&(vm->dst), 1);
          } else if (a < b) {
            stack_push(&(vm->dst), 255);
          } else {
            stack_push(&(vm->dst), 0);
          }
        } else { // restore args
          stack_pushn(&(vm->dst), argsize + 1, a);
          stack_pushn(&(vm->dst), argsize + 1, b);
        }
      } else {
        if (a > b) {
          stack_push(&(vm->dst), 1);
        } else if (a < b) {
          stack_push(&(vm->dst), 255);
        } else {
          stack_push(&(vm->dst), 0);
        }
      }
      break;
    case 6: // str
      addr = (size_t) stack_popn(&(vm->dst), OKVM_WORD_SIZE);
      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          for (int i = argsize; i >= 0; i--) {
            vm->ram[addr + i] = stack_pop(&(vm->dst)); 
          }
        } else { // restore
          stack_pushn(&(vm->dst), OKVM_WORD_SIZE, addr);
        }
      } else {
        for (int i = argsize; i >= 0; i--) {
          vm->ram[addr + i] = stack_pop(&(vm->dst)); 
        }
      }
      break;
    case 7: // lod
      addr = (size_t) stack_popn(&(vm->dst), OKVM_WORD_SIZE);
      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          for (int i = 0; i < argsize + 1; i++) {
            stack_push(&(vm->dst), vm->ram[addr + i]);
          }
        } else { // restore
          stack_pushn(&(vm->dst), OKVM_WORD_SIZE, addr);
        }
      } else {
        for (int i = 0; i < argsize + 1; i++) {
          stack_push(&(vm->dst), vm->ram[addr + i]);
        }
      }
      break;
    case 8: // dup
      n = stack_popn(&(vm->dst), argsize + 1);

      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          stack_pushn(&(vm->dst), argsize + 1, n);
          stack_pushn(&(vm->dst), argsize + 1, n);
        } else { // restore
          stack_pushn(&(vm->dst), argsize + 1, n);
        }
      } else {
        stack_pushn(&(vm->dst), argsize + 1, n);
        stack_pushn(&(vm->dst), argsize + 1, n);
      }
      break;
    case 9: // drp (pop from stack)
      n = stack_popn(&(vm->dst), argsize + 1);

      if (skip_flag) {
        if (stack_pop(&(vm->dst)) == 0) {
          stack_pushn(&(vm->dst), argsize + 1, n);
        } // this one's a lot simpler
      }
      break;
    case 10: // psh (push onto return stack)
      n = stack_popn(&(vm->dst), argsize + 1);

      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          stack_pushn(&(vm->rst), argsize + 1, n);
        } else { // restore
          stack_pushn(&(vm->dst), argsize + 1, n);
        }
      } else {
        stack_pushn(&(vm->rst), argsize + 1, n);
      }
      break;
    case 11: // pop (off of return stack) TODO left off here
      n = stack_popn(&(vm->rst), argsize + 1);

      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          stack_pushn(&(vm->dst), argsize + 1, n);
        } else { // restore
          stack_pushn(&(vm->rst), argsize + 1, n);
        }
      } else {
        stack_pushn(&(vm->dst), argsize + 1, n);
      }
      break;
    case 12: // jmp
      addr = (size_t) stack_popn(&(vm->dst), argsize + 1);
      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          vm->pc = addr;
        } else { // restore
          stack_pushn(&(vm->rst), argsize + 1, addr);
        }
      } else {
        vm->pc = addr;
      }
      break;
    case 13: // lit
      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          for (int i = 0; i < argsize + 1; i++) {
            stack_push(&(vm->dst), FETCH(vm));
          }
        } else {
          // we gotta skip the args in the ROM as well
          for (int i = 0; i < argsize + 1; i++) { FETCH(vm); }
        }
      } else {
        for (int i = 0; i < argsize + 1; i++) {
          stack_push(&(vm->dst), FETCH(vm));
        }
      }
      break;
    case 14: // int
      for (int i = 0; i < argsize + 1; i++) {
        buff[i] = stack_pop(&(vm->dst));
      }
    
      if (skip_flag) {
        if (stack_pop(&(vm->dst)) != 0) {
          for (int i = 0; i < argsize + 1; i++) {
            trigger_device(vm, buff[i]);
          }
        } else { // restore
          for (int i = argsize; i >= 0; i--) {
            stack_push(&(vm->dst), buff[i]);
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
        stack_pop(&(vm->dst));
      }
      break;
  }
}

static void execute(OkVM* vm, uint8_t instr) {
  // instruction binary format: 1baaoooo

  // handling 1 at start
  if ((instr & 0b10000000) == 0) {
    vm->status = OKVM_HALTED;
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
