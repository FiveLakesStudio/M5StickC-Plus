#include <string.h> 
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <NTPClient.h>   // https://github.com/arduino-libraries/NTPClient NTPClient by Fabrice Weinberg
#include <TimeLib.h>     // https://playground.arduino.cc/Code/Time/       Time by Michael Margolis
#include <Timezone.h>    // https://github.com/JChristensen/Timezone       Jack Christensen
#include "UtilLcd.h"
#include "UtilClock.h"
#include "UtilUltrasonicSensor.h"
#include "UtilBleClient.h"

const uint32_t BackgroundColor = DARKGREY;
const uint32_t TextColor = GREEN;
const uint8_t  TextSize = 3;
const uint8_t  TextSizeBig = TextSize + 1;
const uint8_t  ScreenRotation90Degrees = 1;              // // 0 (normal orientation), 1 (90 degrees clockwise), 2 (180 degrees), or 3 (90 degrees counterclockwise)
const uint8_t  ScreenRotation270Degrees = 3;              // // 0 (normal orientation), 1 (90 degrees clockwise), 2 (180 degrees), or 3 (90 degrees counterclockwise)
const unsigned long SerialPortBaudRate = 115200;
const unsigned long ConnectionTimeoutMs = 10 * 1000;
const unsigned long ConnectionRetryMs = 500;

const float differenceThreshold = 0.1;

const float MaxDistanceToShowStopRivian = 1.0;
const float MaxDistanceToShowStopTesla = 2.0;

const uint32_t LoopDelayMs = 10;

const char* WifiSsid = "AirPort";
const char* WifiPassword = "ivacivac";

const unsigned long RebootIntervalMs = 30 * 60 * 1000;  // 30 Minutes
static unsigned long rebootIfNeededTime = 0;            // Rename the variable to rebootIfNeededTime

UtilBleClient bleClientRivian(MacAddressRivian);
UtilBleClient bleClientTesla(MacAddressTesla);

enum DistanceToShow {
  Any,
  Rivian,
  Tesla
};

DistanceToShow distanceToShow = Any;  // set initial value

void rebootIfNeeded(bool force, bool verbose) {
  if (rebootIfNeededTime == 0) {
    rebootIfNeededTime = millis(); // Store the time when the function is called for the first time
  }

  if (!force) {
    if (millis() - rebootIfNeededTime < RebootIntervalMs) {
      return;
    }
  }

  const int16_t textPosY = TextSize*TextSizeBase*2;

  if( verbose ) {
    M5.Lcd.setTextColor(TextColor);
    M5.Lcd.setTextSize(TextSize);
      
    M5.Lcd.fillScreen(BackgroundColor);
    M5.Lcd.setCursor(0, textPosY);
    M5.Lcd.println("Reboot in 3");
    ledPrintln("Boot 3");
    delay(1000); // Wait for a while to allow the display to update
    M5.Lcd.setCursor(0, textPosY);
      
    M5.Lcd.fillScreen(BackgroundColor);
    M5.Lcd.setCursor(0, textPosY);
    M5.Lcd.println("Reboot in 2");
    ledPrintln("Boot 2");
    delay(1000); // Wait for a while to allow the display to update
      
    M5.Lcd.fillScreen(BackgroundColor);
    M5.Lcd.setCursor(0, textPosY);
    M5.Lcd.println("Reboot in 1");
    ledPrintln("Boot 1");
    delay(1000); // Wait for a while to allow the display to update

    M5.Lcd.fillScreen(RED);
    ledPrintln("");
  }

  esp_restart(); // Reboot the M5StickC
}

void setup()
{
  M5.begin();

  M5.Lcd.setRotation(ScreenRotation90Degrees);  
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(BackgroundColor);
  M5.Lcd.setTextColor(TextColor, BackgroundColor);
  M5.Lcd.setTextSize(TextSize);

  Serial.begin(SerialPortBaudRate);
  WiFi.begin(WifiSsid, WifiPassword);

  ledMaxBegin();

  bleClientRivian.begin();
  bleClientTesla.begin();

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

const uint32_t NoChangeDistanceTimeoutMs = 10 * 1000; // 10 seconds

// Add a global variable to store the previous distance and the time of the last change
float previousDistance = UltrasonicSensorUnknownDistance;
unsigned long lastDistanceChangeTime = 0;

const unsigned long LedClearIntervalMs = 10 * 1000; // 10 seconds
unsigned long lastLedClearTime = 0;

bool isDistanceSignificantlyDifferent(float currentDistance, float lastReadDistance, float threshold) {
  if (currentDistance == -1) {
    return false;
  }
  return abs(currentDistance - lastReadDistance) > threshold;
}

void loop() 
{ 
  M5.update();

  // Check if button A is pressed
  if (M5.BtnA.pressedFor(500)) {
    rebootIfNeeded(true, true);
  }

  bool ledAnimate();

  struct tm* dateTimeNow = getDateTimeNow();

  M5.Lcd.setCursor(0, TextSize*TextSizeBase*0);
  lcdPrintTimeIfNeeded();

  M5.Lcd.setCursor(0, TextSize*TextSizeBase*1);
  lcdPrintDateIfNeeded();

  M5.Lcd.setCursor(0, TextSize*TextSizeBase*2);
  M5.Lcd.print(bleClientRivian.isConnected() ? "Rivian/" : "None/");
  M5.Lcd.print(bleClientTesla.isConnected() ? "Tesla" : "None");
  clearToEndOfLine();

  M5.Lcd.setCursor(0, TextSize*TextSizeBase*3);

  float distanceRivianLastRead = bleClientRivian.lastReadFloatValue;
  float distanceTeslaLastRead = bleClientTesla.lastReadFloatValue;
  float distanceRivian = distanceRivianLastRead;
  float distanceTesla = distanceTeslaLastRead;
  float distance = -1;
  char *distanceUsingStr = "";
  float maxDistanceToShowStop = -1;

  switch (distanceToShow) {
    case Any:
      distanceRivian = bleClientRivian.readFloatValue();
      distanceTesla = bleClientTesla.readFloatValue();
      break;

    case Rivian:
      distanceRivian = bleClientRivian.readFloatValue();
      break;

    case Tesla:
      distanceTesla = bleClientTesla.readFloatValue();
      break;
  }

  // If we see a value significantly change then lock in that Vehicle until we show the time.
  // This allows for faster updating as we can focus on a single vehicle.
  //
  if(isDistanceSignificantlyDifferent(distanceRivian, distanceRivianLastRead, differenceThreshold)) {
    distanceToShow = DistanceToShow::Rivian;
  } else if(isDistanceSignificantlyDifferent(distanceTesla, distanceTeslaLastRead, differenceThreshold)){
    distanceToShow = DistanceToShow::Tesla;
  }

  switch (distanceToShow) {
    case Any:
      distanceUsingStr = "*";
      //Serial.print("None: ");  Serial.print(distance); Serial.print(" "); Serial.print(distanceRivian); Serial.print(" "); Serial.print(distanceTesla); Serial.print(""); Serial.println("");
      break;

    case Rivian:
      distanceUsingStr = "R";
      distance = distanceRivian;
      maxDistanceToShowStop = MaxDistanceToShowStopRivian;
      //Serial.print("Rivian: ");  Serial.print(distance); Serial.print(" "); Serial.print(distanceRivianLastRead); Serial.print(" "); Serial.println("");
      break;

    case Tesla:
      distanceUsingStr = "T";
      distance = distanceTesla;
      maxDistanceToShowStop = MaxDistanceToShowStopTesla;
      //Serial.print("Tesla: ");  Serial.print(distance); Serial.print(" "); Serial.print(distanceTeslaLastRead); Serial.print(" "); Serial.println("");
      break;
  }

  char distanceStr[14]; // Allocate a buffer to hold the formatted distance string
  if(distance == UltrasonicSensorUnknownDistance || maxDistanceToShowStop == -1)
     strcpy(distanceStr, "  None"); 
  else if(distance < maxDistanceToShowStop) {
    strncpy(distanceStr, " STOP", sizeof(distanceStr));
    distanceStr[sizeof(distanceStr) - 1] = '\0'; // Ensure null termination
  } else {
    dtostrf(distance - maxDistanceToShowStop, 5, 1, distanceStr); // Convert distance to a string with 6 total characters and 2 decimal places
    strcat(distanceStr, "'"); // Append "ft" to the string
  }
  
  if (isDistanceSignificantlyDifferent(distance, previousDistance, differenceThreshold)) {
    previousDistance = distance;
    lastDistanceChangeTime = millis();
  }

  M5.Lcd.setTextSize(TextSizeBig);
  M5.Lcd.setTextColor(BLUE, BackgroundColor);
  M5.Lcd.print(distanceUsingStr); M5.Lcd.print(distanceStr); clearToEndOfLine();

  if( millis() - lastDistanceChangeTime > NoChangeDistanceTimeoutMs ) {
    rebootIfNeeded(false, false);

    // Clear the LED periodically to see if we can recover from the display getting confused.
    //if (millis() - lastLedClearTime >= LedClearIntervalMs) {
    //  ledClear(); // Clear the LED display
    //  lastLedClearTime = millis(); // Update the last time ledClear() was called
    //}

    struct tm* dateTimeNow = getDateTimeNow();
    char currentTimeStr[16];
    strftime(currentTimeStr, sizeof(currentTimeStr), dateTimeNow->tm_sec % 2 == 0 ? "%I:%M" : "%I %M", dateTimeNow);
    strcat(currentTimeStr, dateTimeNow->tm_hour >= 12 ? "P" : "A");
    ledPrintln( currentTimeStr );

    distanceToShow = DistanceToShow::Any;
  } else {
      ledPrintln(distanceStr);
      rebootIfNeededTime = 0;    // Reset our 45 minute reboot time as we are showing the distance and we don't want to reboot during that!
  }
  
  M5.Lcd.setTextColor(TextColor, BackgroundColor);
  M5.Lcd.setTextSize(TextSize);

  if( resetDisplayIfNeeded() )
    delay(LoopDelayMs);

  delay(LoopDelayMs); // Wait for a second before sending the next message
}


