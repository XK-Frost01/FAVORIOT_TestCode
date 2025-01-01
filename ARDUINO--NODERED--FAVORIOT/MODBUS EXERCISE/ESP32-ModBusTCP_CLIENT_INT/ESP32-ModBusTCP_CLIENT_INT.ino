#include <WiFi.h>               // for WiFi connection
#include <ModbusIP_ESP.h>       // for ESP modbus function
//hibiscus sense sensor
#include <Adafruit_APDS9960.h>  
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
IPAddress slaveIP(192, 168, 0, 43);  // Modify to your Modbus slave IP, same as nodered pi4 IP
const uint16_t slavePORT = 10502;    // nodered default PORT for modbus

ModbusIP modbus;
Adafruit_APDS9960 apds;
Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;

uint16_t sensorValue[11];  // Store sensor values
long previousMillis = 0;

void connectToWiFi() {
  Serial.print("Connecting to ssid " + String(ssid) + "...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected");
}

//recoonect the modbus connection
void reconnectModbus() {
  if (!modbus.isConnected(slaveIP)) {
    Serial.println("Reconnecting to Modbus...");
    if (modbus.connect(slaveIP, slavePORT)) {
      Serial.println("Reconnected to Modbus Slave");
    } else {
      Serial.println("Failed to reconnect to Modbus Slave");
    }
  }
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  Serial.println(WiFi.localIP());

  // Initialize sensors
  if (!apds.begin() || !bme.begin() || !mpu.begin()) {
    Serial.println("Failed to initialize Hibiscus Sense sensors");
  }
  apds.enableProximity(true);

  modbus.client();
  // establish connection with modbus nodered
  if (modbus.connect(slaveIP, slavePORT)) {
    Serial.println("Connected to Modbus Slave");
  } else {
    Serial.println("Failed to connect to Modbus Slave");
  }
}

void loop() {
  // Collect sensor values
  // every value inside the array
  sensorValue[0] = apds.readProximity();  // Read proximity value
  sensorValue[1] = bme.readHumidity();

  for (uint16_t i=0; i<2; i++){
    // writing the array value according to array sequence
    if (modbus.writeHreg(slaveIP, i, frontValue[i])) {  // High word
      Serial.println("Sensor Value " + String(i) + ": " + String(sensorValue[i]));
    } else {
      Serial.println("Failed to send sensor value to register " + String(i));
    }
  }
  // Reconnect Modbus if needed
  reconnectModbus();
  delay(1000);  // Wait for 1 second before sending again
}
