#include <string.h> 
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <NTPClient.h>   // https://github.com/arduino-libraries/NTPClient NTPClient by Fabrice Weinberg
#include <TimeLib.h>     // https://playground.arduino.cc/Code/Time/       Time by Michael Margolis
#include <Timezone.h>    // https://github.com/JChristensen/Timezone       Jack Christensen
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "UtilLcd.h"
#include "UtilClock.h"

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
const uint8_t UltrasonicSensorSampleCount = 5;
const unsigned long UltrasonicSensorSampleDelayMs = 10;

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

  float distance = GetDistanceFeetAverage(UltrasonicSensorSampleCount);
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

float GetDistanceFeetAverage(uint8_t numSamples)
{
  float sum = 0;
  int numSamplesTaken = 0; 
  float distance = UltrasonicSensorUnknownDistance;

  for (uint8_t index = 0; index < numSamples; index += 1) {
    distance = GetDistanceFeet();
    if( distance == UltrasonicSensorUnknownDistance )
       continue;
    numSamplesTaken += 1;
    sum += distance;
    delay(UltrasonicSensorSampleDelayMs); // Add a short delay between readings
  }

  if( distance == UltrasonicSensorUnknownDistance || numSamplesTaken == 0)
     return UltrasonicSensorUnknownDistance;

  return sum / numSamplesTaken;
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


