#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>

void test_byte_set();
void test_byte_fetch();

uint8_t ok_mem_read(size_t address) {
  return 0;
}

void ok_mem_write(size_t address, uint8_t val) {
  return;
}

uint8_t ok_fetch(size_t address) {
  return 0;
}

int main() {
  printf("...test-helpers: ");

  test_byte_fetch();
  test_byte_set();

  return 0;
}

void test_byte_fetch() {
  printf("\n    ok_get_bytes ");
  uint8_t buffer[32];
  
  buffer[3] = 0x01;
  buffer[4] = 0x45; // storing short value "325" at address 3

  uint16_t result = (uint16_t) ok_get_bytes(buffer, 3, 2);
  assert(result == 0x145);
  printf("PASSED");
}

void test_byte_set() {
  printf("\n    ok_set_bytes ");
  uint8_t buffer[32];

  ok_set_bytes(buffer, 3, 2, 0x145);

  assert(buffer[3] == 0x01);
  assert(buffer[4] == 0x45);

  printf("PASSED\n");
}
