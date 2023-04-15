#include <M5StickCPlus.h>
#include <MD_Parola.h>    // https://github.com/MajicDesigns/MD_Parola search for "MajicDesigns MD_Parola" in library manager 
#include <MD_MAX72xx.h>   // https://github.com/MajicDesigns/MD_MAX72XX search for "MajicDesigns Max7219" in library manager 
#include <SPI.h>
#include "UtilMax7219.h"

// Pin connections for M5StickC Plus to MAX7219 module
//
#define DATA_PIN  26 // or MOSI ORANGE
#define CLK_PIN   0  // or SCK  Green
#define CS_PIN    25 // or SS   YELLOW

// Hardware configuration for the MAX7219 module
//
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

// Create the objects needed for doing text and controlling the display
//
MD_Parola mxParola = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

unsigned long resetInterval = 60 * 60 * 1000; // 1 hour in milliseconds
unsigned long lastResetTime = 0;

void ledMaxBegin() {
  mxParola.begin();
  mxParola.setIntensity(5); // set brightness to 5 (out of 15)

  mxParola.displayReset();

  ledPrintln("Hello");
  ledAnimate();
}

bool resetDisplayIfNeeded() {
  unsigned long currentTime = millis();

  if (mxParola.getWriteError()) {
    Serial.println("Write error occurred!");
    mxParola.clearWriteError();
  }

  if (currentTime - lastResetTime < resetInterval)
    return false;
   
  //mxParola.displayReset();
  mxParola.setIntensity(5); // set brightness to 5 (out of 15)
  mxParola.displayClear();
  mxParola.clearWriteError();

  lastResetTime = currentTime;
  return true;
}

void ledPrintln(char *text) {
  // Declare a static buffer to store the last displayed text
  static char lastText[32] = {0};

  // Check the input text length and truncate it if necessary
  char truncatedText[32];
  strncpy(truncatedText, text, sizeof(truncatedText) - 1);
  truncatedText[sizeof(truncatedText) - 1] = '\0';

  // Check if the new text is the same as the last displayed text
  if (strcmp(truncatedText, lastText) == 0) {
    // If the text is the same, don't update the display and return
    return;
  }

  // Update the display with the new text
  mxParola.displayText(truncatedText, PA_CENTER, 0, 0, PA_PRINT, PA_PRINT);

  // If there was an error, go ahead and clear the error
  if (mxParola.getWriteError()) {
    Serial.println("Write error occurred!");
    mxParola.clearWriteError();
  }

  ledAnimate();

  // Update the last displayed text with the new text
  strncpy(lastText, truncatedText, sizeof(lastText) - 1);
  lastText[sizeof(lastText) - 1] = '\0';
}

bool ledAnimate() {
  return mxParola.displayAnimate();
}