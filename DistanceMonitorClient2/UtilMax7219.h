#ifndef UTIL_MAX_7219_H
#define UTIL_MAX_7219_H

void ledMaxBegin();
bool resetDisplayIfNeeded();
void ledPrintln(char *text);
bool ledAnimate();
void ledClear();
#endif


