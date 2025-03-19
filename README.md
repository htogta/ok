# ok - A Minimal Stack-Based Virtual Machine

This repository contains the specification and reference implementation for the 
`ok` (virtual) computer.

## Features

`ok` is a simple stack-based virtual machine designed with the following goals 
in mind:

- Ease of implementation
- Ease of language targeting
- Minimalism
- Expressiveness

`ok` aspires to be useful for educational purposes (such as teaching low-level 
concepts as well as compiler development) as well as being able to run well in
constrained environments.

## More Info

See `SPECIFICATION.md` for the VM specification (instruction set, 
memory model, etc.)

The base virtual machine specification is the only "canonical" part of the `ok`
computing stack- creating your own unique assemblers, compilers, emulators, and 
specific device specifications is encouraged.
