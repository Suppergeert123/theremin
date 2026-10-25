#ifndef VOLUME_H
#define VOLUME_H

#include <stdint.h>

// start ADC en timer 2 Fast PWM
void InitializeVolume(void);

// potmeter waarde tussen 0 en 255
uint8_t GetVolume(void);

// set de PWM duty cycle 
void SetVolume(uint8_t volume);

#endif 