# Changelog

Here are the latest changes from the previous version, many of which are backward-incompatible.

- removed divide, sub, bitwise-or, multiply, and modulo to be implemented later in a separate "Math" device or through pseudo-instructions
- added simple `add`, `and`, `xor`, and `shf` as the primary math opcodes
- reworked `cmp` opcode to push 0, 1, or 255 rather than 2 separate bytes
- renamed `syn` instruction to `int` (for "interrupt")
- renamed `dbg` instruction to `sys` (for "sysinfo")
- abandoned the old assembly language (`moka`) and began implementing a new assembler in C
