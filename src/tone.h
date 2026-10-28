
#ifndef TONE_H
#define TONE_H

#include <stdbool.h>
#include <stdint.h>

// voor sei() aanroepen in main
void InitializeTone(void);

// pas de toon aan door timer 0 aan te passen
void AdjustTone(uint16_t frequency_hz);

// zet de toon aan of uit
void EnableTone(bool enabled);

#endif
