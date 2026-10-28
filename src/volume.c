#include "volume.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include "volume_calc.h"

static volatile uint8_t latest_volume_adc = 0;

void InitializeVolume(void)
{
    //ADC
    ADMUX = (1 << REFS0) | (1 << ADLAR);

    // ADCSRB op free running modus
    ADCSRB = 0;

    DIDR0 |= (1 << ADC0D);

    // ADCSRA: ADC on (ADEN), start the first conversion (ADSC), automatic restart (ADATE),
    // interrupt when a conversion is done (ADIE), prescaler 128 (ADPS2..0 = 1): 125 kHz ADC clock.
    // alles terug te vinden in TO hfd 5
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // timer 2 op fast PWM 
    DDRD |= (1 << PD3);                                      // PD3 output
    OCR2B = 0;                                               // start silent
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);    // fast PWM
    TCCR2B = (1 << CS20);                                    // geen prescaler
}

uint8_t GetVolume(void)
{
    return AdcToPercent(latest_volume_adc);
}

void SetVolume(uint8_t percent)
{
    OCR2B = PercentToDuty(percent);
}

ISR(ADC_vect)
{
    latest_volume_adc = ADCH;        // ADLAR = 1: the 8 most significant bits
}
