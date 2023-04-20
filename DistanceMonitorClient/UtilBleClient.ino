#include "UtilBleClient.h"
#include <M5StickCPlus.h>

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

UtilBleClient::UtilBleClient(const std::string& macAddress) : deviceMacAddress(macAddress) {
  lastReadFloatValue = InvalidFixedPointValue;
}

std::string UtilBleClient::getDeviceName() {
  if (deviceMacAddress == MacAddressTesla) {
    return "Tesla Client";
  } else if (deviceMacAddress == MacAddressRivian) {
    return "Rivian Client";
  } else {
    return "Unknown Client";
  }
}

bool UtilBleClient::connectToDeviceIfNeeded() {
  if (pClient != nullptr && pClient->isConnected())
    return true;

  //Serial.print("BLE Trying to Connect to "); Serial.println(deviceMacAddress.c_str());
  pClient = NimBLEDevice::createClient(NimBLEAddress(deviceMacAddress));

  pClient->setConnectTimeout(1);

  if (!pClient->connect(NimBLEAddress(deviceMacAddress))) {
    //Serial.println("BLE Failed to connect! ");
    return false;
  }

  pRemoteService = pClient->getService(NimBLEUUID(SERVICE_UUID));
  if (pRemoteService == nullptr) {
    //Serial.println("Failed to find the service!");
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID(CHARACTERISTIC_UUID));
  if (pRemoteCharacteristic == nullptr) {
    //Serial.println("Failed to find the characteristic!");
    pClient->disconnect();
    return false;
  }

  Serial.print("BLE Connected "); Serial.println(deviceMacAddress.c_str());

  return true;
}

static bool _isBleInit = false;

void UtilBleClient::begin() {
  if( !_isBleInit ) {
    NimBLEDevice::init("");
    _isBleInit = true;
  }

  connectToDeviceIfNeeded();
}

float UtilBleClient::readFloatValue() {
  if (!connectToDeviceIfNeeded()) {
    lastReadFloatValue = InvalidFixedPointValue;
    return InvalidFixedPointValue;
  }

  bool canRead = pRemoteCharacteristic->canRead();
  if (pRemoteCharacteristic == nullptr || !canRead) {
    lastReadFloatValue = InvalidFixedPointValue;
    Serial.println( canRead ? "Characteristic Invalid" : "Characteristic can't be read");
    return InvalidFixedPointValue;
  }

  std::string value = pRemoteCharacteristic->readValue();
  uint8_t* pData = (uint8_t*)value.data();
  size_t length = value.length();

  if (length != BufferSize) {
    lastReadFloatValue = InvalidFixedPointValue;
    Serial.println("Characteristic buffer size doesn't match expected size");
    return InvalidFixedPointValue;
  }

  float floatValue = bleReadFloatFromFixed16x8(pData);
  lastReadFloatValue = floatValue;
  
  return floatValue;
}

float UtilBleClient::bleReadFloatFromFixed16x8(uint8_t *byteArray) {
  // Verify the start marker
  if (byteArray[MarkerStartIndex] != BleBufferMarkerStart) {
    return InvalidFixedPointValue;
  }

  // This is a bit of a hack right now as I don't want to debug what's going on with signed floats and this doesn't really support
  // negactive distances anyway...
  //
  if( byteArray[HighByteIndex] == 0xFF && byteArray[MiddleByteIndex] == 0xFF && byteArray[LowByteIndex] == 0xFF)
    return InvalidFixedPointValue;

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
