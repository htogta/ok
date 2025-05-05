#include "okvm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// usage: okmin [-v] file.rom
void print_usage();
void print_version();
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

// run a VM rom- returns a system exit code for main to return
int run_file(char* filepath) {
  // file handling boilerplate
  FILE* fileptr = fopen(filepath, "rb");

  if (fileptr == NULL) {
    fprintf(stderr, "ERROR: file not found %s\n", filepath);
    return 1;
  }
  
  fseek(fileptr, 0, SEEK_END);
  size_t file_size = ftell(fileptr);
  rewind(fileptr);


  // make a buffer for the file bytes before loading
  unsigned char* program = malloc(file_size);
  if (program == NULL) {
    fprintf(stderr, "ERROR: failed to allocate memory for program\n");
    fclose(fileptr);
    return 1;
  }

  size_t read_bytes = fread(program, sizeof(unsigned char), file_size, fileptr);
  if (read_bytes < file_size) {
    fprintf(stderr, "ERROR: failed to read file data\n");
    fclose(fileptr);
    free(program);
    return 1;
  }
  
  // if the file size is too large to fit in vm ROM, exit here.
  if (read_bytes > (1 << (OKVM_WORD_SIZE * 8))) {
    fprintf(stderr, "ERROR: '%s' too large to fit in VM ROM\n", filepath);
    fclose(fileptr);
    free(program);
    return 1;
  }

  // now we're done with the file
  fclose(fileptr);

  // TODO optimize okvm_init so that we don't have to read the file into a program 
  // buffer before loading it into ROM, and instead just load the file data 
  // directly into ROM?

  // now to init the vm
  OkVM vm;
  okvm_init(&vm, program, read_bytes);

  // we can free program now if we want, it's been stored in the VM's ROM
  free(program);

  vm.status = OKVM_RUNNING;
  while (vm.status == OKVM_RUNNING) {
    okvm_tick(&vm);
  }

  if (vm.status == OKVM_PANIC) {
    fprintf(stderr, "ERROR: VM exited with panic at PC value %zu\n", vm.pc);
    okvm_free(&vm);
    return 1;
  }

  okvm_free(&vm);

  return 0; // success!
}
