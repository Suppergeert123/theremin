#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#include "display.h"
#include "frequency.h"
#include "sensor.h"
#include "filter.h"
#include "buttons.h"
#include "display7seg.h"
#include "volume.h"
#include "tone.h"

#define STANDARD_FILTER_SIZE 5
static bool sound_enabled = false;   // later voor buzzer

static void Initialize(void)
{
    InitDisplay();
    InitializeUltrasonicSensor();
    InitFilter(STANDARD_FILTER_SIZE);
    InitializeButtons();
    Init7SegmentDisplay();
    InitializeVolume();
    InitializeTone();
    UpdateFilterDisplay(GetFilterSize());

    sei();
}

int main(void)
{
    Initialize();

    while (1)
    {
        SetVolume(GetVolume());

        ReadDistance();

        if (hasNewMeasurement())
        {
            uint16_t distance = GetReadDistance();

            if (IsDistanceInRange(distance))
            {
                uint16_t filtered = FilterDistance(distance);
                uint16_t frequency = CalculateFrequency(filtered);

                EnableTone(true);
                AdjustTone(frequency);
                UpdateDisplay(distance, frequency);
            }
            else
            {
                EnableTone(false);
                ResetFilter();
                ShowNoHand(MIN_FREQ_HZ);
            }
        }

        int8_t step = ReadButtons(); // +1 als er vergroot is, -1 als er verkleint is
        if (step != 0)
        {
            SetFilterSize(NextFilterSize(GetFilterSize(), step));
            UpdateFilterDisplay(GetFilterSize());
        }

    }
}
