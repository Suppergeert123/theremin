#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

extern volatile uint16_t buttonInterruptCount;

// setup de pins en interne pull ups
void InitializeButtons(void);

// returnt 1 wanneer wanneer vergroot button is gedrukt en -1 wanneer de verklein button is gedrukt
int8_t ReadButtons(void);

// gebruikt om debounce tegen te gaan. Wacht 2 ticks 
void ButtonsTick(void);

#endif 
