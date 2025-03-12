#ifndef _RTCTIME_H
#define _RTCTIME_H

#ifdef HAS_RTC

#include <Wire.h> // must be included here so that Arduino library object file references work

#include "globals.h"
#include "timekeeper.h"



uint8_t rtc_init(void);
uint8_t set_rtctime(time_t t);
void sync_rtctime(void);
time_t get_rtctime(uint16_t *msec);
float get_rtctemp(void);

#endif

#endif // _RTCTIME_H
