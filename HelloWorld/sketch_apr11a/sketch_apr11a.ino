#include <M5StickCPlus.h>
#include <WiFi.h>
#include <NTPClient.h>   // https://github.com/arduino-libraries/NTPClient NTPClient by Fabrice Weinberg
#include <TimeLib.h>     // https://playground.arduino.cc/Code/Time/       Time by Michael Margolis

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const uint32_t BackgroundColor = BLACK;
const uint32_t TextColor = GREEN;
const uint8_t  TextSize = 3;
const uint8_t  ScreenRotation90Degrees = 1;              // // 0 (normal orientation), 1 (90 degrees clockwise), 2 (180 degrees), or 3 (90 degrees counterclockwise)
const unsigned long SerialPortBaudRate = 9600;

const char* WifiSsid = "AirPort";
const char* WifiPassword = "ivacivac";

/* After M5StickC is started or reset
  the program in the setUp () function will be run, and this part will only be run once.
  After M5StickCPlus is started or reset, the program in the setup() function will be executed, and this part will only be executed once. */
void setup()
{
  // Initialize the M5StickCPlus object. Initialize the M5StickCPlus object
  M5.begin();

  M5.Lcd.setRotation(ScreenRotation90Degrees);  
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(BackgroundColor);
  M5.Lcd.setTextColor(TextColor, BackgroundColor);

  Serial.begin(SerialPortBaudRate); // set the baud rate to 9600 (or your desired value)

  // LCD display. LCd display
  M5.Lcd.setTextSize(TextSize);
  M5.Lcd.print("Hello\nWorld\nTest\n");

  WiFi.begin(WifiSsid, WifiPassword);

  M5.Lcd.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print(".");
  }

  M5.Lcd.print("\nConnected\n");

  // Set the Datetime based on the NTPClient time
  if( true ) {             
    timeClient.setTimeOffset(-4 * 60 * 60);  // Set the time zone to GMT -5 (Eastern Standard Time)
  } else {
    timeClient.setTimeOffset(-5 * 60 * 60);  // Set the time zone to GMT -5 (Eastern Standard Time)
  }
  timeClient.begin();
  if( timeClient.update() ) {
    M5.Lcd.fillScreen(BackgroundColor);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("Setting Time");
    time_t currentTime = timeClient.getEpochTime();
    struct tm *timeinfo = localtime(&currentTime);
    setTime(currentTime);
    setRTC(currentTime);        
  }

  //time_t currentTime = timeClient.getEpochTime();
  //RTC_TimeTypeDef test = timeToRtcTime(currentTime);
  //M5.Rtc.SetTime(&test);

  delay(1000); // Wait for a second before sending the next message
}

bool _firstTimeThoughLoop = true;

/* After the program in setup() runs, it runs the program in loop()
The loop() function is an infinite loop in which the program runs repeatedly
After the program in the setup() function is executed, the program in the loop() function will be executed
The loop() function is an endless loop, in which the program will continue to run repeatedly */
void loop() 
{
  time_t currentTime = now(); // timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&currentTime);
  char currentLocalTimeStr[16];
  strftime(currentLocalTimeStr, sizeof(currentLocalTimeStr), "%I:%M:%S %p", timeinfo);   // %H for 24 hour time, %I for 12 Hour time

  if( _firstTimeThoughLoop ) {
      _firstTimeThoughLoop = false;
      Serial.println("\nFirst Time");
      time_t nowTime = now();
      struct tm *timeinfo = localtime(&nowTime);
      char currentLocalTimeStr[40];
      strftime(currentLocalTimeStr, sizeof(currentLocalTimeStr), "%m/%d/%Y %I:%M:%S %p", timeinfo);   // %H for 24 hour time, %I for 12 Hour time
      Serial.println("Now time:");
      Serial.println(currentLocalTimeStr);
  }

  M5.Lcd.fillScreen(BackgroundColor);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Current time:");
  M5.Lcd.println(currentLocalTimeStr);

  Serial.println("Hello World Test");
  Serial.println(currentLocalTimeStr);

  delay(10000); // Wait for a second before sending the next message
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
