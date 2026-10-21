
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

#include "hd44780pcf8574.h"

#define LCD_COLUMNS 16

static bool distance_shown = false; //lcd start op met niks
static bool frequency_shown = false; // lcd start op met niks

static uint16_t last_distance = 0; //voor lcd
static uint16_t last_frequency = 0; // voor lcd

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


int main(void)
{
    uint16_t distance = 5;
    int8_t step = 1;
    uint16_t frequency;

    InitLcdDisplay();

    while (1)
    {
        frequency = 1400 - 18 * distance;

        UpdateDisplay(distance, frequency);

        distance = distance + step;

        if (distance >= 65)
        {
            step = -1;
        }

        if (distance <= 5)
        {
            step = 1;
        }

        // Alleen voor deze dummytest
        _delay_ms(200);
    }
}