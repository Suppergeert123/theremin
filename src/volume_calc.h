#ifndef VOLUME_CALC_H
#define VOLUME_CALC_H

#include <stdint.h>

// 0 -> 0%  255 -> 100%
uint8_t AdcToPercent(uint8_t adc_value);

// weer terug naar waarde 0..255
uint8_t PercentToDuty(uint8_t percent);

#endif 
