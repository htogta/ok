default: build-example

test: test-vm test-helpers

test-helpers:
  cc tests/test-helpers.c -o tests/test-helpers
  ./tests/test-helpers

test-vm:
  cc tests/test-vm.c -o tests/test-vm
  ./tests/test-vm

build-example:
  cc examples/okmin.c -o examples/okmin
