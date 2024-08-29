#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLEAdvertisedDevice.h>

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"

BLEScan* pBLEScan;
BLEAdvertisedDevice* myDevice = nullptr;
bool deviceFound = false;

// Define the callback class before setup
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());
      
      // Check if this is the correct slave device
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
        Serial.println("Slave device found!");
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        deviceFound = true;
      }
    }
};

void setup() {
  Serial.begin(115200);
  
  // Initialize BLE
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  
  // Set the callback class for scanning
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
}

void loop() {
  if (deviceFound && myDevice) {
    // Connect to the slave device
    BLEClient* pClient = BLEDevice::createClient();
    Serial.println("Attempting to connect to the slave...");
    if (pClient->connect(myDevice)) {
      Serial.println("Connected to the slave");

      // Get the service and characteristic
      BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
      if (pRemoteService == nullptr) {
        Serial.println("Failed to find our service UUID");
        pClient->disconnect();
        return;
      }
      BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
      if (pRemoteCharacteristic == nullptr) {
        Serial.println("Failed to find our characteristic UUID");
        pClient->disconnect();
        return;
      }

      // Send heartbeat signal to the slave
      std::string heartbeatSignal = "HEARTBEAT";
      pRemoteCharacteristic->writeValue(heartbeatSignal);
      Serial.println("Sent heartbeat signal to slave");

      // Read the response
      std::string response = pRemoteCharacteristic->readValue();
      Serial.print("Received response from slave: ");
      Serial.println(response.c_str());

      // Disconnect
      pClient->disconnect();
      Serial.println("Disconnected from the slave");
    } else {
      Serial.println("Failed to connect to the slave");
    }

    // Clear the device so we can scan again
    myDevice = nullptr;
    deviceFound = false;
  } else {
    Serial.println("Scanning for slave devices...");
    pBLEScan->start(5); // Scan for 5 seconds
  }
  delay(2000); // Increase delay to stabilize BLE communication
}
