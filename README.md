# ok - A Minimal Stack-Based Virtual Machine

This repository contains the specification and reference implementation for the 
`ok` (virtual) computer.

*WARNING*: this virtual computer and its specification are still in beta; 
expect limited documentation and potential non-backwards-compatible updates in 
the future.

## Install

If you have [just](https://github.com/casey/just) installed, simply clone this 
repository, navigate to the repository folder, and run `just build`. The 
executable `okmin` will be in the `build` folder.

Otherwise, compile it yourself with:

```sh
cc src/okmin.c src/okvm.c src/okstack.c -o build/okmin
```

## Roadmap

- [x] 256-byte circular stack implementation
- [x] circular stack unit tests
- [x] Basic VM instruction execution
- [x] Basic VM unit tests
- [x] Minimal output device handling (`stdout` and `stderr`)
- [x] Basic CLI stuff (reading ROM from file, `okmin`, etc)
- [ ] Our first assembler! (IN PROGRESS)
- [ ] Testing assembled ROM files (IN PROGERSS)
- [ ] Flesh-out emulator output device specification (i.e. improve `syn`)
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
