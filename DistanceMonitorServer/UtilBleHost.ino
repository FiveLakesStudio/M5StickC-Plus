#include <M5StickCPlus.h>
#include <NimBLEDevice.h>  // https://github.com/h2zero/NimBLE-Arduino
#include "UtilBleHost.h"

#define SERVICE_UUID_RIVIAN "f5b13a29-196a-4b42-bffa-85c6e44c6f00"
#define SERVICE_UUID_TESLA  "f5b13a29-196a-4b42-bffa-85c6e44c7000"

#define SERVICE_UUID SERVICE_UUID_TESLA
#define CHARACTERISTIC_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f01"



NimBLEServer* pServer;
NimBLEService* pService;
NimBLECharacteristic* pCharacteristic;

const uint8_t BleBufferMarkerStart = 0xAB;
const uint8_t BleBufferMarkerEnd = 0xEF;

const int MarkerStartIndex = 0;
const int HighByteIndex = 1;
const int MiddleByteIndex = 2;
const int LowByteIndex = 3;
const int SumHighByteIndex = 4;
const int SumLowByteIndex = 5;
const int MarkerEndIndex = 6;

const int BufferSize = 7;

const float InvalidFixedPointValue = -1.0;

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
  uint8_t byteArray[BufferSize];

  // Add the fixed value at the beginning of the buffer
  byteArray[MarkerStartIndex] = BleBufferMarkerStart;

  // This is a bit of a hack right now as I don't want to debug what's going on with signed floats and this doesn't really support
  // negactive distances anyway...
  //
  if( value < 0 ) {
    byteArray[HighByteIndex] = 0xFF;
    byteArray[MiddleByteIndex] = 0xFF;
    byteArray[LowByteIndex] = 0xFF;
    byteArray[SumHighByteIndex] = 0xFF;
    byteArray[SumLowByteIndex] = 0xFF;
  }
  else {
    int32_t intValue = (int32_t)(value * (1 << 8));

    // Convert the intValue into 3-byte representation
    byteArray[HighByteIndex] = (uint8_t)(intValue >> 16); // Store the high byte
    byteArray[MiddleByteIndex] = (uint8_t)((intValue >> 8) & 0xFF); // Store the middle byte
    byteArray[LowByteIndex] = (uint8_t)(intValue & 0xFF); // Store the low byte

    // Sum the first 3 bytes and store the result in the last 2 bytes
    uint16_t sum = byteArray[HighByteIndex] + byteArray[MiddleByteIndex] + byteArray[LowByteIndex];
    byteArray[SumHighByteIndex] = (uint8_t)(sum >> 8); // Store the high byte of the sum
    byteArray[SumLowByteIndex] = (uint8_t)(sum & 0xFF); // Store the low byte of the sum
  }

  // Add the fixed value at the end of the buffer
  byteArray[MarkerEndIndex] = BleBufferMarkerEnd;

  bleWriteData(byteArray, sizeof(byteArray));
}

void bleWriteData(byte *data, size_t length) {
  pCharacteristic->setValue(data, length);
}
