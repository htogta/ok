#define OKVM_IMPLEMENTATION
#include "../okvm.h"
// okvm.h includes stdio already, so we don't need to include it again

// device functions must have this signature
unsigned char serial_output(OkVM* vm, unsigned char op);

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("usage: okmin file.rom\n");
    return 1;
  }

  OkVM vm;
  int init_failed = okvm_init_from_file(&vm, argv[1]); // allocates memory
  if (init_failed) return 1;
  
  // registering an external device 
  int reg_failed = okvm_register_device(&vm, serial_output);
  if (reg_failed) return 1;

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) okvm_tick(&vm);

  okvm_free(&vm); // this frees the memory allocated at init
  return vm.status != OKVM_HALTED;
}

// our device function just prints the byte at a specific RAM address
unsigned char serial_output(OkVM* vm, unsigned char op) {
  // you don't have to use the "op" argument if you don't want to
  char c = vm->ram[0xbabe];
  putchar(c);
  return c;
}
