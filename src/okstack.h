#ifndef OKSTACK_H
#define OKSTACK_H

typedef struct {
  unsigned char sp;
  unsigned char data[256];
} OkStack; 

void stack_push(OkStack* s, unsigned char i);
unsigned char stack_pop(OkStack* s);
void stack_pushn(OkStack* s, unsigned char n, unsigned int val);
unsigned int stack_popn(OkStack* s, unsigned char n);

OkStack stack_init();

#endif
