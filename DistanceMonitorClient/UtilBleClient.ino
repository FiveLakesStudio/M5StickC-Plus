#include <M5StickCPlus.h>
#include <NimBLEDevice.h>           // https://github.com/h2zero/NimBLE-Arduino
#include <NimBLEAdvertisedDevice.h>
#include "UtilBleClient.h"

#define SERVICE_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f00"
#define CHARACTERISTIC_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f01"

const int bleScanTimeSeconds = 5; //In seconds

NimBLEAdvertisedDevice* foundDevice = nullptr;
NimBLEClient* pClient = nullptr;
NimBLERemoteService* pRemoteService = nullptr;
NimBLERemoteCharacteristic* pRemoteCharacteristic = nullptr;

class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
    if (advertisedDevice->isAdvertisingService(NimBLEUUID(SERVICE_UUID))) {
      Serial.print("Found Device: ");
      Serial.print(advertisedDevice->getAddress().toString().c_str());
      Serial.print(" RSSI: ");
      Serial.println(advertisedDevice->getRSSI());
      
      foundDevice = new NimBLEAdvertisedDevice(*advertisedDevice);
    }
  }
};

void bleConnectToFoundDevice() {
  if (foundDevice == nullptr) {
    Serial.println("No device found with the specified service UUID.");
    return;
  }

  pClient = NimBLEDevice::createClient(foundDevice->getAddress());
  Serial.print("Connecting to found device: ");
  Serial.println(foundDevice->getAddress().toString().c_str());

  if (pClient->connect(foundDevice)) {
    Serial.println("BLE Connected!");
    pRemoteService = pClient->getService(NimBLEUUID(SERVICE_UUID));
    pRemoteCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID(CHARACTERISTIC_UUID));
  } else {
    Serial.println("BLE Failed to connect!");
  }
}

void bleBeginClient() 
{
  NimBLEDevice::init("");
  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true); //active scan uses more power, but get results faster
  pScan->setInterval(100);
  pScan->setWindow(99);  // less or equal setInterval value

  Serial.println("Scanning for devices with service UUID ...");
  pScan->start(bleScanTimeSeconds, nullptr, true);

  while (pScan->isScanning()) {
    delay(10);
  }

  bleConnectToFoundDevice();
}

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
