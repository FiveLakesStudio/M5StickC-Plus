#include <string.h> 
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <NTPClient.h>   // https://github.com/arduino-libraries/NTPClient NTPClient by Fabrice Weinberg
#include <TimeLib.h>     // https://playground.arduino.cc/Code/Time/       Time by Michael Margolis
#include <Timezone.h>    // https://github.com/JChristensen/Timezone       Jack Christensen
#include "UtilLcd.h"
#include "UtilClock.h"
#include "UtilUltrasonicSensor.h"
#include "UtilBleHost.h"

const uint32_t BackgroundColor = BLACK;
const uint32_t TextColor = GREEN;
const uint8_t  TextSize = 3;
const uint8_t  TextSizeBig = TextSize + 1;
const uint8_t  ScreenRotation90Degrees = 1;              // // 0 (normal orientation), 1 (90 degrees clockwise), 2 (180 degrees), or 3 (90 degrees counterclockwise)
const uint8_t  ScreenRotation270Degrees = 3;              // // 0 (normal orientation), 1 (90 degrees clockwise), 2 (180 degrees), or 3 (90 degrees counterclockwise)
const unsigned long SerialPortBaudRate = 115200;
const unsigned long ConnectionTimeoutMs = 10 * 1000;
const unsigned long ConnectionRetryMs = 500;

const uint32_t LoopDelayMs = 100;

const char* WifiSsid = "AirPort";
const char* WifiPassword = "ivacivac";

/* After M5StickC is started or reset
  the program in the setUp () function will be run, and this part will only be run once.
  After M5StickCPlus is started or reset, the program in the setup() function will be executed, and this part will only be executed once. */
void setup()
{
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

  bleBeginHost();
  //byte initial_custom_data[] = {0x12, 0x34, 0x56, 0x78};
  //bleStartAdvertisingWithCustomData(initial_custom_data, sizeof(initial_custom_data));

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

  M5.Lcd.setCursor(0, TextSize*TextSizeBase*0);
  ledPrintTimeIfNeeded();

  M5.Lcd.setCursor(0, TextSize*TextSizeBase*1);
  ledPrintDateIfNeeded();

  M5.Lcd.setCursor(0, TextSize*TextSizeBase*2);
  clearToEndOfLine();

  M5.Lcd.setCursor(0, TextSize*TextSizeBase*3);

  float distance = GetDistanceFeetAverage(UltrasonicSensorSampleCount);

  char distanceStr[10]; // Allocate a buffer to hold the formatted distance string
  if(distance == UltrasonicSensorUnknownDistance)
     strcpy(distanceStr, "  -.--"); 
  else 
     dtostrf(distance, 5, 1, distanceStr); // Convert distance to a string with 6 total characters and 2 decimal places

  M5.Lcd.setTextSize(TextSizeBig);
  M5.Lcd.setTextColor(BLUE, BackgroundColor);
  M5.Lcd.print(distanceStr);  M5.Lcd.print("ft"); clearToEndOfLine();
  M5.Lcd.setTextColor(TextColor, BackgroundColor);
  M5.Lcd.setTextSize(TextSize);

  bleWriteFloatAsFixed16x8(distance);

  delay(LoopDelayMs); // Wait for a second before sending the next message
}


