# build and test the library

test-stack:
  @echo "Testing my stack..."
  @cc src/test_stack.c src/okstack.c -o tests/test_stack
  ./tests/test_stack
  @echo " done."

test:
  test-stack

default: test
