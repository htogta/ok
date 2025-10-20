build-example:
  cc examples/okmin.c -o examples/okmin

test: test-helpers test-be-stack test-fet test-jmp test-lod test-multibyte test-nop test-putchar test-shf test-skip-lit test-skip test-str

@test-helpers:
  cc tests/test-helpers.c -o tests/test-helpers
  ./tests/test-helpers
  rm tests/test-helpers

@test-be-stack:
  cc tests/test-be-stack.c -o tests/test-be-stack
  ./tests/test-be-stack
  rm tests/test-be-stack

@test-fet:
  cc tests/test-fet.c -o tests/test-fet
  ./tests/test-fet
  rm tests/test-fet

@test-jmp:
  cc tests/test-jmp.c -o tests/test-jmp
  ./tests/test-jmp
  rm tests/test-jmp

@test-lod:
  cc tests/test-lod.c -o tests/test-lod
  ./tests/test-lod
  rm tests/test-lod

@test-multibyte:
  cc tests/test-multibyte.c -o tests/test-multibyte
  ./tests/test-multibyte
  rm tests/test-multibyte

@test-nop:
  cc tests/test-nop.c -o tests/test-nop
  ./tests/test-nop
  rm tests/test-nop

@test-putchar:
  cc tests/test-putchar.c -o tests/test-putchar
  ./tests/test-putchar
  rm tests/test-putchar

@test-shf:
  cc tests/test-shf.c -o tests/test-shf
  ./tests/test-shf
  rm tests/test-shf

@test-skip-lit:
  cc tests/test-skip-lit.c -o tests/test-skip-lit
  ./tests/test-skip-lit
  rm tests/test-skip-lit

@test-skip:
  cc tests/test-skip.c -o tests/test-skip
  ./tests/test-skip
  rm tests/test-skip

@test-str:
  cc tests/test-str.c -o tests/test-str
  ./tests/test-str
  rm tests/test-str

# TODO build example 
