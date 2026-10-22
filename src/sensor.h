#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>
#include <stdint.h>

// setup de pins en timer1
void InitializeUltrasonicSensor(void);

// laat de sensor een meting starten
void ReadDistance(void);

// true als er een meting of timeout ISR is geweest
bool hasNewMeasurement(void);

// geeft de afstand in cm terug / of 0 als timout is
uint16_t GetReadDistance(void);

#endif 
