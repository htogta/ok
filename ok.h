#ifndef OK_H
#define OK_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
  OK_RUNNING, // currently executing
  OK_HALTED, // halted normally
  OK_PANIC, // halted abnormally
} OkStatus;

typedef struct {
  uint8_t d; // data stack pointer
  uint8_t dst[256]; // circular data stack
  uint8_t r; // return stack pointer
  uint8_t rst[256]; // circular return stack
  size_t pc; // program counter
  OkStatus status; // current VM status
} OkState;

// useful constants
#define OK_WORD_SIZE (3) // word size in bytes
#define OK_MEM_SIZE (1 << (8 * OK_WORD_SIZE)) // default maximum memory

// memory reading prototypes; these are implemented by the person making the
// VM's emulator
extern uint8_t ok_mem_read(size_t address); // get RAM
extern void ok_mem_write(size_t address, uint8_t val); // set RAM
extern uint8_t ok_fetch(size_t address); // get instruction

// initialize the VM
void ok_init(OkState* s);

// cycle the VM clock
OkStatus ok_tick(OkState* s);

// some helper functions that the user may use for fetching big-endian
// values from byte buffers (RAM or program memory)
uint32_t ok_get_bytes(uint8_t* buffer, size_t index, uint8_t amt);
void ok_set_bytes(uint8_t* buffer, size_t index, uint8_t amt, uint32_t val);

// this helper function opens a file at a path and loads it into a byte buffer
// it loads the bytes from the file into the buffer, with the file's first byte
// at buffer[start]. Returns nonzero upon failure.
int ok_load_file(uint8_t* buffer, size_t start, const char* filepath);

#ifdef OK_IMPLEMENTATION // VM implementation

#include <stdio.h> // for ok_load_file

// helper functions for reading/writing values in buffers

// get an amt-wide value at index
uint32_t ok_get_bytes(uint8_t* buffer, size_t index, uint8_t amt) {
  uint32_t out = 0;

  for (uint8_t i = 0; i < amt; i++) {
    out = (out << 8) | buffer[index + i];
  }

  return out;
}

// set buffer at index to val, where val is amt bytes wide
void ok_set_bytes(uint8_t* buffer, size_t index, uint8_t amt, uint32_t val) {
  for (int i = amt - 1; i >= 0; i--) {
    buffer[index + i] = (uint8_t) (val & 0xff);
    val >>= 8;
  }
}

// helper function for loading a file at a path
int ok_load_file(uint8_t* buffer, size_t start, const char* filepath) {
  FILE* f = fopen(filepath, "rb");
  if (!f) return 1;

  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return 1;
  }
  long filesize = ftell(f);
  if (filesize < 0) {
    fclose(f);
    return 1;
  }
  rewind(f);

  // check bounds, we know the buffer is at most OK_MEM_SIZE bytes
  if ((size_t)filesize + start > OK_MEM_SIZE) {
    fclose(f);
    return 1;
  }

  // read file into buffer at offset "start"
  size_t read = fread(buffer + start, 1, filesize, f);
  fclose(f);

  if (read != (size_t)filesize) return 1;

  return 0;
}

// circular stack functions
static inline void ok_dst_push(OkState* s, uint8_t n, uint32_t val) {
  for (int i = n - 1; i >= 0; i--) {
    s->dst[s->d] = (uint8_t) ((val >> (8 * i)) & 0xFF);;
    s->d++;
  }
}

static inline uint32_t ok_dst_pop(OkState* s, uint8_t n) {
  uint32_t out = 0;
  for (int i = 0; i < n; i++) {
    uint8_t byte = s->dst[s->d - 1];
    s->dst[s->d--] = 0;
    out |= (uint32_t) byte << (8 * i);
  }

  return out;
}

static inline void ok_rst_push(OkState* s, uint8_t n, uint32_t val) {
  for (int i = n - 1; i >= 0; i--) {
    s->rst[s->r] = (uint8_t) ((val >> (8 * i)) & 0xFF);;
    s->r++;
  }
}

static inline uint32_t ok_rst_pop(OkState* s, uint8_t n) {
  uint32_t out = 0;
  for (int i = 0; i < n; i++) {
    uint8_t byte = s->rst[s->r - 1];
    s->rst[s->r--] = 0;
    out |= (uint32_t) byte << (8 * i);
  }

  return out;
}

// VM initialization
void ok_init(OkState* s) {
  s->d = 0;
  s->r = 0;
  s->pc = 0;
  s->status = OK_RUNNING;
  for (int i = 0; i < 256; i++) {
    s->dst[i] = 0;
    s->rst[i] = 0;
  }
}

static void execute(OkState* s, uint8_t instr);

// cycle the VM clock
OkStatus ok_tick(OkState* s) {
  execute(s, ok_fetch(s->pc++));
  return s->status;
}

// TODO this could be DRAMATICALLY simplified
static void handle_opcode(OkState* vm, uint8_t opcode, uint8_t arg, uint8_t skip) {
  
  // pre-declaring these
  uint32_t a, b;
  size_t addr;
  uint32_t n;
  uint8_t byte;

  switch (opcode) {
    case 0: // add
      b = ok_dst_pop(vm, arg + 1);
      a = ok_dst_pop(vm, arg + 1);
      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          ok_dst_push(vm, arg + 1, a + b);
        } else {
          ok_dst_push(vm, arg + 1, a);
          ok_dst_push(vm, arg + 1, b);
        }
      } else {
        ok_dst_push(vm, arg + 1, a + b);
      }
      break;
    case 1: // and
      b = ok_dst_pop(vm, arg + 1);
      a = ok_dst_pop(vm, arg + 1);
      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          ok_dst_push(vm, arg + 1, a & b);
        } else {
          ok_dst_push(vm, arg + 1, a);
          ok_dst_push(vm, arg + 1, b);
        }
      } else {
        ok_dst_push(vm, arg + 1, a & b);
      }
      break;
    case 2: // xor
      b = ok_dst_pop(vm, arg + 1);
      a = ok_dst_pop(vm, arg + 1);
      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          ok_dst_push(vm, arg + 1, a ^ b);
        } else {
          ok_dst_push(vm, arg + 1, a);
          ok_dst_push(vm, arg + 1, b);
        }
      } else {
        ok_dst_push(vm, arg + 1, a ^ b);
      }
      break;
    case 3: // shf
      byte = (uint8_t) ok_dst_pop(vm, 1);
      n = ok_dst_pop(vm, arg + 1);

      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          n = n >> (byte & 0x0f); // right shift first
          n = n << ((byte & 0xf0) >> 4); // then left
          ok_dst_push(vm, arg + 1, n);
        } else {
          ok_dst_push(vm, arg + 1, n);
          ok_dst_push(vm, 1, byte);
        }
      } else {
        n = n >> (byte & 0x0f); // right shift first
        n = n << ((byte & 0xf0) >> 4); // then left
        ok_dst_push(vm, arg + 1, n);
      }
      break;
    case 4: // swp
      b = ok_dst_pop(vm, arg + 1);
      a = ok_dst_pop(vm, arg + 1);

      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          // i.e. push b, then push a
          ok_dst_push(vm, arg + 1, b);
          ok_dst_push(vm, arg + 1, a);
        } else {
          ok_dst_push(vm, arg + 1, a);
          ok_dst_push(vm, arg + 1, b);
        }
      } else {
        // i.e. push b, then push a
        ok_dst_push(vm, arg + 1, b);
        ok_dst_push(vm, arg + 1, a);
      }
      break;
    case 5: // cmp
      b = ok_dst_pop(vm, arg + 1);
      a = ok_dst_pop(vm, arg + 1);

      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          if (a > b) {
            ok_dst_push(vm, 1, 1);
          } else if (a < b) {
            ok_dst_push(vm, 1, 255);
          } else {
            ok_dst_push(vm, 1, 0);
          }
        } else { // restore args
          ok_dst_push(vm, arg + 1, a);
          ok_dst_push(vm, arg + 1, b);
        }
      } else {
        if (a > b) {
          ok_dst_push(vm, 1, 1);
        } else if (a < b) {
          ok_dst_push(vm, 1, 255);
        } else {
          ok_dst_push(vm, 1, 0);
        }
      }
      break;
    case 6: // str
      addr = (size_t) ok_dst_pop(vm, OK_WORD_SIZE);
      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          for (int i = arg; i >= 0; i--) {
            ok_mem_write(addr + i, ok_dst_pop(vm, 1));
          }
        } else { // restore
          ok_dst_push(vm, OK_WORD_SIZE, addr);
        }
      } else {
        for (int i = arg; i >= 0; i--) {
          ok_mem_write(addr + i, ok_dst_pop(vm, 1)); 
        }
      }
      break;
    case 7: // lod
      addr = (size_t) ok_dst_pop(vm, OK_WORD_SIZE);
      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          for (int i = 0; i < arg + 1; i++) {
            ok_dst_push(vm, 1, ok_mem_read(addr + i));
          }
        } else { // restore
          ok_dst_push(vm, OK_WORD_SIZE, addr);
        }
      } else {
        for (int i = 0; i < arg + 1; i++) {
          ok_dst_push(vm, 1, ok_mem_read(addr + i));
        }
      }
      break;
    case 8: // dup
      n = ok_dst_pop(vm, arg + 1);

      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          ok_dst_push(vm, arg + 1, n);
          ok_dst_push(vm, arg + 1, n);
        } else { // restore
          ok_dst_push(vm, arg + 1, n);
        }
      } else {
        ok_dst_push(vm, arg + 1, n);
        ok_dst_push(vm, arg + 1, n);
      }
      break;
    case 9: // drp (pop from stack)
      n = ok_dst_pop(vm, arg + 1);

      if (skip) {
        if (ok_dst_pop(vm, 1) == 0) {
          ok_dst_push(vm, arg + 1, n);
        } // this one's a lot simpler
      }
      break;
    case 10: // psh (push onto return stack)
      n = ok_dst_pop(vm, arg + 1);

      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          ok_rst_push(vm, arg + 1, n);
        } else { // restore
          ok_dst_push(vm, arg + 1, n);
        }
      } else {
        ok_rst_push(vm, arg + 1, n);
      }
      break;
    case 11: // pop (off of return stack)
      n = ok_rst_pop(vm, arg + 1);

      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          ok_dst_push(vm, arg + 1, n);
        } else { // restore
          ok_rst_push(vm, arg + 1, n);
        }
      } else {
        ok_dst_push(vm, arg + 1, n);
      }
      break;
    case 12: // jmp
      addr = (size_t) ok_dst_pop(vm, arg + 1);
      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          vm->pc = addr;
        } else { // restore
          ok_dst_push(vm, arg + 1, addr);
        }
      } else {
        vm->pc = addr;
      }
      break;
    case 13: // lit
      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          for (int i = 0; i < arg + 1; i++) {
            ok_dst_push(vm, 1, ok_fetch(vm->pc++));
          }
        } else {
          // we gotta skip the args in the ROM as well
          for (int i = 0; i < arg + 1; i++) { vm->pc++; }
        }
      } else {
        for (int i = 0; i < arg + 1; i++) {
          ok_dst_push(vm, 1, ok_fetch(vm->pc++));
        }
      }
      break;
    case 14: // fet
      addr = (size_t) ok_dst_pop(vm, OK_WORD_SIZE);
      if (skip) {
        if (ok_dst_pop(vm, 1) != 0) {
          for (int i = 0; i < arg + 1; i++) {
            ok_dst_push(vm, 1, ok_fetch(addr + i));
          }
        } else { // restore
          ok_dst_push(vm, OK_WORD_SIZE, addr);
        }
      } else {
        for (int i = 0; i < arg + 1; i++) {
          ok_dst_push(vm, 1, ok_fetch(addr + i));
        }
      }
      break;
    case 15: // nop
      if (skip) {
        // even tho it's meaningless, skip flag here can still pop a flag byte
        ok_dst_pop(vm, 1);
      }
      break;
  }
}


// decode and execute opcode
static void execute(OkState* s, uint8_t instr) {
  // handling 1 at start
  if ((instr & 0b10000000) == 0) {
    s->status = OK_HALTED;
    return;
  }
  
  // decoding "a" -> the argument size flag
  uint8_t a = (instr & 0b00110000) >> 4;
  
  // decoding opcodes
  uint8_t opcode = instr & 0x000f;

  // split this off for brevity
  handle_opcode(s, opcode, a, (instr & 0b01000000) != 0); 
}


#endif // OK_IMPLEMENTATION

#endif // OK_H
