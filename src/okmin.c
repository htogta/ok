#define OKVM_IMPLEMENTATION
#include "okvm.h"

// usage: okmin [-v] file.rom
void print_usage();
void print_version();
unsigned char serial_output(OkVM* vm);
int run_file(char* filepath);

int main(int argc, char* argv[]) {
  if (argc > 3 || argc <= 1) { // incorrect amt of args
    print_usage(); 
    return 1; // TODO proper exit codes?
  }

  // only occurs with "okmin -v file.rom"
  if (argc == 3) {
    if (strcmp(argv[1], "-v") != 0) {
      fprintf(stderr, "ERROR: unrecognized argument \"%s\"\n", argv[1]);
      print_usage();
      return 1;
    }
    print_version();

    // try to open and run path argv[2]
    return run_file(argv[2]);
  }

  if (argc == 2) {
    if (strcmp(argv[1], "-v") == 0) {
      print_version();
    } else {
      return run_file(argv[1]);
    }
  }

  return 0;
}

void print_usage() {
  printf("usage: okmin [-v] file.rom\n");
}

void print_version() {
  printf("OKMin - A minimal OK virtual machine, version 0.1.0 (beta)\n");
}

unsigned char serial_output(OkVM* vm) {
  char c = vm->ram[0xbabe];
  putchar(c);
  return (unsigned char) c;
}

// run a VM rom- returns a system exit code for main to return
int run_file(char* filepath) {
  // initialize VM struct (RAM and stacks)
  OkVM vm;
  int init_failed = okvm_init_from_file(&vm, filepath);
  if (init_failed) return 1;
  
  // device registration
  int reg_failed = okvm_register_device(&vm, 0b11000000, serial_output);
  if (reg_failed) return 1;

  vm.status = OKVM_RUNNING; // this actually starts the VM
  while (vm.status == OKVM_RUNNING) {
     okvm_tick(&vm); // one clock cycle
  }

  // okvm_init allocates RAM and ROM on the heap, so don't forget to free it
  okvm_free(&vm);

  // if you want, you can return from main based on the VM status
  return vm.status != OKVM_HALTED;
}
