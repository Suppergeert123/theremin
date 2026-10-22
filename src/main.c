#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#include "display.h"
#include "frequency.h"
#include "sensor.h"

static bool sound_enabled = false;   // later voor buzzer

static void Initialize(void)
{
    InitDisplay();
    InitializeUltrasonicSensor();

    // TODO:
    // - ADC free-running voor de potmeter initialiseren
    // - ADC-interrupt inschakelen
    // - buzzer-timers initialiseren
    // - pin-change-interrupts voor de knoppen instellen
    // - TWI en 7-segmentdisplay initialiseren

    sei();
}

int main(void)
{
    Initialize();

    while (1)
    {
        ReadDistance();

        if (hasNewMeasurement())
        {
            uint16_t distance = GetReadDistance();

            if (IsDistanceInRange(distance))
            {
                uint16_t frequency = CalculateFrequency(distance);

                sound_enabled = true;
                UpdateDisplay(distance, frequency);
            }
            else
            {
                sound_enabled = false;
                ShowNoHand(MIN_FREQ_HZ);
            }
        }
    }
}
