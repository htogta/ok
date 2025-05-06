# ok - A Minimal Stack-Based Virtual Machine

This repository contains the specification and reference implementation for the 
`ok` (virtual) computer.

> WARNING: this virtual computer and its specification are still in beta; expect limited documentation and potential non-backwards-compatible updates in the future.

## Including as a library

The VM is implemented as a single C header, so creating your own minimal emulator
is very easy. Here is an example:

```c

#define OKVM_IMPLEMENTATION
#include "okvm.h"
// okvm.h includes stdio already, so we don't need to include it again

// device functions must have this signature
unsigned char serial_output(OkVM* vm);

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("usage: okmin file.rom\n");
    return 1;
  }

  OkVM vm;

  // NOTE: allocates memory for RAM and ROM
  int init_failed = okvm_init_from_file(&vm, argv[1]);
  if (init_failed) return 1;

  // the binary is the device flag, least significant bit must be 1
  int reg_failed = okvm_register_device(&vm, 0b10000000, serial_output);
  if (reg_failed) return 1;

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) okvm_tick(&vm);

  okvm_free(&vm); // this frees the vm RAM and ROM

  return vm.status != OKVM_HALTED;
}

// our device function just prints the byte at a specific address
unsigned char serial_output(OkVM* vm) {
  char c = vm->ram[0xbabe];
  putchar(c);
  return c;
}


```

## Running examples/tests

If you have [just](https://github.com/casey/just) installed, simply clone this 
repository, navigate to the repository folder, and run `just build-example` to
build the example emulator.

Run tests using `just test`.

## Roadmap

- [x] 256-byte circular stack implementation
- [x] circular stack unit tests
- [x] Basic VM instruction execution
- [x] Basic VM unit tests
- [x] Minimal output device handling (`stdout` and `stderr`)
- [x] Basic CLI stuff (reading ROM from file, `okmin`, etc)
- [x] Our first assembler! (see [moka](https://codeberg.org/hitogata/moka))
- [x] Testing assembled ROM files
- [x] Flesh-out emulator output device specification (i.e. improve `syn`)
- [x] Single-header implementation
- [ ] Compilation flags for different device word sizes
- [ ] SDL backend for graphical emulator


Note: the assembler currently in development is not a "standard" for how the 
`ok` computer's assembly language is supposed to look- feel free to experiment
with creating your own assembly languages for this VM.

## Goals

`ok` is a simple stack-based virtual machine designed with the following goals 
in mind:

- Ease of implementation
- Ease of language targeting
- Minimalism
- Expressiveness

`ok` aspires to be useful for educational purposes (such as teaching low-level 
concepts as well as compiler development) 

`ok` also aspires to run well on older or more constrained hardware, and can
be configured to use as little as 2.25 KiB of memory (combined between stacks, 
RAM, and instruction memory). 

## Features

- Two 512-byte circular stacks
- Up to 4 GiB of virtual RAM and 4 GiB of instruction memory
- Configurable word sizes
- 16 primitive stack operations

## More Info

See `SPECIFICATION.md` for the VM specification (instruction set, 
memory model, etc.)

The VM has no device input or output instructions, so device memory is directly 
mapped to values of RAM. This is because the base virtual machine specification 
is the only "canonical" part of the `ok` computing stack- creating your own 
unique assemblers, compilers, emulators, and specific device specifications is 
encouraged.

See also:

- This repository on [Codeberg](https://codeberg.org/hitogata/ok)
- This repository on [GitHub](https://github.com/goneal26/ok)
