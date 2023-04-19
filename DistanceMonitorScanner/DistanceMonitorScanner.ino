#include <string.h>
#include <M5StickCPlus.h>
#include "UtilLcd.h"
#include "UtilBleScanner.h"

const uint32_t BackgroundColor = BLACK;
const uint32_t TextColor = GREEN;
const uint8_t TextSizeBase = 8;
const uint8_t TextSize = 3;
const uint8_t TextSizeBig = TextSize + 1;
const uint8_t ScreenRotation90Degrees = 1;
const uint8_t ScreenRotation270Degrees = 3;
const unsigned long SerialPortBaudRate = 115200;

const uint32_t LoopDelayMs = 250;

UtilBleScanner bleScanner;

void setup() {
  M5.begin();

  M5.Lcd.setRotation(ScreenRotation90Degrees);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(BackgroundColor);
  M5.Lcd.setTextColor(TextColor, BackgroundColor);
  M5.Lcd.setTextSize(TextSize);

  Serial.begin(SerialPortBaudRate);
  Serial.println("Starting Scanner");

  bleScanner.beginClient();
}

void loop() {
  M5.update();

  M5.Lcd.setCursor(0, TextSize * TextSizeBase * 0);
  M5.Lcd.print("Scanning..."); clearToEndOfLine();

  bleScanner.findDevices();

  if (bleScanner.foundDeviceRivian != nullptr) {
    M5.Lcd.setCursor(0, TextSize * TextSizeBase * 1);
    M5.Lcd.print("Rivian: ");
    M5.Lcd.print(bleScanner.foundDeviceRivian->getAddress().toString().c_str());
    clearToEndOfLine();
  }

  if (bleScanner.foundDeviceTesla != nullptr) {
    M5.Lcd.setCursor(0, TextSize * TextSizeBase * 2);
    M5.Lcd.print("Tesla: ");
    M5.Lcd.print(bleScanner.foundDeviceTesla->getAddress().toString().c_str());
    clearToEndOfLine();
  }

  delay(LoopDelayMs); // Wait for a second before sending the next message
}
