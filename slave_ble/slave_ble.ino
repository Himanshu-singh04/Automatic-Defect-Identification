#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic;
bool deviceConnected = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize BLE
  BLEDevice::init("Slave_ESP32");
  pServer = BLEDevice::createServer();
  
  // Create a BLE service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Create a BLE characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ | 
                      BLECharacteristic::PROPERTY_WRITE
                    );
  
  // Start the service
  pService->start();
  
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);  
  pAdvertising->setMaxPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection...");
}

void loop() {
  // Check if a device is connected
  if (deviceConnected) {
    // Respond to heartbeat from master
    std::string heartbeatResponse = "OK";
    pCharacteristic->setValue(heartbeatResponse);
    Serial.println("Sent response to master: OK");
    delay(1000);
  }
}
