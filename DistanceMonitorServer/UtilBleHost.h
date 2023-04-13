#ifndef UTIL_BLE_HOST_H
#define UTIL_BLE_HOST_H

void bleBeginHost();
void bleWriteFloatAsFixed16x8(float value);
float bleReadFloatFromFixed16x8(uint8_t *byteArray);
void bleWriteData(byte *data, size_t length);

#endif


