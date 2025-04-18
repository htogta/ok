#include "okstack.h"

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
