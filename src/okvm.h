#ifndef OKVM_H
#define OKVM_H

#include "okstack.h"
#include <stddef.h>

#define OKVM_WORD_SIZE (3)

// some magic numbers:
#define PORT_STDOUT (0xbabe)
#define PORT_STDERR (0xbeef)

typedef enum {
  OKVM_RUNNING,
  OKVM_HALTED,
  OKVM_PANIC, // fatal error, eg divide by 0
} OkVM_status;

typedef struct {
  OkStack dst; // data stack
  OkStack rst; // return stack
  size_t pc; // program counter
  unsigned char* ram;
  unsigned char* rom;
  OkVM_status status;
} OkVM;

void okvm_init(OkVM* vm, unsigned char* program, size_t rom_size); // NOTE: allocates memory!
void okvm_init_from_file(OkVM* vm, char* filepath);
OkVM_status okvm_tick(OkVM* vm);
void okvm_free(OkVM* vm);

#endif
