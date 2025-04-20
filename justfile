# build and test the library

test-stack:
  @echo "Testing my stack..."
  @cc src/test_stack.c src/okstack.c -o tests/test_stack
  ./tests/test_stack
  @echo " done."

test-vm:
  @cc src/test_vm.c src/okvm.c src/okstack.c -o tests/test_vm
  ./tests/test_vm
  
test-all:
  test-stack
  test-vm

default: test-all
