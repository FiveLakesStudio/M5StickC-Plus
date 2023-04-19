#pragma once

#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>

#define MacAddressRivian "e8:9f:6d:08:d4:f6"
#define MacAddressTesla "4c:75:25:cd:ef:d2"

class UtilBleClient {
  private:
    NimBLEClient* pClient;
    NimBLERemoteService* pRemoteService;
    NimBLERemoteCharacteristic* pRemoteCharacteristic;
    std::string deviceMacAddress;

    bool connectToDeviceIfNeeded();

  public:
    std::string getDeviceName(); // Add this method declaration
    float lastReadFloatValue;

    UtilBleClient(const std::string& macAddress);
    void begin();
    float readFloatValue();
    float bleReadFloatFromFixed16x8(uint8_t *byteArray);
};
