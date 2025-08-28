# Specification (version 0.1.0)

## Bytecode

Single-byte instruction words appear in the following format:

`1 b a a o o o o`

If the least-significant-bit is not a 1, it is interpreted as a "halt" 
instruction, stopping the machine. This way, uninitialized values in ROM 
(zeroes, by default) will halt execution.

If bit `b` is 1, the opcode's arguments are popped, and then a single-byte
boolean flag is popped afterwards. If that flag byte is zero, the opcode's 
arguments are restored to the stack (minus the flag byte), and the opcode is 
skipped.

`a` is the variable argument byte counter, taking values from 1 to 4.

`o` refers to the opcode, one of 16 stack-manipulation instructions:

arg* => "arg" is a variable amount of bytes

argW => "arg" is the machine's word size (in # of bytes)

arg(1-4) => takes 1-4 bytes

- `add` ( a* b* -- a+b* ) add 2 stack values
- `and` ( a* b* -- a&b* ) bitwise-and two values
- `xor` ( a* b* -- a^b* ) xor two values
- `shf` ( n* s1 -- m* ) logically shift a value- high nibble is left shift, low nibble is right shift, with right shift occurring first
- `cmp` ( a* b* -- cmp1 ) cmp = 0 if equal, 1 if a > b, 255 if a < b
- `swp` ( a* b* -- b* a* ) swap top 2 values
- `str` ( data* addrW -- ) store data at RAM[addr]
- `lod` ( addrW -- data* ) fetch data at RAM[addr]
- `dup` ( n* -- n* n* ) duplicate top value
- `drp` ( n* --  ) discard top value
- `psh` ( data* -- ) push data to return stack
- `pop` ( -- data* ) pop data from return stack
- `jmp` ( dest* -- ) jump to dest
- `lit` ( -- val* ) push immediate bytes
- `int` ( id* -- stat* ) device call/interrupt (see below)
- `nop` ( -- ) no operation, regardless of `a` flag (although `b` flag still works)

### The "int" opcode

`int` pops a byte off of the stack and interprets it as a device ID, where the
high nibble refers to which device is being called, and the low nibble is an
"opcode" (0-15) that gets passed as an argument to that device function. 

At most 16 device functions can be registered to the VM at a time.

Device functions are C functions that have the following signature:

```c
unsigned char device_function(OkVM* vm, unsigned char op);
```

Where "op" is the low nibble of the byte that you pass to the `int` instruction.
You don't *have* to use the `op` argument if you don't want to. The byte that 
is returned by the device function gets pushed back onto the VM stack after the
function is called, and you can define it however you wish.

If `int` tries to call a device that doesn't exist, the VM will panic.

You can call multiple devices with a single `int` instruction, using the `aa`
bytecode flags.

## Note on memory and the stack

Integers are stored in *big-endian* order in RAM and ROM, and multi-byte integers
on the stack have their low (most significant) byte closest to the top.
