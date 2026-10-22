#include "display.h"

#include <stdbool.h>
#include <stdio.h>

#include "frequency.h"
#include "hd44780pcf8574.h"

#define LCD_COLUMNS 16

typedef enum //alleen als de state veranderd hoeft er een update te komen
{
    SCREEN_WAITING,
    SCREEN_VALUES,
    SCREEN_NO_HAND
} ScreenState;

static ScreenState state = SCREEN_WAITING;

static bool distance_shown = false;
static bool frequency_shown = false;
static uint16_t last_distance = 0;
static uint16_t last_frequency = 0;

static void DrawLine(uint8_t row, const char *text)
{
    char line[LCD_COLUMNS + 1];

    snprintf(line, sizeof(line), "%-16s", text);

    HD44780_PCF8574_PositionXY(PCF8574_ADDRESS, 0, row);
    HD44780_PCF8574_DrawString(PCF8574_ADDRESS, line);
}

// lijn 2 is hetzelfde tot de waarde veranderd dus hoeft niet telkens aangeroepen te worden
static void ShowFrequency(uint16_t frequency_hz)
{
    char text[LCD_COLUMNS + 1];

    if (!frequency_shown || frequency_hz != last_frequency)
    {
        snprintf(text, sizeof(text), "Freq: %uHz", frequency_hz);
        DrawLine(1, text);

        last_frequency = frequency_hz;
        frequency_shown = true;
    }
}

void InitDisplay(void)
{
    HD44780_PCF8574_Init(PCF8574_ADDRESS);
    HD44780_PCF8574_DisplayOn(PCF8574_ADDRESS);
    HD44780_PCF8574_DisplayClear(PCF8574_ADDRESS);

    DrawLine(0, "Waiting...");
    state = SCREEN_WAITING;
    distance_shown = false;

    frequency_shown = false;
    ShowFrequency(MIN_FREQ_HZ);
}

void UpdateDisplay(uint16_t distance_cm, uint16_t frequency_hz)
{
    char text[LCD_COLUMNS + 1];

    if (state != SCREEN_VALUES)
    {
        state = SCREEN_VALUES;
        distance_shown = false;     // state veranderd dus opnieuw lijn drawen   
    }

    if (!distance_shown || distance_cm != last_distance)
    {
        snprintf(text, sizeof(text), "Distance: %ucm", distance_cm);
        DrawLine(0, text);

        last_distance = distance_cm;
        distance_shown = true;
    }

    ShowFrequency(frequency_hz);
}

void ShowNoHand(uint16_t frequency_hz)
{
    if (state != SCREEN_NO_HAND)
    {
        state = SCREEN_NO_HAND;
        DrawLine(0, "No hand detected");
        distance_shown = false;
    }

    ShowFrequency(frequency_hz);
}
