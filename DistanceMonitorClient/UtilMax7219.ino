#include <M5StickCPlus.h>
#include <MD_Parola.h>    // https://github.com/MajicDesigns/MD_Parola search for "MajicDesigns MD_Parola" in library manager 
#include <MD_MAX72xx.h>   // https://github.com/MajicDesigns/MD_MAX72XX search for "MajicDesigns Max7219" in library manager 
#include <SPI.h>
#include "UtilMax7219.h"

// Pin connections for M5StickC Plus to MAX7219 module
//
#define DATA_PIN 26  // DIN Orange
#define CLK_PIN 36   // 36 Green
#define CS_PIN 32     // Yellow 

// Hardware configuration for the MAX7219 module
//
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

// Create the objects needed for doing text and controlling the display
//
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
MD_Parola mxParola = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void ledMaxBegin() {
  mxParola.begin();

  ledPrintln("Welcome...");
  ledAnimate();
}

void ledPrintln(char *text) {
  //  mxParola.displayReset();
  mxParola.displayText(text, PA_CENTER, 0, 0, PA_PRINT, PA_PRINT);
}

bool ledAnimate() {
  return mxParola.displayAnimate();
}