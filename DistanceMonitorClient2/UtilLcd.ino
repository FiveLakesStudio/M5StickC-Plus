#include <M5StickCPlus.h>
#include "UtilLcd.h"

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
