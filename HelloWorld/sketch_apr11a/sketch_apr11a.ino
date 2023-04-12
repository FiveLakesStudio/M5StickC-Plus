#include <string.h> 
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <NTPClient.h>   // https://github.com/arduino-libraries/NTPClient NTPClient by Fabrice Weinberg
#include <TimeLib.h>     // https://playground.arduino.cc/Code/Time/       Time by Michael Margolis
#include <Timezone.h>    // https://github.com/JChristensen/Timezone       Jack Christensen

WiFiUDP ntpUDP;
NTPClient timeNtpClient(ntpUDP, "pool.ntp.org");

// US Eastern Time Zone (New York, Washington D.C., Miami, etc.)
TimeChangeRule EDT = {"EDT", Second, Sun, Mar, 2, -240}; // UTC - 4 hours
TimeChangeRule EST = {"EST", First, Sun, Nov, 2, -300};  // UTC - 5 hours
Timezone timezone(EDT, EST);

const uint32_t BackgroundColor = BLACK;
const uint32_t TextColor = GREEN;
const uint8_t  TextSize = 3;
const uint8_t  TextSizeBig = TextSize + 2;
const uint8_t  ScreenRotation90Degrees = 1;              // // 0 (normal orientation), 1 (90 degrees clockwise), 2 (180 degrees), or 3 (90 degrees counterclockwise)
const uint8_t  ScreenRotation270Degrees = 3;              // // 0 (normal orientation), 1 (90 degrees clockwise), 2 (180 degrees), or 3 (90 degrees counterclockwise)
const unsigned long SerialPortBaudRate = 9600;
const unsigned long ConnectionTimeoutMs = 10 * 1000;
const unsigned long ConnectionRetryMs = 500;

const char* WifiSsid = "AirPort";
const char* WifiPassword = "ivacivac";

#define UltrasonicSensorTriggerPin 26
#define UltrasonicSensorEchoPin 36
const float UltrasonicSensorMinDistanceCm = 2;
const float UltrasonicSensorMaxDistanceCm = 400;
const float UltrasonicSensorUnknownDistance = -1;


/* After M5StickC is started or reset
  the program in the setUp () function will be run, and this part will only be run once.
  After M5StickCPlus is started or reset, the program in the setup() function will be executed, and this part will only be executed once. */
void setup()
{
  // Initialize the M5StickCPlus object. Initialize the M5StickCPlus object
  M5.begin();

  pinMode(UltrasonicSensorTriggerPin, OUTPUT);
  pinMode(UltrasonicSensorEchoPin, INPUT);

  M5.Lcd.setRotation(ScreenRotation90Degrees);  
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(BackgroundColor);
  M5.Lcd.setTextColor(TextColor, BackgroundColor);
  M5.Lcd.setTextSize(TextSize);

  Serial.begin(SerialPortBaudRate);
  WiFi.begin(WifiSsid, WifiPassword);

  M5.Lcd.println("Connecting");
  unsigned long connectStartTime = millis();
  char *connectionStatusStr = NULL;
  while (connectionStatusStr == NULL && millis() - connectStartTime < ConnectionTimeoutMs) {    
    switch(WiFi.status()) {
      case WL_CONNECTED:      connectionStatusStr = "Connected";      break;
      case WL_NO_SHIELD:      connectionStatusStr = "Not Supported";  break;
      case WL_NO_SSID_AVAIL:  connectionStatusStr = "SSID Not Found"; break;

      case WL_IDLE_STATUS:    
      case WL_SCAN_COMPLETED: 
      case WL_CONNECT_FAILED: 
      case WL_CONNECTION_LOST:
      case WL_DISCONNECTED:   
        delay(ConnectionRetryMs);
        M5.Lcd.print(".");
         break;
    }
  }

  if( connectionStatusStr == NULL )
     connectionStatusStr = "WIFI Timeout";

  M5.Lcd.println("");
  M5.Lcd.println(connectionStatusStr);
  delay(1000);

  setupRealTimeClockFromInternet();
}

/* After the program in setup() runs, it runs the program in loop()
The loop() function is an infinite loop in which the program runs repeatedly
After the program in the setup() function is executed, the program in the loop() function will be executed
The loop() function is an endless loop, in which the program will continue to run repeatedly */
void loop() 
{
  struct tm* dateTimeNow = getDateTimeNow();

  //M5.Lcd.fillScreen(BackgroundColor);
  M5.Lcd.setCursor(0, 0);

  char currentLocalTimeStr[16];
  strftime(currentLocalTimeStr, sizeof(currentLocalTimeStr), "%I:%M:%S %p", dateTimeNow);   // %H for 24 hour time, %I for 12 Hour time
  M5.Lcd.print(currentLocalTimeStr);  clearToEndOfLine();

  char currentLocalDateStr[16];
  strftime(currentLocalDateStr, sizeof(currentLocalDateStr), "%m:%d:%Y", dateTimeNow);
  M5.Lcd.print(currentLocalDateStr);  clearToEndOfLine();

  clearToEndOfLine();

  float distance = GetDistanceFeet();
  char distanceStr[10]; // Allocate a buffer to hold the formatted distance string
  if(distance == UltrasonicSensorUnknownDistance)
     strcpy(distanceStr, "  -.--"); 
  else 
     dtostrf(distance, 6, 2, distanceStr); // Convert distance to a string with 6 total characters and 2 decimal places

  M5.Lcd.setTextSize(TextSizeBig);
  M5.Lcd.setTextColor(BLUE, BackgroundColor);
  M5.Lcd.print(distanceStr);  M5.Lcd.print("ft"); clearToEndOfLine();
  M5.Lcd.setTextColor(TextColor, BackgroundColor);
  M5.Lcd.setTextSize(TextSize);

  delay(1000); // Wait for a second before sending the next message
}

void clearToEndOfLine() {
  // Get the screen dimensions and rotation
  int screenWidth = M5.Lcd.width();
  int screenHeight = M5.Lcd.height();
  //int screenRotation = M5.Lcd.getRotation();

  int currentX = M5.Lcd.getCursorX();
  int currentY = M5.Lcd.getCursorY();
  int lineHeight = M5.Lcd.fontHeight();

  int remainingWidth = screenWidth - currentX;
  M5.Lcd.fillRect(currentX, currentY, remainingWidth, lineHeight, BackgroundColor);
  M5.Lcd.println("");
}

// Returns UltrasonicSensorUnknownDistance (-1) if we couldn't read the value
//
float GetDistanceFeet() 
{
  digitalWrite(UltrasonicSensorTriggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(UltrasonicSensorTriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(UltrasonicSensorTriggerPin, LOW);

  long duration = pulseIn(UltrasonicSensorEchoPin, HIGH);

  /*
  ** When the HC-SR04 sensor sends an ultrasonic pulse, it travels to an object, reflects off of it, and then returns to the sensor.
  ** The sensor measures the total time it takes for the pulse to travel to the object and back, called the "round-trip time." To 
  ** find the distance to the object, we need to consider only the one-way time, i.e., the time it takes for the pulse to travel 
  ** from the sensor to the object. This is why we divide the round-trip time by 2.
  **
  ** Now let's examine the 0.0344 value. The speed of sound in air is approximately 343 meters per second (m/s) or 34,300 centimeters
  ** per second (cm/s) at room temperature (20°C). When we calculate the distance, we need to convert the time measured by the sensor
  ** (in microseconds) into distance (in centimeters). To do this, we can use the following formula:
  **
  **    distance (cm) = (time (µs) * speed of sound (cm/µs)) / 2
  **
  ** Since the speed of sound is 34,300 cm/s, we need to convert it to cm/µs:
  **
  **    34,300 cm/s * (1 s / 1,000,000 µs) = 0.0343 cm/µs (rounded to 0.0344 cm/µs for simplicity)
  **
  ** Now, we can rewrite the formula as:
  **
  **    distance (cm) = (time (µs) * 0.0344) / 2
  **
  ** So, the 0.0344 / 2 expression in the code is a result of the speed of sound conversion (from cm/s to cm/µs) and
  ** considering only the one-way travel time of the ultrasonic pulse.
  */
  float distanceCm = duration * 0.0344 / 2;

  // Check if the measured distance is within the valid range
  //
  if (distanceCm < UltrasonicSensorMinDistanceCm || distanceCm > UltrasonicSensorMaxDistanceCm) 
    return UltrasonicSensorUnknownDistance;

  // Convert CM to Feet 
  // 
  float distanceFeet = distanceCm * 0.0328084;

  return distanceFeet;
}

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
