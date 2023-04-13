#ifndef UTIL_BLE_CLIENT_H
#define UTIL_BLE_CLIENT_H

//extern float gDistance;

void bleBeginClient();
float bleReadFloatValue();

float bleReadFloatFromFixed16x8(uint8_t *byteArray);
#endif


