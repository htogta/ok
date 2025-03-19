# pkbk

## bytecode

`1 b a a o o o o`

b => if 1, pop top byte, if that top byte is not 255 skip current instruction
a => variable argument counter
o => opcode

If the first bit is not a 1, it is interpreted as a "halt" instruction, stopping the machine.

### opcodes and stack effects

arg* => "arg" is a variable amount of bytes
argW => "arg" is the machine's word size (in # of bytes)
arg(1-4) => takes 1-4 bytes

`lod` ( addrW -- data* ) fetch data at RAM[addr]
`str` ( data* addrW -- ) store data at RAM[addr]
`asb` ( a* b* -- b+a* b-a* ) add and subtract
`dmd` ( a* b* -- b%a* b/a* ) divide and modulo
`aor` ( a* b* -- b&a* b|a* ) bitwise and & bitwise or
`mxr` ( a* b* -- bxa* b^a* ) multiply and xor
`swp` ( a* b* -- b* a* ) swap top 2 values
`shf` ( n* -- n>>1* n<<1* ) left and right shift
`cmp` ( a* b* -- gl1 eq1) eq1 = 255 if a and b are equal, otherwise 0. If eq1 is 0, gl1 will be 255 if b>a, otherwise 0.
`dup` ( n* -- n* n* ) duplicate top value
`drp` ( n* --  ) discard top value
`psh` ( data* -- ) push data to return stack
`pop` ( -- data* ) pop data from return stack
`jmp` ( dest* -- ) jump to dest
`cal` ( dest* -- origW ) jump to dest and push original location
`lit` ( -- val* ) push immediate bytes

## assembly language features

All assembly instructions are simply the opcode names shown above, except for `lit`.
To push a literal, use `#` followed by some number of bytes as case-insensitive hexadecimal numbers:
`#ff` pushes a single byte, 255.
`#00ff` pushes two bytes, but still just the number 255, only with leading zeroes.
The assembly language uses big-endian number literals, so `#00ff` will result in `ff` or 255 being at the top of the stack.

As a result, numbers are stored on the stack with the most-significant-byte closest to the top.

Prepend with a `?` to set the `b` flag:
`?jmp` = conditional jump if top is 255

Append a number from 1 to 4 for variable args:
`cmp1` compares 2 single byte arguments 
`cmp2` compares 2 two-byte arguments, etc.

Appending no number implies a variable-argument size of however many bytes are 
in a "word", i.e. the size of a pointer into RAM or ROM. 

Note that byte literals start with a # followed by 2 hex digits (case insensitive)

### FORTH-style compilation mode

Of course, we have compilation mode like FORTH:

`: add asb drp ;` = defines a word "add" which adds the top 2 bytes.
`: sub asb swp drp ;` = defines a word "sub" which subtracts the top 2 bytes.

### Labels

You can declare a label with `(labelname)`

To push that label's index in ROM onto the stack (as a word), do `.labelname`

Labels declared in compilation mode can act as "relative" labels:

`: test (lbl) .lbl pop1 ?jmp ;`

However, when a label is declared in compilation mode, that label can only
be referenced in the same instance of compilation mode. So, the following does
not work:

`: labelhere (lbl) add ; .lbl`
