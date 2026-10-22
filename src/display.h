#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

// start de display in waiting modus
void InitDisplay(void);

// set nieuwe distance en freq
void UpdateDisplay(uint16_t distance_cm, uint16_t frequency_hz);

// wanneer er niks bij de sensor is
void ShowNoHand(uint16_t frequency_hz);

#endif 
