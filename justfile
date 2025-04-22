test-all: test-stack test-vm

test-stack:
  cc src/test_stack.c src/okstack.c -o tests/test_stack
  ./tests/test_stack

test-vm:
  cc src/test_vm.c src/okvm.c src/okstack.c -o tests/test_vm
  ./tests/test_vm

build-min:
  cc src/okmin.c src/okvm.c src/okstack.c -o build/okmin

build: build-min

run: build-min
  ./build/okmin
