extern(C) void main()
{
  import core.stdc.stdio : printf;

	struct VM {
	  static immutable size_t STACK_SIZE = 512;
	  byte[STACK_SIZE] stack;
	  byte[STACK_SIZE] rstack;
	  size_t sp; // main stack pointer
	  size_t rp; // return stack pointer

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

	  void rpush(byte value) {
	    if (rp < STACK_SIZE) {
	      stack[rp++] = value;
	    } else {
	      printf("Stack overflow.\n");
	    }
	  }

	  byte rpop() {
	    if (rp > 0) {
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

