#include <M5StickCPlus.h>
#include <NimBLEDevice.h>           // https://github.com/h2zero/NimBLE-Arduino
#include <NimBLEAdvertisedDevice.h>
#include "UtilBleScanner.h"

#define SERVICE_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f00"

const int bleScanTimeSeconds = 10; //In seconds

NimBLEScan* pScan = nullptr;
NimBLEAdvertisedDevice* foundDevice = nullptr;

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

bool bleFindDevices() {
  if (foundDevice == nullptr) {   
    if( pScan->isScanning() )
       return false;

    Serial.println("Scanning for devices with service UUID ...");
    pScan->start(bleScanTimeSeconds, nullptr, true);

    return false;
  }
}

void bleBeginClient() 
{
  NimBLEDevice::init("");
  
  pScan = NimBLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true); //active scan uses more power, but get results faster
  pScan->setInterval(100);
  pScan->setWindow(99);  // less or equal setInterval value
}

