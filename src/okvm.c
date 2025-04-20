#include "okvm.h"
#include <stdlib.h>

// TODO maybe refactor codebase to just define stack data structure in here?
// or maybe even as macros rather than separate functions?

// pop 1-4 bytes as a 32-bit int
static unsigned int stack_popn(OkStack* stack, unsigned char n) {
  unsigned int out = 0;
  for (size_t i = 0; i < n; i++) {
    out = (out << 8) | stack_pop(stack);
  }
  
  return out;
}

static void stack_pushn(OkStack* stack, unsigned char n, unsigned int val) {
  for (int i = 0; i < n; i++) {
    unsigned char byte = (unsigned char) ((val >> (8 * i)) & 0xFF);
    stack_push(stack, byte);
  }
}

void vm_init(OkVM* self, unsigned char* program) { // NOTE: allocates memory!
  self->dst = stack_init();
  self->rst = stack_init();
  self->pc = 0;
  self->status = VM_HALTED;
  self->rom = program;

  // handle word size
  self->ram = (unsigned char*) calloc(1 << (VM_WORD_SIZE * 8), sizeof(unsigned char)); 
}

void vm_free(OkVM* self) {
  free(self->ram); // only this needs to be freed,
  // because only this is on the heap
}

static unsigned char fetch(OkVM* vm) {
  unsigned char instr = vm->rom[vm->pc];
  vm->pc++;
  return instr;
}

static void handle_opcode(OkVM* vm, unsigned char opcode, unsigned char argsize) {

  unsigned int a, b;
  if (opcode >= 0 && opcode <= 5) {
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
        vm->status = VM_PANIC; // should this panic?
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
      addr = (size_t) stack_popn(&(vm->dst), VM_WORD_SIZE);
      unsigned int data = stack_popn(&(vm->dst), argsize + 1);
      for (size_t i = 0; i < argsize + 1; i++) {
        vm->ram[addr + i] = stack_pop(&(vm->dst)); 
      }
      break;
    case 7: // lod
      addr = (size_t) stack_popn(&(vm->dst), VM_WORD_SIZE);
      for (size_t i = 0; i < argsize + 1; i++) {
        stack_push(&(vm->dst), vm->ram[addr + i]);
      }
      break;
    case 8: // dup
      n = stack_popn(&(vm->dst), argsize + 1);
      stack_pushn(&(vm->dst), argsize + 1, n);
      stack_pushn(&(vm->dst), argsize + 1, n);
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
      for (size_t i = 0; i < argsize + 1; i++) {
        stack_push(&(vm->dst), fetch(vm));
      }
    case 14: // syn TODO
      // this guy's complicated. It should pop an address,
      // and then use that as the start of a hardcoded "buffer" 
      // for that device.

      // So if the framebuffer starts at 0xAABB, for example,
      // you would run `syn` on 0xAABB to iterate over all bytes
      // in the framebuffer (starting at 0xAABB) and render
      // them all to the "screen".
      
      // input is treated as a completely different device,
      // with a different address for its "buffer".

      // this way you get the simplicity of memory-mapped I/O,
      // but it's easy to also hook this part up to an emulator.

      // all device addresses should be stored in the range 
      // RAM[0] to RAM[(16 * WORD_SIZE) - 1]
      // allowing for up to 16 devices total,
      // so for a 32-bit machine, that's addresses 0x00 to 0x3f.
      // 24-bit machine, that's 0x00 to 0x2f
      // 16-bit machine, that's 0x00 to 0x1f
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
          // push program counter
          stack_pushn(&(vm->dst), VM_WORD_SIZE, (unsigned char) vm->pc);
          break;
        case 3:
          // push machine word size in bytes (1-4)
          stack_push(&(vm->dst), VM_WORD_SIZE);
          break;
      }
      break;
  }
}

static void execute(OkVM* vm, unsigned char instr) {
  // instruction binary format: 1baaoooo

  // handling 1 at start
  if ((instr & 0b10000000) == 0) {
    vm->status = VM_HALTED;
    return;
  }

  // handling "b" -> the skip flag
  if ((instr & 0b01000000) != 0) {
    unsigned char top = stack_pop(&(vm->dst));
    if (top != 0b11111111) {
      return;
    }
  }

  // decoding "a" -> the argument size flag
  unsigned char argsize = (instr & 0b00110000) >> 4;

  // decoding opcodes
  unsigned char opcode = instr & 0x000f;
  handle_opcode(vm, opcode, argsize); // split this off for brevity
}

OkVM_status vm_tick(OkVM* self) {
  // fetch and exec current instr
  unsigned char instr = fetch(self);
  execute(self, instr);
  // TODO handle I/O?
  
  return self->status;
}
