#include "UtilBleScanner.h"

#define SERVICE_UUID "f5b13a29-196a-4b42-bffa-85c6e44c6f00"

void UtilBleScanner::MyAdvertisedDeviceCallbacks::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
  if (advertisedDevice->isAdvertisingService(NimBLEUUID(SERVICE_UUID))) {
    Serial.print("Found Device: ");
    Serial.print(advertisedDevice->getAddress().toString().c_str());
    Serial.print(" RSSI: ");
    Serial.println(advertisedDevice->getRSSI());

    parent->foundDevices.push_back(new NimBLEAdvertisedDevice(*advertisedDevice));
  }
}

UtilBleScanner::UtilBleScanner() : bleScanTimeSeconds(10) {
}

bool UtilBleScanner::findDevices() {
  if (foundDevices.empty()) {
    if (pScan->isScanning())
      return false;

    Serial.println("Scanning for devices with service UUID ...");
    pScan->start(bleScanTimeSeconds, nullptr, true);

    return false;
  }
}

void UtilBleScanner::beginClient() {
  NimBLEDevice::init("");

  pScan = NimBLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(this));
  pScan->setActiveScan(true); //active scan uses more power, but get results faster
  pScan->setInterval(100);
  pScan->setWindow(99); // less or equal setInterval value
}
