#ifndef UTIL_BLE_SCANNER_H
#define UTIL_BLE_SCANNER_H

#include <M5StickCPlus.h>
#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>
#include <vector>

class UtilBleScanner {
private:
  const int bleScanTimeSeconds;

  NimBLEScan* pScan;
  std::vector<NimBLEAdvertisedDevice*> foundDevices;

  class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    UtilBleScanner* parent;
  public:
    MyAdvertisedDeviceCallbacks(UtilBleScanner* pParent) : parent(pParent) {}
    void onResult(NimBLEAdvertisedDevice* advertisedDevice);
  };

public:
  UtilBleScanner();
  bool findDevices();
  void beginClient();
};

#endif // UTIL_BLE_SCANNER_H
