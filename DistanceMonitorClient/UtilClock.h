#ifndef UTIL_CLOCK_H
#define UTIL_CLOCK_H

void setupRealTimeClockFromInternet();
struct tm* getDateTimeNow();
void setRTC(time_t timeToSet);
void setLocalTimeFromRTC();

void ledPrintTimeIfNeeded();
void ledPrintDateIfNeeded();

#endif


