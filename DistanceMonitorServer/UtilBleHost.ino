#include <M5StickCPlus.h>
#include <NimBLEDevice.h>  // https://github.com/h2zero/NimBLE-Arduino
#include "UtilBleHost.h"

#define SERVICE_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f00"
#define CHARACTERISTIC_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f01"

NimBLEServer* pServer;
NimBLEService* pService;
NimBLECharacteristic* pCharacteristic;

void bleBeginHost() 
{
  NimBLEDevice::init("FLS_USS");
  
  pServer = NimBLEDevice::createServer();
  
  pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  
  pCharacteristic->setValue("");
  
  pService->start();
  
  NimBLEAdvertising* pAdvertising = pServer->getAdvertising();
  
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x12);
  
  pAdvertising->start();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void bleWriteFloatAsFixed16x8(float value) {
  union {
    int32_t intValue;
    uint8_t byteArray[3];
  } fixedPointBuffer;

  fixedPointBuffer.intValue = (int32_t)(value * (1 << 8));
  bleWriteData(fixedPointBuffer.byteArray, sizeof(fixedPointBuffer.byteArray));
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

void bleWriteData(byte *data, size_t length) {
  pCharacteristic->setValue(data, length);
}
