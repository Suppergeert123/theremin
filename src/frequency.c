#include "frequency.h"

bool IsDistanceInRange(uint16_t distance_cm)
{
    return distance_cm >= MIN_DISTANCE_CM && distance_cm <= MAX_DISTANCE_CM;
}

uint16_t CalculateFrequency(uint16_t distance_cm)
{
    if (!IsDistanceInRange(distance_cm))
    {
        return MIN_FREQ_HZ;
    }

    return MAX_FREQ_HZ -
       ((uint32_t)distance_cm * (MAX_FREQ_HZ - MIN_FREQ_HZ)) / MAX_DISTANCE_CM;
}
