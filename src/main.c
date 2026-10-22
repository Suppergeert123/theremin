#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "hd44780pcf8574.h"

#define LCD_COLUMNS 16

#define TRIG_PIN PB1
#define ECHO_PIN PB0

#define MIN_DISTANCE_CM 2
#define MAX_DISTANCE_CM 65

#define MIN_FREQ_HZ 230
#define MAX_FREQ_HZ 1400

#define ECHO_TIMEOUT_TICKS 12000U   // 6 ms, genoeg voor ruim 65 cm

static volatile uint8_t timeout_count = 0;
static volatile uint8_t capture_count = 0;


// gedeelde waardes
static volatile uint16_t distance_cm = 0;
static volatile uint8_t measurement_ready = 0;
static volatile uint8_t measurement_timeout = 0;
static volatile uint8_t cooldown_overflows = 0;

static uint16_t frequency_hz = MIN_FREQ_HZ;
static bool sound_enabled = false;

static bool distance_shown = false;
static bool frequency_shown = false;

static uint16_t last_distance = 0;
static uint16_t last_frequency = 0;

// --------- LCD functies ----------- //

static void DrawLine(uint8_t row, const char *text)
{
    char line[LCD_COLUMNS + 1];

    snprintf(line, sizeof(line), "%-16s", text);

    HD44780_PCF8574_PositionXY(PCF8574_ADDRESS, 0, row);
    HD44780_PCF8574_DrawString(PCF8574_ADDRESS, line);
}


static void InitLcdDisplay(void)
{
    HD44780_PCF8574_Init(PCF8574_ADDRESS);
    HD44780_PCF8574_DisplayOn(PCF8574_ADDRESS);
    HD44780_PCF8574_DisplayClear(PCF8574_ADDRESS);
}


static void UpdateDisplay(uint16_t distance, uint16_t frequency)
{
    char text[LCD_COLUMNS + 1];

    if (!distance_shown || distance != last_distance)
    {
        snprintf(text, sizeof(text), "Distance: %ucm", distance);
        DrawLine(0, text);

        last_distance = distance;
        distance_shown = true;
    }

    if (!frequency_shown || frequency != last_frequency)
    {
        snprintf(text, sizeof(text), "Freq: %uHz", frequency);
        DrawLine(1, text);

        last_frequency = frequency;
        frequency_shown = true;
    }
}


// ---------- sensor functies ----- //

static void InitializeUltrasonicSensor(void)
{
    DDRB |= (1 << TRIG_PIN); 
    DDRB &= ~(1 << ECHO_PIN);
    PORTB &= ~(1 << TRIG_PIN);
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // Timer1 prescaler 8, eerste flank stijgend
    TCCR1B = (1 << ICES1) | (1 << CS11);
    TIMSK1 = (1 << TOIE1);   // alleen de overflow-interrupt (voor de wachttijd); capture en compare worden in ReadDistance aangezet
}


// meting starten
static void ReadDistance(void)
{
    uint8_t old_sreg = SREG;
    cli();

    // niet starten als een meting nog bezig is, of als de sensor nog rust nodig heeft
    if (measurement_ready || cooldown_overflows ||
        (TIMSK1 & ((1 << ICIE1) | (1 << OCIE1A))))
    {
        SREG = old_sreg;
        return;
    }

    measurement_timeout = 0;
    TCCR1B |= (1 << ICES1);            // eerst naar stijgende flank kijken
    TCNT1 = 0;

    TIFR1 = (1 << ICF1) | (1 << OCF1A);

    OCR1A = ECHO_TIMEOUT_TICKS;
    TIMSK1 |= (1 << ICIE1) | (1 << OCIE1A);

    cooldown_overflows = 2;            // ~65 ms voor de volgende trigger

    SREG = old_sreg;

    PORTB |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTB &= ~(1 << TRIG_PIN);
}


static bool hasNewMeasurement(void)
{
    return measurement_ready != 0;
}


// Retourneert 0 wanneer de meting is verlopen.
static uint16_t GetReadDistance(void)
{
    uint8_t old_sreg = SREG;
    uint16_t result;
    uint8_t timed_out;

    cli(); //tijdelijk interupts pauzeren

    result = distance_cm;
    timed_out = measurement_timeout;
    measurement_ready = 0;

    SREG = old_sreg;

    if (timed_out)
    {
        return 0;
    }

    return result;
}


// ---- freq functies

static uint16_t CalculateFrequency(uint16_t distance)
{
    if (distance < MIN_DISTANCE_CM ||
        distance > MAX_DISTANCE_CM)
    {
        sound_enabled = false;
        return MIN_FREQ_HZ;
    }

    sound_enabled = true;

    return MAX_FREQ_HZ -
       ((uint32_t)distance * (MAX_FREQ_HZ - MIN_FREQ_HZ)) / MAX_DISTANCE_CM;
}


// -------- INIT --------- //
static void Initialize(void)
{
    InitLcdDisplay();
    InitializeUltrasonicSensor();

    // TODO:
    // - ADC free-running voor de potmeter initialiseren
    // - ADC-interrupt inschakelen
    // - buzzer-timers initialiseren
    // - pin-change-interrupts voor de knoppen instellen
    // - TWI en 7-segmentdisplay initialiseren

    DrawLine(0, "Waiting...");
    DrawLine(1, "Freq: 230Hz");

    sei();
}


// ---------------- ISR's ---------- //

ISR(TIMER1_CAPT_vect)
{
    static uint16_t start_ticks = 0;
    uint16_t captured_ticks = ICR1;

    if (TCCR1B & (1 << ICES1))
    {
        // Stijgende flank: echo begint.
        start_ticks = captured_ticks;
        TCCR1B &= ~(1 << ICES1);
        TIFR1 = (1 << ICF1);
    }
    else
    {
        capture_count++;
        // Dalende flank: echo eindigt.
        uint16_t pulse_ticks = captured_ticks - start_ticks;

        // Timer1-tick = 0,5 us.
        distance_cm = pulse_ticks / 116U;

        measurement_timeout = 0;
        measurement_ready = 1;

        TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1A)); //interupts weer uit 
        TCCR1B |= (1 << ICES1);
        TIFR1 = (1 << ICF1);
    }
}


ISR(TIMER1_COMPA_vect)
{
    timeout_count++;

    measurement_timeout = 1;
    measurement_ready = 1;

    TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1A)); //interupts weer uit
    TCCR1B |= (1 << ICES1);
}

ISR(TIMER1_OVF_vect)
{
    if (cooldown_overflows)
    {
        cooldown_overflows--;
    }
}

// -------- MAIN ---------- //

int main(void)
{
    Initialize();
    while (1)
    {
        ReadDistance();
        if (hasNewMeasurement())
        {
            uint16_t measured_distance = GetReadDistance();

            if (measured_distance >= MIN_DISTANCE_CM && measured_distance <= MAX_DISTANCE_CM)
            {
                frequency_hz = CalculateFrequency(measured_distance);
                UpdateDisplay(measured_distance, frequency_hz);
            }
            else
            {
                sound_enabled = false;
                frequency_hz = MIN_FREQ_HZ;

                DrawLine(0, "No hand detected");
                DrawLine(1, "Freq: 230Hz");

                distance_shown = false;
                frequency_shown = false;
            }
        }
    }
}