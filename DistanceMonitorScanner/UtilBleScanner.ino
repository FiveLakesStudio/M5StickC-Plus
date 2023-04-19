#include "UtilBleScanner.h"

#define SERVICE_UUID_RIVIAN "f5b13a29-196a-4b42-bffa-85c6e44c6f00"
#define SERVICE_UUID_TESLA "12345678-90ab-cdef-1234-567890abcdef"

void UtilBleScanner::MyAdvertisedDeviceCallbacks::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
  if (advertisedDevice->isAdvertisingService(NimBLEUUID(SERVICE_UUID_RIVIAN))) {
    Serial.print("Found Rivian Device: ");
    Serial.print(advertisedDevice->getAddress().toString().c_str());
    Serial.print(" RSSI: ");
    Serial.println(advertisedDevice->getRSSI());

    parent->foundDeviceRivian = new NimBLEAdvertisedDevice(*advertisedDevice);
  } else if (advertisedDevice->isAdvertisingService(NimBLEUUID(SERVICE_UUID_TESLA))) {
    Serial.print("Found Tesla Device: ");
    Serial.print(advertisedDevice->getAddress().toString().c_str());
    Serial.print(" RSSI: ");
    Serial.println(advertisedDevice->getRSSI());

    parent->foundDeviceTesla = new NimBLEAdvertisedDevice(*advertisedDevice);
  }
}

UtilBleScanner::UtilBleScanner() : bleScanTimeSeconds(10), foundDeviceRivian(nullptr), foundDeviceTesla(nullptr) {
}

bool UtilBleScanner::findDevices() {
  if (foundDeviceRivian == nullptr && foundDeviceTesla == nullptr) {
    if (pScan->isScanning())
      return false;

    Serial.println("Scanning for devices with Rivian and Tesla service UUIDs ...");
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
