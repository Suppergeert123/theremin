#include "sensor.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#define TRIG_PIN PB1
#define ECHO_PIN PB0

// timer 1 op prescaler 8. timout is ruim genoeg voor 65cm
#define ECHO_TIMEOUT_TICKS 12000U
#define TRIGGER_TICKS 20U  // 10us trigger pulse 

// sensor heeft 60ms timeout tussen metingen nodig. 2 overflows van 32ms
#define COOLDOWN_OVERFLOWS 2

typedef enum
{
    START_PULSE,
    END_PULSE,
    WAIT
} SensorState;

static SensorState state = START_PULSE;
static uint16_t trigger_start_ticks = 0; 

// gedeelde variabelen
static volatile uint16_t pulse_ticks = 0;
static volatile uint8_t capture_flag = 0;         
static volatile uint8_t measurement_timeout = 0;
static volatile uint8_t cooldown_overflows = 0;

// variabelen die door de WAIT state worden geset 
static uint16_t distance_cm = 0;
static bool measurement_ready = false; 

static void SetTriggerHigh(void)
{
    PORTB |= (1 << TRIG_PIN);
}

static void SetTriggerLow(void)
{
    PORTB &= ~(1 << TRIG_PIN);
}

static uint16_t CalculateDistance(uint16_t ticks)
{
    return ticks / 116U;
}

// begin met luisteren voor de echo, aangeroepen in de END PULSE state
static void StartEchoCapture(void)
{
    uint8_t old_sreg = SREG;
    cli();

    measurement_timeout = 0;
    capture_flag = 0;

    TCCR1B |= (1 << ICES1);
    TCNT1 = 0;
    TIFR1 = (1 << ICF1) | (1 << OCF1A);

    OCR1A = ECHO_TIMEOUT_TICKS;
    TIMSK1 |= (1 << ICIE1) | (1 << OCIE1A);

    cooldown_overflows = COOLDOWN_OVERFLOWS;

    SREG = old_sreg;
}

void InitializeUltrasonicSensor(void)
{
    DDRB |= (1 << TRIG_PIN);
    DDRB &= ~(1 << ECHO_PIN);
    SetTriggerLow();

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // Timer1 prescaler 8 eerst opgaande edge
    TCCR1B = (1 << ICES1) | (1 << CS11);

    // alleen de cooldown interupt, anderen worden bij ReadDistance aangezet
    TIMSK1 = (1 << TOIE1);

    state = START_PULSE;
}

void ReadDistance(void)
{
    switch (state)
    {
    case START_PULSE:
        // alleen door als sensor klaar is en vorige meting is verwerkt
        if (measurement_ready || cooldown_overflows)
        {
            break;
        }
        SetTriggerHigh();                    // entry state
        trigger_start_ticks = TCNT1;         // exit state
        state = END_PULSE;                  
        break;

    case END_PULSE:
        if ((uint16_t)(TCNT1 - trigger_start_ticks) >= TRIGGER_TICKS)
        {
            SetTriggerLow();                 // exit state
            StartEchoCapture();
            state = WAIT;
        }
        break;

    case WAIT:
        if (capture_flag)                    
        {
            capture_flag = 0;

            // do state
            distance_cm = measurement_timeout ? 0 : CalculateDistance(pulse_ticks);
            measurement_ready = true;

            state = START_PULSE;
        }
        break;
    }
}

bool hasNewMeasurement(void)
{
    return measurement_ready != 0;
}

uint16_t GetReadDistance(void)
{
    measurement_ready = false;
    return distance_cm;
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
        pulse_ticks = captured_ticks - start_ticks;

        measurement_timeout = 0;
        capture_flag = 1;

        TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1A));
        TCCR1B |= (1 << ICES1);
        TIFR1 = (1 << ICF1);
    }
}

ISR(TIMER1_COMPA_vect)
{
    measurement_timeout = 1;
    capture_flag = 1;

    TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1A));
    TCCR1B |= (1 << ICES1);
}

ISR(TIMER1_OVF_vect)
{
    if (cooldown_overflows)
    {
        cooldown_overflows--;
    }

    ButtonsTick();
}
