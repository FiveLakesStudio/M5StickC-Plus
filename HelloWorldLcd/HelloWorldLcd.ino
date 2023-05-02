#include <M5StickCPlus.h>
#include <Arduino_GFX_Library.h>   // https://github.com/moononournation/Arduino_GFX

const uint32_t BackgroundColor = DARKGREY;
const uint32_t TextColor = GREEN;
const uint8_t  TextSize = 3;
const uint8_t  TextSizeBig = TextSize + 1;
const uint8_t  ScreenRotation90Degrees = 1;              // // 0 (normal orientation), 1 (90 degrees clockwise), 2 (180 degrees), or 3 (90 degrees counterclockwise)
const uint8_t  ScreenRotation270Degrees = 3;              // // 0 (normal orientation), 1 (90 degrees clockwise), 2 (180 degrees), or 3 (90 degrees counterclockwise)
const unsigned long SerialPortBaudRate = 115200;

Arduino_DataBus *bus = new Arduino_HWSPI(32 /* dc - Yellow-2 */, 25 /* CS/SS -  Yellow*/, 0 /* Clock - Green */, 26 /* DC/MOSI - Orange */);
Arduino_GFX *gfx = new Arduino_TFT(bus, 33 /* RST - White */);


void setup() {
  M5.begin();

  M5.Lcd.setRotation(ScreenRotation90Degrees);  
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(BackgroundColor);
  M5.Lcd.setTextColor(TextColor, BackgroundColor);
  M5.Lcd.setTextSize(TextSize);
  M5.Lcd.println("Hello");

  gfx->begin();
  gfx->fillScreen(BLACK);
  gfx->setCursor(10, 10);
  gfx->setTextColor(RED);
  gfx->println("Hello World!");
}

void loop() {
  //M5.update();

}
