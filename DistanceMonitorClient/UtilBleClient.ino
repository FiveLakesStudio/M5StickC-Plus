#include <M5StickCPlus.h>
#include <NimBLEDevice.h>  // https://github.com/h2zero/NimBLE-Arduino
#include "UtilBleClient.h"

#define SERVICE_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f00"
#define CHARACTERISTIC_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f01"

NimBLEAdvertisedDevice* pDevice;
NimBLEClient* pClient;
NimBLERemoteService* pRemoteService;
NimBLERemoteCharacteristic* pRemoteCharacteristic;

float bleReadFloatValue() 
{
  if (pRemoteCharacteristic == nullptr || !pRemoteCharacteristic->canRead()) 
    return -1.0;

  std::string value = pRemoteCharacteristic->readValue();
  uint8_t* pData = (uint8_t*)value.data();
  size_t length = value.length();
  float floatValue = bleReadFloatFromFixed16x8(pData);
  return floatValue;
}

void bleBeginClient() 
{
}

float bleReadFloatFromFixed16x8(uint8_t *byteArray) {
  union {
    int32_t intValue;
    uint8_t byteArray[3];
  } fixedPointBuffer;

  // Copy the byte array into the union struct
  memcpy(fixedPointBuffer.byteArray, byteArray, sizeof(fixedPointBuffer.byteArray));

  // Convert the 16.8 fixed-point value to a float
  float floatValue = (float)fixedPointBuffer.intValue / (1 << 8);

  return floatValue;
}
