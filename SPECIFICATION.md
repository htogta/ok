# Specification (version 0.1.0)

## Bytecode

Single-byte instruction words appear in the following format:

`1 b a a o o o o`

If the least-significant-bit is not a 1, it is interpreted as a "halt" 
instruction, stopping the machine. This way, uninitialized values in ROM 
(zeroes, by default) will halt execution.

If bit `b` is 1, pop the top byte on the stack before executing the opcode. If 
that top byte is zero, skip the current instruction.

`a` is the variable argument byte counter, taking values from 1 to 4.

`o` refers to the opcode, one of 16 stack-manipulation instructions:

arg* => "arg" is a variable amount of bytes

argW => "arg" is the machine's word size (in # of bytes)

arg(1-4) => takes 1-4 bytes

- `asb` ( a* b* -- b-a* b+a* ) add and subtract
- `dmd` ( a* b* -- b%a* b/a* ) divide and modulo
- `aor` ( a* b* -- b|a* b&a* ) bitwise and & bitwise or
- `mxr` ( a* b* -- b^a* bxa* ) multiply and xor
- `swp` ( a* b* -- b* a* ) swap top 2 values
- `cmp` ( a* b* -- gl1 eq1 ) eq1 = 255 if a and b are equal, otherwise 0. If eq1 is 0, gl1 will be 255 if b>a, otherwise 0.
- `str` ( data* addrW -- ) store data at RAM[addr]
- `lod` ( addrW -- data* ) fetch data at RAM[addr]
- `dup` ( n* -- n* n* ) duplicate top value
- `drp` ( n* --  ) discard top value
- `psh` ( data* -- ) push data to return stack
- `pop` ( -- data* ) pop data from return stack
- `jmp` ( dest* -- ) jump to dest
- `lit` ( -- val* ) push immediate bytes
- `syn` ( id* -- stat* ) see below 

`syn` pops multiple bytes off of the stack, interpreting them as unique IDs for
VM devices. "Devices" are really just C functions that modify the state of the 
VM itself and return a single-byte status code- that way the VM can easily
call C functions.

For each single-byte device ID, its corresponding "device function" is called,
and the status byte that gets returned by that function gets pushed onto the
stack.

Device IDs and device functions are defined by the user- the device ID byte 
must have the following form:

`1 a a a a a a a`

The `a` bits can refer to whatever you wish. 

> Note: If `syn` pops a device ID that is invalid or it does not recognize, the VM exits with a panic.

> Note: Devices must be unique- you cannot register multiple devices with the same ID.

- `dbg` ( -- debug ) the "debug" instruction pushes debug values to the stack

This instruction, rather than using `aa` to determine argument size, uses `aa` 
to determine which arg to push onto stack:

0 = stack pointer (one byte)

1 = return pointer (one byte)

2 = current program counter value (one word)

3 = machine word size in bytes (one byte)

> Note: division or modulo by 0 returns 0, and sets the VM to panic.
