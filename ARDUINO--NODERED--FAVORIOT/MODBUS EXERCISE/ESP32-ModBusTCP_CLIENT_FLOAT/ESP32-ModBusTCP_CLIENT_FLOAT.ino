#include <WiFi.h>
#include <ModbusIP_ESP.h>
#include <Adafruit_APDS9960.h>  // Include the APDS9960 library for proximity
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
IPAddress slaveIP(192, 168, 0, 43);  // Modify to your Modbus slave IP
const uint16_t slavePORT = 10502;

ModbusIP modbus;
Adafruit_APDS9960 apds;
Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;

float sensorValue[11];  // Store sensor values
// save the value total value inside uint32_t
uint32_t registerValue[11];
// save the upper 4 value of data
uint16_t frontValue[11];
// save the lower 4 value of data
uint16_t pointer[11];
sensors_event_t a, g, temp;
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
  // establish the connection for the modbus client-server
  if (modbus.connect(slaveIP, slavePORT)) {
    Serial.println("Connected to Modbus Slave");
  } else {
    Serial.println("Failed to connect to Modbus Slave");
  }
}

void loop() {
  if(millis() - previousMillis > 3000){
    previousMillis = millis();
    // Collect sensor values using float value
    sensorValue[0] = apds.readProximity();    // Read proximity value
    sensorValue[1] = bme.readHumidity();
    sensorValue[2] = bme.readAltitude(1013.25);
    sensorValue[3] = bme.readPressure();
    sensorValue[4] = bme.readTemperature();
    mpu.getEvent(&a, &g, &temp);
    sensorValue[5] = a.acceleration.x;
    sensorValue[6] = a.acceleration.y;
    sensorValue[7] = a.acceleration.z;
    sensorValue[8] = g.gyro.x;
    sensorValue[9] = g.gyro.y;
    sensorValue[10] = g.gyro.z;

    for (uint16_t i=0; i<11; i++){
      // enter the float data the 32uint by changing the data to int value
      registerValue[i] = sensorValue[i] * 100;
      Serial.println("register value: " + String(registerValue[i]) +", Sensor value: " + String(sensorValue[i]));
      //save the upper byte which 16uint value
      frontValue[i] = (registerValue[i] >> 16) & 0xFFFF;
      // save the lower byte which 16uint value
      pointer[i] = registerValue[i] & 0xFFFF;
      Serial.println("front value: "+String(frontValue[i])+", pointer value: "+String(pointer[i]));
      //write the upper value data inside the even address to send to modbus
      if (modbus.writeHreg(slaveIP, i * 2, frontValue[i])) {    // High word
        Serial.println("High Word sent to register " + String(i * 2) + ": " + String(frontValue[i]));
      } 
      else {
        Serial.println("Failed to send High Word to register " + String(i * 2));
      }
      // write the lower value data inside the odd address to send to modbus
      if (modbus.writeHreg(slaveIP, i * 2 + 1, pointer[i])) {  // Low word
        Serial.println("Low Word sent to register " + String(i * 2 + 1) + ": " + String(pointer[i]));
      }
      else {
        Serial.println("Failed to send Low Word to register " + String(i * 2 + 1));
      }
    }
  }
  // Reconnect Modbus if needed
  reconnectModbus();
}