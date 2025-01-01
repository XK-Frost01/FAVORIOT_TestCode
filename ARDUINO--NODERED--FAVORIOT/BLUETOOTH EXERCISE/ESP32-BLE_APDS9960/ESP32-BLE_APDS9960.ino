#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_APDS9960.h>      

BLEServer *pServer;
BLECharacteristic *pCharacteristic;
Adafruit_APDS9960 apds;

// UUIDs for BLE service and characteristic
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

bool deviceConnected = false;  // Track connection status

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
  }
};

void setup() {
  Serial.begin(115200);

  // Initialize BLE
  BLEDevice::init("HibiscusESP32");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());  // Set the connection callbacks
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->setValue("0");
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
  
  // Initialize BME280 sensor
  if (!bme.begin()) {  // I2C address: 0x76 or 0x77
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);  // Stop execution
  }
}

void loop() {
  // Constantly check connection status
  if (deviceConnected) {
    // Read sensor data
    if(millis() - previousMillis > 15000){
      previousMillis = millis();
      Serial.println("Proximity: " + String(apds.readProximity()));
      String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{";
            json += "\"proximity\":\"" + String(apds.readProximity()) + "\",";
            json += "}}";
      // Update BLE characteristic
      pCharacteristic->setValue(json.c_str());
      pCharacteristic->notify();
      // Debug output
      Serial.println(json);
    }
  } else {
    // Optionally, print a message when no device is connected
    Serial.println("Waiting for device to connect...");
  }
  delay(10000);  // Adjust delay as needed (10 seconds)
}