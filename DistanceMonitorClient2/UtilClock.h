#ifndef UTIL_CLOCK_H
#define UTIL_CLOCK_H

const uint8_t TextSizeBase = 8;

void setupRealTimeClockFromInternet();
struct tm* getDateTimeNow();
void setRTC(time_t timeToSet);
void setLocalTimeFromRTC();

char* lcdPrintTimeIfNeeded();
char* lcdPrintDateIfNeeded();

#endif


