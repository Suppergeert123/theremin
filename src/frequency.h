#ifndef FREQUENCY_H
#define FREQUENCY_H

#include <stdbool.h>
#include <stdint.h>

#define MIN_DISTANCE_CM 0
#define MAX_DISTANCE_CM 65

#define MIN_FREQ_HZ 230
#define MAX_FREQ_HZ 1400

// true als het binnen 2 tot 65cm is
bool IsDistanceInRange(uint16_t distance_cm);

// geeft freq in hz terug
uint16_t CalculateFrequency(uint16_t distance_cm);

#endif 
