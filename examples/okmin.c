#define OKVM_IMPLEMENTATION
#include "../okvm.h"
// okvm.h includes stdio already, so we don't need to include it again

// device functions must have this signature
unsigned char serial_output(OkVM* vm);

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("usage: okmin file.rom\n");
    return 1;
  }

  OkVM vm;

  
  // NOTE: allocates memory for RAM and ROM
  int init_failed = okvm_init_from_file(&vm, argv[1]);
  if (init_failed) return 1;
  
  // the binary is the device flag, least significant bit must be 1
  int reg_failed = okvm_register_device(&vm, 0b10000000, serial_output);
  if (reg_failed) return 1;

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) okvm_tick(&vm);

  okvm_free(&vm); // this frees the vm RAM and ROM

  return vm.status != OKVM_HALTED;
}

// our device function just prints the byte at a specific address
unsigned char serial_output(OkVM* vm) {
  char c = vm->ram[0xbabe];
  putchar(c);
  return c;
}
