#include <M5StickCPlus.h>
#include <NimBLEDevice.h>  // https://github.com/h2zero/NimBLE-Arduino
#include "UtilBleClient.h"

#define SERVICE_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f00"
#define CHARACTERISTIC_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f01"

NimBLEAdvertisedDevice* pDevice;
NimBLEClient* pClient;
NimBLERemoteService* pRemoteService;
NimBLERemoteCharacteristic* pRemoteCharacteristic;

//float gDistance = UltrasonicSensorUnknownDistance;

//void notifyCallback(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length)
//{
//  float value = bleReadFloatFromFixed16x8(pData);
//  Serial.print("Received value: ");
//  Serial.println(value);
//  gDistance = value;
//}

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
  NimBLEDevice::init("FLS_USS_Client");
  NimBLEDevice::setScanFilterMode(NimBLEScan::SCAN_MODE_LOW_LATENCY);
  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());

  scanForServiceUUID(SERVICE_UUID);

  // Wait for scan to complete
  while (pScan->isScanning()) {
    delay(10);
  }

  // Connect to the first found device with the name "FLS_USS"
  //
  for (int i = 0; i < pScan->getResults()->getCount(); i++) {
    //NimBLEAdvertisedDevice advertisedDevice = pScan->getResults()->getDevice(i);
    //if (advertisedDevice.getName() != "FLS_USS")
    //   continue;
     
    pDevice = new NimBLEAdvertisedDevice(advertisedDevice);
    pClient = NimBLEDevice::createClient();

    Serial.print("Connecting to device: ");
    Serial.println(pDevice->getName().c_str());

    pClient->connect(pDevice);
    Serial.println("Connected!");

    pRemoteService = pClient->getService(NimBLEUUID(SERVICE_UUID));
    pRemoteCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID(CHARACTERISTIC_UUID));

    //if (pRemoteCharacteristic->canNotify()) {
    //  pRemoteCharacteristic->registerForNotify(notifyCallback);
    //  Serial.println("Registered for notifications");
    //}
    break;
  }
}

class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
    Serial.print("Found Device: ");
    Serial.print(advertisedDevice->getAddress().toString().c_str());
    Serial.print(" RSSI: ");
    Serial.println(advertisedDevice->getRSSI());
  }
};

void scanForServiceUUID(const char* uuid) {
  NimBLEUUID serviceUUID(uuid);
  NimBLEScan* pScan = NimBLEDevice::getScan();

  pScan->clearResults();
  pScan->setInterval(45);
  pScan->setWindow(15);
  pScan->setActiveScan(true);
  pScan->setFilterPolicy(NimBLEScan::FILTER_POLICY_SCAN_ALL);
  pScan->setFilterDuplicate(true);

  NimBLEScanFilter filter;
  filter.setServiceUUID(serviceUUID);
  pScan->addFilter(filter);

  Serial.println("Scanning for devices with service UUID ...");
  pScan->start(5, nullptr, true);
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
