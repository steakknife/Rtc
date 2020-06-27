
#include <Arduino.h>
#include "RtcUtility.h"

uint8_t BcdToUint8(uint8_t val)
{
    return val - 6 * (val >> 4);
}

uint8_t Uint8ToBcd(uint8_t val)
{
    return val + 6 * (val / 10);
}

uint8_t BcdToBin24Hour(uint8_t bcdHour)
{
    return (bcdHour & 0x40)
                                        // add 12-hours if PM, 0 if AM
        ? (BcdToUint8(bcdHour & 0x1f) + (((bcdHour & 0x20) >> 3) * 3))
        : BcdToUint8(bcdHour);
}
