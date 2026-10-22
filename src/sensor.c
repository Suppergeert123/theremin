#include "sensor.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define TRIG_PIN PB1
#define ECHO_PIN PB0

// timer 1 op prescaler 8. timout is ruim genoeg voor 65cm
#define ECHO_TIMEOUT_TICKS 12000U

// sensor heeft 60ms timeout tussen metingen nodig. 2 overflows van 32ms
#define COOLDOWN_OVERFLOWS 2

// gedeelde variabelen
static volatile uint16_t distance_cm = 0;
static volatile uint8_t measurement_ready = 0;
static volatile uint8_t measurement_timeout = 0;
static volatile uint8_t cooldown_overflows = 0;

void InitializeUltrasonicSensor(void)
{
    DDRB |= (1 << TRIG_PIN);
    DDRB &= ~(1 << ECHO_PIN);
    PORTB &= ~(1 << TRIG_PIN);

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // Timer1 prescaler 8 eerst opgaande edge
    TCCR1B = (1 << ICES1) | (1 << CS11);

    // alleen de cooldown interupt, anderen worden bij ReadDistance aangezet
    TIMSK1 = (1 << TOIE1);
}

void ReadDistance(void)
{
    uint8_t old_sreg = SREG;
    cli();

    // niks doen als sensor niet ready is
    if (measurement_ready || cooldown_overflows ||
        (TIMSK1 & ((1 << ICIE1) | (1 << OCIE1A))))
    {
        SREG = old_sreg;
        return;
    }

    measurement_timeout = 0;
    TCCR1B |= (1 << ICES1);            // eerst rising edge
    TCNT1 = 0;

    TIFR1 = (1 << ICF1) | (1 << OCF1A);

    OCR1A = ECHO_TIMEOUT_TICKS;
    TIMSK1 |= (1 << ICIE1) | (1 << OCIE1A);

    cooldown_overflows = COOLDOWN_OVERFLOWS;

    SREG = old_sreg;

    PORTB |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTB &= ~(1 << TRIG_PIN);
}

bool hasNewMeasurement(void)
{
    return measurement_ready != 0;
}

uint16_t GetReadDistance(void)
{
    uint8_t old_sreg = SREG;
    uint16_t result;
    uint8_t timed_out;

    cli();

    result = distance_cm;
    timed_out = measurement_timeout;
    measurement_ready = 0;

    SREG = old_sreg;

    return timed_out ? 0 : result;
}

// ---------------- ISR's -------------- //

ISR(TIMER1_CAPT_vect)
{
    static uint16_t start_ticks = 0;
    uint16_t captured_ticks = ICR1;

    if (TCCR1B & (1 << ICES1))
    {
        // echo start
        start_ticks = captured_ticks;
        TCCR1B &= ~(1 << ICES1);
        TIFR1 = (1 << ICF1);           // flaggen wissen
    }
    else
    {
        // echo einde
        uint16_t pulse_ticks = captured_ticks - start_ticks;

        distance_cm = pulse_ticks / 116U;

        measurement_timeout = 0;
        measurement_ready = 1;

        TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1A));
        TCCR1B |= (1 << ICES1);
        TIFR1 = (1 << ICF1);
    }
}

ISR(TIMER1_COMPA_vect)
{
    measurement_timeout = 1;
    measurement_ready = 1;

    TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1A));
    TCCR1B |= (1 << ICES1);
}

ISR(TIMER1_OVF_vect)
{
    if (cooldown_overflows)
    {
        cooldown_overflows--;
    }
}
