#define OK_IMPLEMENTATION
#include "../ok.h"
#include <stdio.h>
#include <assert.h>

void test_byte_set();
void test_byte_fetch();

int main() {
  printf("Testing byte buffer helper functions...\n");

  test_byte_set();
  test_byte_fetch();

  printf("Done.\n");

  return 0;
}

void test_byte_fetch() {
  printf("...test_byte_fetch ");
  uint8_t buffer[32];
  
  buffer[3] = 0x01;
  buffer[4] = 0x45; // storing short value "325" at address 3

  uint16_t result = (uint16_t) ok_fetch_bytes(2, 3, buffer);
  assert(result == 0x145);
  printf("PASSED\n");
}

void test_byte_set() {
  printf("...test_byte_set ");
  uint8_t buffer[32];

  ok_set_bytes(0x145, 2, 3, buffer);

  assert(buffer[3] == 0x01);
  assert(buffer[4] == 0x45);

  printf("PASSED\n");
}
