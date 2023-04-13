#include <M5StickCPlus.h>
#include <NimBLEDevice.h>  // https://github.com/h2zero/NimBLE-Arduino
#include "UtilBleClient.h"

#define SERVICE_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f00"
#define CHARACTERISTIC_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f01"

NimBLEAdvertisedDevice* pDevice;
NimBLEClient* pClient;
NimBLERemoteService* pRemoteService;
NimBLERemoteCharacteristic* pRemoteCharacteristic;

float gDistance = UltrasonicSensorUnknownDistance;

void notifyCallback(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length)
{
  float value = bleReadFloatFromFixed16x8(pData);
  Serial.print("Received value: ");
  Serial.println(value);
  gDistance = value;
}

void bleBeginClient() 
{
  NimBLEDevice::init("FLS_USS_Client");
  NimBLEScan* pScan = NimBLEDevice::getScan();

  // Set scan parameters
  pScan->setInterval(0x50);
  pScan->setWindow(0x30);
  pScan->setActiveScan(true);
  pScan->setFilter(NimBLEScanFilter());
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  
  // Start scanning for devices
  Serial.println("Scanning...");
  pScan->start(5, false);

  // Wait for scan to complete
  while (pScan->isScanning()) {
    delay(10);
  }

  // Connect to the first found device
  if (MyAdvertisedDeviceCallbacks::m_pDevice != nullptr) {
    pDevice = MyAdvertisedDeviceCallbacks::m_pDevice;
    pClient = NimBLEDevice::createClient();

    Serial.print("Connecting to device: ");
    Serial.println(pDevice->getName().c_str());

    pClient->connect(pDevice);
    Serial.println("Connected!");

    pRemoteService = pClient->getService(NimBLEUUID(SERVICE_UUID));
    pRemoteCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID(CHARACTERISTIC_UUID));

    if (pRemoteCharacteristic->canNotify()) {
      pRemoteCharacteristic->registerForNotify(notifyCallback);
      Serial.println("Registered for notifications");
    }
  }
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
