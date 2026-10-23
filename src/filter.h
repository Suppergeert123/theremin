#ifndef FILTER_H
#define FILTER_H

#include <stdbool.h>
#include <stdint.h>

#define FILTER_MIN_SIZE 1
#define FILTER_MAX_SIZE 15

// maakt filter aan met start buffer (size) true als het lukt anders false
bool InitFilter(uint8_t size);

// voegt nieuwe meting toe en returnt het mediaan
uint16_t FilterDistance(uint16_t distance);

// verander de filter buffer
bool SetFilterSize(uint8_t size);

// return de filter buffer
uint8_t GetFilterSize(void);

// berekent de nieuwe filter buffer, altijd in stappen van 2 zodat het oneven blijft
uint8_t NextFilterSize(uint8_t current, int8_t step);

// gebruikt wanneer er geen hand meer gezien wordt, volgende meting vult alles
void ResetFilter(void);

// haal de hele filter leeg
void FreeFilter(void);

#endif 
