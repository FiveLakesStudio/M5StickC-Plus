#include <M5StickCPlus.h>
#include "UtilClock.h"

WiFiUDP ntpUDP;
NTPClient timeNtpClient(ntpUDP, "pool.ntp.org");

// US Eastern Time Zone (New York, Washington D.C., Miami, etc.)
TimeChangeRule EDT = {"EDT", Second, Sun, Mar, 2, -240}; // UTC - 4 hours
TimeChangeRule EST = {"EST", First, Sun, Nov, 2, -300};  // UTC - 5 hours
Timezone timezone(EDT, EST);

void setupRealTimeClockFromInternet()
{
  M5.Lcd.fillScreen(BackgroundColor);
  M5.Lcd.setCursor(0, 0);

  timeNtpClient.begin();

  if( timeNtpClient.update() ) 
  {
    M5.Lcd.println("Setting Time from NTP");
    time_t currentTime = timeNtpClient.getEpochTime();
    time_t localTime = timezone.toLocal(currentTime);

    struct tm *timeinfo = localtime(&localTime);

    setTime(localTime);
    setRTC(localTime);       
  } 
  else
  {
    M5.Lcd.println("Setting Time From RTC");
    setLocalTimeFromRTC();
  }

  delay(1000); // Wait for a second So user can see prompt
}

struct tm* getDateTimeNow()
{
  time_t currentTime = now(); // timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&currentTime);
  return timeinfo;  
}

void setRTC(time_t timeToSet) {
  // Convert the time_t value to a tm struct
  struct tm *timeInfo = localtime(&timeToSet);

  // Set the RTC date
  RTC_DateTypeDef rtcDate;
  rtcDate.Year = timeInfo->tm_year + 1900;
  rtcDate.Month = timeInfo->tm_mon + 1;
  rtcDate.Date = timeInfo->tm_mday;
  rtcDate.WeekDay = timeInfo->tm_wday;
  M5.Rtc.SetData(&rtcDate);

  // Set the RTC time
  RTC_TimeTypeDef rtcTime;
  rtcTime.Hours = timeInfo->tm_hour;
  rtcTime.Minutes = timeInfo->tm_min;
  rtcTime.Seconds = timeInfo->tm_sec;
  M5.Rtc.SetTime(&rtcTime);
}

void setLocalTimeFromRTC() {
  RTC_TimeTypeDef rtcTime;
  RTC_DateTypeDef rtcDate;

  M5.Rtc.GetTime(&rtcTime);
  M5.Rtc.GetData(&rtcDate);

  struct tm timeInfo;
  timeInfo.tm_year = rtcDate.Year - 1900;
  timeInfo.tm_mon = rtcDate.Month - 1;
  timeInfo.tm_mday = rtcDate.Date;
  timeInfo.tm_hour = rtcTime.Hours;
  timeInfo.tm_min = rtcTime.Minutes;
  timeInfo.tm_sec = rtcTime.Seconds;
  timeInfo.tm_isdst = -1; // Let the system determine DST (Daylight Saving Time)

  time_t epochTime = mktime(&timeInfo);
  setTime(epochTime);
}

// Add global variables to store the last printed date and time
char lastTimeStr[16] = "";
char lastDateStr[16] = "";

void ledPrintTimeIfNeeded() {
  struct tm* dateTimeNow = getDateTimeNow();

  char currentTimeStr[16];
  strftime(currentTimeStr, sizeof(currentTimeStr), "%I:%M:%S %p", dateTimeNow); // %H for 24 hour time, %I for 12 Hour time

  if (strcmp(lastTimeStr, currentTimeStr) == 0)
    return;

  // The time has changed; update the display
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print(currentTimeStr);
  clearToEndOfLine();

  // Update the last printed time
  strncpy(lastTimeStr, currentTimeStr, sizeof(lastTimeStr));
}

void ledPrintDateIfNeeded() {
  struct tm* dateTimeNow = getDateTimeNow();

  char currentDateStr[16];
  strftime(currentDateStr, sizeof(currentDateStr), "%m:%d:%Y", dateTimeNow);

  if (strcmp(lastDateStr, currentDateStr) == 0)
    return;

  // The date has changed; update the display
  M5.Lcd.setCursor(0, TextSize * 8);
  M5.Lcd.print(currentDateStr);
  clearToEndOfLine();

  // Update the last printed date
  strncpy(lastDateStr, currentDateStr, sizeof(lastDateStr));
}

