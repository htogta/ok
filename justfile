default: build-example

test:
  cc tests/test-vm.c -o tests/test-vm
  ./tests/test-vm

build-example:
  cc examples/okmin.c -o examples/okmin
