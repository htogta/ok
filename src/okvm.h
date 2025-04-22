#ifndef OKVM_H
#define OKVM_H

#include "okstack.h"
#include <stddef.h>

#define VM_WORD_SIZE (3)

// some magic numbers:
#define PORT_STDOUT (0xbabe)
#define PORT_STDERR (0xbeef)

typedef enum {
  VM_RUNNING,
  VM_HALTED,
  VM_PANIC, // fatal error, eg divide by 0
} OkVM_status;

typedef struct {
  OkStack dst; // data stack
  OkStack rst; // return stack
  size_t pc; // program counter
  unsigned char* ram;
  unsigned char* rom;
  OkVM_status status;
} OkVM;

void vm_init(OkVM* self, unsigned char* program, size_t rom_size); // NOTE: allocates memory!
OkVM_status vm_tick(OkVM* self);
void vm_free(OkVM* self);

#endif
