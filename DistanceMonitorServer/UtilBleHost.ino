#include <M5StickCPlus.h>
#include <BLEDevice.h>     // See https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/src/BLEDevice.h
//#include <BLEUtils.h>
//#include <BLEServer.h>
//#include <BLE2902.h>
#include "UtilBleHost.h"

#define SERVICE_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f22"
#define CHARACTERISTIC_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f99"

void bleBeginHost() {
  Serial.println("Starting BLE work!");

  BLEDevice::init("FLS_USS");
  
  BLEServer *pServer = BLEDevice::createServer();
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  
  pCharacteristic->setValue("Hello World... 123456");
  
  pService->start();
  
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  //pAdvertising->setMinPreferred(0x12);
  
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
  Serial.println(BLEDevice::getInitialized() ? "TRUE" : "FALSE");
}

// https://medium.com/@jalltechlab/bluetooth-ble-advertising-with-arduino-esp32-sample-code-no-coding-part-2-972deb23b1c3
// https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/src/BLEAdvertising.h

/*
void bleStartAdvertisingWithCustomData(byte *custom_data, size_t data_length) {
  unsigned long startTime = millis(); // Record start time

  gAdvertising->addServiceUUID(SERVICE_UUID);
  gAdvertising->setScanResponse(true);
  gAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  gAdvertising->setMinPreferred(0x12);

  BLEAdvertisementData advData;
  BLEAdvertisementData respData;

  // Add custom data to the advertising payload as manufacturer-specific data
  //advData.addData(0xFF, reinterpret_cast<char*>(custom_data), data_length);
  advData.addData("FLS-Test");
  gAdvertising->setAdvertisementData(advData);
  gAdvertising->setScanResponseData(respData);

  // Set advertising parameters
  gAdvertising->setMinInterval(0x80);  // Minimum advertising interval (in 0.625ms units)
  gAdvertising->setMaxInterval(0x200); // Maximum advertising interval (in 0.625ms units)
  gAdvertising->setAppearance(ESP_BLE_APPEARANCE_GENERIC_TAG);
  gAdvertising->setScanFilter(false, false);

  BLEDevice::startAdvertising();

  unsigned long endTime = millis(); // Record end time

  // Calculate and print the time taken
  unsigned long timeTaken = endTime - startTime;
  Serial.print("Time taken by bleStartAdvertisingWithCustomData: ");
  Serial.print(timeTaken);
  Serial.println(" ms");
}

void bleUpdateCustomDataAndAdvertise(byte *custom_data, size_t data_length) {
  //bleStartAdvertisingWithCustomData(custom_data, data_length);
  //Serial.println("Updated custom data and restarted advertising");
}
*/