extern(C) void main()
{
  import core.stdc.stdio : printf;

	struct VM {
	  static immutable size_t STACK_SIZE = 256;
	  byte[STACK_SIZE] stack;
	  size_t sp;

	  void init() {
	    sp = 0;
	  }

	  void push(byte value) {
	    if (sp < STACK_SIZE) {
	      stack[sp++] = value;
	    } else {
	      printf("Stack overflow.\n");
	    }
	  }

	  byte pop() {
	    if (sp > 0) {
	      return stack[--sp];
	    } else {
	      printf("Stack underflow.");
	      return 0;
	    }
	  }
	}

	VM vm;
	vm.init();
	vm.push(42);
	printf("Popped: %d\n", vm.pop());
}

