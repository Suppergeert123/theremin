#include <stdint.h>
#include "display7seg.h"
#include "twi.h"

#define PCF8574_ADDRESS   0x21
#define PCF8574AN_ADDRESS 0x39

// om alle getallen te kunnen laten zien
static const uint8_t hexSegments[16] =
{
    0x3F, /* 0 */
    0x06, /* 1 */
    0x5B, /* 2 */
    0x4F, /* 3 */
    0x66, /* 4 */
    0x6D, /* 5 */
    0x7D, /* 6 */
    0x07, /* 7 */
    0x7F, /* 8 */
    0x6F, /* 9 */
    0x77, /* A */
    0x7C, /* b */
    0x39, /* C */
    0x5E, /* d */
    0x79, /* E */
    0x71  /* F */
};

static void WritePort(uint8_t address, uint8_t value)
{
    TWI_MT_Start();
    TWI_Transmit_SLAW((char)address);
    TWI_Transmit_Byte((char)value);
    TWI_Stop();
}

void Init7SegmentDisplay(void)
{
    ShowFilterSize(0);
}

void ShowFilterSize(uint8_t size)
{
    if (size > 15)
    {
        return;
    }

    uint8_t pattern = (uint8_t)~hexSegments[size];

    WritePort(0x21, pattern);
    WritePort(0x39, pattern);
}
