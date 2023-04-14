#include <M5StickCPlus.h>
#include <NimBLEDevice.h>           // https://github.com/h2zero/NimBLE-Arduino
#include <NimBLEAdvertisedDevice.h>
#include "UtilBleClient.h"

#define SERVICE_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f00"
#define CHARACTERISTIC_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f01"

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

const int bleScanTimeSeconds = 5; //In seconds

NimBLEScan* pScan = nullptr;
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

// Returns true if we are connected and good to go!
//
bool bleFindAndConnectToDeviceIfNeeded() {
  if (foundDevice == nullptr) {   
    if( pScan->isScanning() )
       return false;

    Serial.println("Scanning for devices with service UUID ...");
    pScan->start(bleScanTimeSeconds, nullptr, true);

    return false;
  }

  if( pClient != nullptr && pClient->isConnected() )
    return true;

  Serial.print("BLE Trying to Connect to ");  Serial.println(foundDevice->getAddress().toString().c_str());
  pClient = NimBLEDevice::createClient(foundDevice->getAddress());

  if (!pClient->connect(foundDevice)) {
    Serial.print("BLE Failed to connect!");  Serial.println(foundDevice->getAddress().toString().c_str());
    return false;
  }

  Serial.println("BLE Connected!"); Serial.println(foundDevice->getAddress().toString().c_str());
  pRemoteService = pClient->getService(NimBLEUUID(SERVICE_UUID));
  pRemoteCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID(CHARACTERISTIC_UUID));
}

void bleBeginClient() 
{
  NimBLEDevice::init("");
  
  pScan = NimBLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true); //active scan uses more power, but get results faster
  pScan->setInterval(100);
  pScan->setWindow(99);  // less or equal setInterval value

  bleFindAndConnectToDeviceIfNeeded();
}

float bleReadFloatValue() 
{
  if( !bleFindAndConnectToDeviceIfNeeded() )
    return -1.0;

  if (pRemoteCharacteristic == nullptr || !pRemoteCharacteristic->canRead()) 
    return -1.0;

  std::string value = pRemoteCharacteristic->readValue();
  uint8_t* pData = (uint8_t*)value.data();
  size_t length = value.length();

  if( length != BufferSize )
     return -1.0;

  float floatValue = bleReadFloatFromFixed16x8(pData);
  return floatValue;
}

float bleReadFloatFromFixed16x8(uint8_t *byteArray) {
  // Verify the start marker
  if (byteArray[MarkerStartIndex] != BleBufferMarkerStart) {
    return InvalidFixedPointValue;
  }

  int32_t intValue = 0;

  // Combine the 3 bytes into an int32_t value
  intValue |= (int32_t)byteArray[HighByteIndex] << 16;
  intValue |= (int32_t)byteArray[MiddleByteIndex] << 8;
  intValue |= (int32_t)byteArray[LowByteIndex];

  // Verify the end marker
  if (byteArray[MarkerEndIndex] != BleBufferMarkerEnd) {
    return InvalidFixedPointValue;
  }

  // Check if the computed sum is correct
  uint16_t sum = byteArray[HighByteIndex] + byteArray[MiddleByteIndex] + byteArray[LowByteIndex];
  uint16_t receivedSum = (uint16_t)(byteArray[SumHighByteIndex] << 8) | byteArray[SumLowByteIndex];

  if (sum != receivedSum) {
    return InvalidFixedPointValue;
  }

  // Convert the fixed-point 16x8 value to a float
  float floatValue = (float)intValue / (1 << 8);

  return floatValue;
}