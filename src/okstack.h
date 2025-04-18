#ifndef OKSTACK_H
#define OKSTACK_H

typedef struct {
  unsigned char sp;
  unsigned char data[256];
} OkStack; 

void stack_push(OkStack* s, unsigned char i);
unsigned char stack_pop(OkStack* s);
OkStack stack_init();

#endif
