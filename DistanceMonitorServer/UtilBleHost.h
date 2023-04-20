#ifndef UTIL_BLE_HOST_H
#define UTIL_BLE_HOST_H

#define MacAddressRivian "e8:9f:6d:08:d4:f6"
#define MacAddressTesla "4c:75:25:cd:ef:d2"

void bleBeginHost();
void bleWriteFloatAsFixed16x8(float value);
float bleReadFloatFromFixed16x8(uint8_t *byteArray);
void bleWriteData(byte *data, size_t length);
const char* bleGetName();

#endif


