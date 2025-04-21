#include "okstack.h"
#include <assert.h>

OkStack stack_init() {
  return (OkStack){}; // pre-init values to 0s
}

void stack_push(OkStack* s, unsigned char i) {
  s->data[s->sp] = i;
  s->sp++;
}

unsigned char stack_pop(OkStack* s) {
  unsigned char out = s->data[s->sp - 1];
  s->data[s->sp--] = 0;
  return out;
}

// pop 1-4 bytes as a 32-bit int
unsigned int stack_popn(OkStack* s, unsigned char n) {
  assert(n >= 1); assert(n <= 4);
  
  unsigned int out = 0;
  for (int i = 0; i < n; i++) {
    out = (out << 8) | stack_pop(s);
  }
  
  return out;
}

// push 1-4 bytes of a 32-bit int
void stack_pushn(OkStack* s, unsigned char n, unsigned int val) {
  assert(n >= 1); assert(n <= 4);
  
  for (int i = 0; i < n; i++) {
    unsigned char byte = (unsigned char) ((val >> (8 * i)) & 0xFF);
    stack_push(s, byte);
  }
}
