#include "volume_calc.h"

uint8_t AdcToPercent(uint8_t adc_value)
{
    return (uint8_t)(((uint16_t)adc_value * 100U + 127U) / 255U);
}

uint8_t PercentToDuty(uint8_t percent)
{
    if (percent > 100)
    {
        percent = 100;
    }
    return (uint8_t)(((uint16_t)percent * 255U + 50U) / 100U);
}
