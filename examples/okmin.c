#define OK_IMPLEMENTATION
#include "../ok.h"
// okvm.h includes stdio already, so we don't need to include it again

// device functions must have this signature
uint8_t serial_output(uint8_t* ram, uint8_t* rom);

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("usage: okmin file.rom\n");
    return 1;
  }

  OkVM vm;
  int init_failed = okvm_init_from_file(&vm, argv[1]); // allocates memory
  if (init_failed) return 1;
  
  // registering an external device
  // since the port is 55, the opcodes "#37 INT1" will trigger the device
  int reg_failed = okvm_register_device(&vm, serial_output, 55);
  if (reg_failed) return 1;

  vm.status = OK_RUNNING;
  while (vm.status == OK_RUNNING) okvm_tick(&vm);

  okvm_free(&vm); // this frees the memory allocated at init
  return vm.status != OK_HALTED;
}

// our device function just prints the byte at a specific RAM address
uint8_t serial_output(uint8_t* ram, uint8_t* rom) {
  char c = ram[0x37]; // here is that "magic" address
  putchar(c);
  return c;
}
