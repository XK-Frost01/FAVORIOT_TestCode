#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_MPU6050.h>

BLEServer *pServer;
BLECharacteristic *pCharacteristic;
Adafruit_MPU6050 mpu;
sensors_event_t  a, g, temp;

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
      mpu.getEvent(&a,&g,&temp);
      Serial.println("Acceleration X:" + String(a.acceleration.x) + ", Y:"+String(a.acceleration.y) + ", Z:"+String(a.acceleration.z) + " m/s^2");
      Serial.println("Rotation X:" + String(g.gyro.x) + ", Y:" + String(g.gyro.y) + ", Z:" + String(g.gyro.z) + " rad/s");
      String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{";
            json += "\"accx\":\"" + String(a.acceleration.x) + "\",";
            json += "\"accy\":\"" + String(a.acceleration.y) + "\",";
            json += "\"accz\":\"" + String(a.acceleration.z) + "\",";
            json += "\"gyrox\":\"" + String(g.gyro.x) + "\",";
            json += "\"gyroy\":\"" + String(g.gyro.y) + "\",";
            json += "\"gyroz\":\"" + String(g.gyro.z) + "\"";
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