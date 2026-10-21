
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "hd44780pcf8574.h"

#define LCD_COLUMNS 16
#define TRIG_PIN PB1
#define ECHO_PIN PB0

//lcd
static bool distance_shown = false; //lcd start op met niks
static bool frequency_shown = false; // lcd start op met niks
static uint16_t last_distance = 0; //voor lcd
static uint16_t last_frequency = 0; // voor lcd

//afstandsmeter
volatile uint16_t capture_start = 0;
volatile uint16_t distance_cm = 0;
volatile uint16_t freq = 0;
volatile uint8_t measurement_ready = 0;
volatile uint8_t measurement_timeout = 0;

// Schrijft een tekst op een LCD regel van 16 tekens
static void DrawLine(uint8_t row, const char *text)
{
    char line[LCD_COLUMNS + 1];

    snprintf(line, sizeof(line), "%-16s", text); // zet getallen -> tekst

    HD44780_PCF8574_PositionXY(PCF8574_ADDRESS, 0, row);
    HD44780_PCF8574_DrawString(PCF8574_ADDRESS, line);
}


// Initialiseert het LCD
static void InitLcdDisplay(void)
{
    HD44780_PCF8574_Init(PCF8574_ADDRESS);
    HD44780_PCF8574_DisplayOn(PCF8574_ADDRESS);
    HD44780_PCF8574_DisplayClear(PCF8574_ADDRESS);
}

static void InitUntrasonicSensor(void)
{
    DDRB |= (1 << TRIG_PIN);
    DDRB &= ~(1 << ECHO_PIN);
    PORTB &= ~(1 << TRIG_PIN);

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // Input Capture: eerst stijgende flank
    TCCR1B = (1 << ICES1) | (1 << CS11);

    // Input Capture en Compare A interrupts
    TIMSK1 = (1 << ICIE1) | (1 << OCIE1A);

    sei();
}


// Werkt het LCD bij als de afstand of frequentie verandert
static void UpdateDisplay(uint16_t distance_cm, uint16_t frequency_hz)
{
    char text[LCD_COLUMNS + 1];

    if (!distance_shown || distance_cm != last_distance)
    {
        snprintf(text, sizeof(text), "Distance: %ucm", distance_cm);
        DrawLine(0, text);

        last_distance = distance_cm;
        distance_shown = true;
    }

    if (!frequency_shown || frequency_hz != last_frequency)
    {
        snprintf(text, sizeof(text), "Freq: %uHz", frequency_hz);
        DrawLine(1, text);

        last_frequency = frequency_hz;
        frequency_shown = true;
    }
}

static void TriggerSensor(void)
{
    PORTB |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTB &= ~(1 << TRIG_PIN);
}

ISR(TIMER1_CAPT_vect)
{
    static uint16_t start_ticks = 0;
    uint16_t captured_ticks = ICR1;

    if (TCCR1B & (1 << ICES1))
    {
        start_ticks = captured_ticks; 

        TCCR1B &= ~(1 << ICES1); //volgende keer naar dalende flank kijken
    }
    else
    {
        uint16_t pulse_ticks = captured_ticks - start_ticks;

        // 1 tick = 0,5 us. afstand in cm = tijd in us / 58
        distance_cm = pulse_ticks / 116;

        measurement_ready = 1;

        TCCR1B |= (1 << ICES1); //weer terug naar stijgende flank
    }
}

int main(void)
{
    InitLcdDisplay();
    InitUntrasonicSensor();

    UpdateDisplay(distance_cm, freq);

    while (1)
    {
        measurement_ready = 0; //oude meting weer wissen
        TriggerSensor();
        _delay_ms(30);

        if (measurement_ready)
        {
            freq = 1400 -18 * distance_cm;
            UpdateDisplay(distance_cm, freq);
        }
        else
        {
            UpdateDisplay(-1, freq);
        }

        // HC-SR04 niet te snel opnieuw triggeren
        _delay_ms(30);
    }
}

