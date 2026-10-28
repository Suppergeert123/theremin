
#include "tone.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#define TONE_PRESCALER 256UL
#define TIMER0_MAX_OCR 255U
#define PWM_PIN PD3

static volatile bool tone_enabled = false;
static volatile bool pwm_connected = false;

void InitializeTone(void)
{
    // Timer0: CTC-modus, geen hardware-output van Timer0.
    TCCR0A = (1 << WGM01);
    TCCR0B = 0;

    // Beginwaarde voor de compare match.
    OCR0A = 135;

    // Interrupt bij Output Compare Match A.
    TIMSK0 = (1 << OCIE0A);

    // Timer0 starten met prescaler 256.
    TCCR0B = (1 << CS02);

    tone_enabled = false;
    pwm_connected = false;

    // Zorg dat de PWM-uitgang uit staat.
    TCCR2A &= ~(1 << COM2B1);
    PORTD &= ~(1 << PWM_PIN);
}

void AdjustTone(uint16_t frequency_hz)
{
    if (frequency_hz == 0)
    {
        EnableTone(false);
        return;
    }

    // Timer0-interrupt moet twee keer per toonperiode optreden.
    uint32_t ocr =
        (F_CPU / (2UL * TONE_PRESCALER * frequency_hz)) - 1UL;

    if (ocr > TIMER0_MAX_OCR)
    {
        ocr = TIMER0_MAX_OCR;
    }

    // Timer0 is 8-bit. Pas OCR0A atomair aan.
    uint8_t saved_sreg = SREG;
    cli();
    OCR0A = (uint8_t)ocr;
    SREG = saved_sreg;
}

void EnableTone(bool enabled)
{
    uint8_t saved_sreg = SREG;
    cli();

    tone_enabled = enabled;

    if (!enabled)
    {
        pwm_connected = false;
        TCCR2A &= ~(1 << COM2B1);
        PORTD &= ~(1 << PWM_PIN);
    }

    SREG = saved_sreg;
}

ISR(TIMER0_COMPA_vect)
{
    if (!tone_enabled)
    {
        pwm_connected = false;
        TCCR2A &= ~(1 << COM2B1);
        PORTD &= ~(1 << PWM_PIN);
        return;
    }

    if (pwm_connected)
    {
        // PWM-uitgang loskoppelen en de pin laag maken.
        TCCR2A &= ~(1 << COM2B1);
        PORTD &= ~(1 << PWM_PIN);
        pwm_connected = false;
    }
    else
    {
        // PWM-uitgang van Timer2 weer inschakelen.
        TCCR2A |= (1 << COM2B1);
        pwm_connected = true;
    }
}
