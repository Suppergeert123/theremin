#include "buttons.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#define BUTTON_MIN_PIN  PD4
#define BUTTON_PLUS_PIN PD5
#define BUTTON_MASK     ((1 << BUTTON_MIN_PIN) | (1 << BUTTON_PLUS_PIN))

// gebruikt in sensor om debounce te voorkomen
#define LOCKOUT_TICKS 2

static volatile uint8_t last_state = BUTTON_MASK;   
static volatile uint8_t lockout = 0;
static volatile uint8_t plus_pressed = 0;
static volatile uint8_t min_pressed = 0;

volatile uint16_t buttonInterruptCount = 0;

void InitializeButtons(void)
{
    DDRD &= ~BUTTON_MASK;            // input
    PORTD |= BUTTON_MASK;            // interne pull up

    last_state = PIND & BUTTON_MASK;

    PCMSK2 |= (1 << PCINT20) | (1 << PCINT21);   // pd4 en pd5
    PCICR |= (1 << PCIE2);
}

int8_t ReadButtons(void)
{
    uint8_t old_sreg = SREG;
    int8_t step = 0;

    cli();
    if (plus_pressed)
    {
        step = 1;
    }
    else if (min_pressed)
    {
        step = -1;
    }
    plus_pressed = 0;
    min_pressed = 0;
    SREG = old_sreg;

    return step;
}

void ButtonsTick(void)
{
    if (lockout)
    {
        lockout--;
    }
}

ISR(PCINT2_vect)
{
    buttonInterruptCount++;

    uint8_t now = PIND & BUTTON_MASK;
    uint8_t pressed = last_state & (uint8_t)~now;     // was high nu low
    uint8_t released = (uint8_t)~last_state & now;    // was low nu high
    last_state = now;

    if (released)
    {
        lockout = LOCKOUT_TICKS;                       //debouncen negeren
    }

    if (lockout)
    {
        return;
    }

    if (pressed & (1 << BUTTON_PLUS_PIN))
    {
        plus_pressed = 1;
        lockout = LOCKOUT_TICKS;
    }
    else if (pressed & (1 << BUTTON_MIN_PIN))
    {
        min_pressed = 1;
        lockout = LOCKOUT_TICKS;
    }
}
