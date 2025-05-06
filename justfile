default: build

test:
  cc src/test_vm.c -o tests/test_vm
  ./tests/test_vm

build:
  cc src/okmin.c -o build/okmin
