#include <WiFi.h>
#include <ModbusIP_ESP.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
IPAddress slaveIP(192, 168, 0, 43);
const uint16_t slavePORT = 10502;
ModbusIP modbus;

void connectToWiFi(){
  Serial.print("Connecting to ssid " + String(ssid) + "...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected");
}

void setup(){
  Serial.begin(115200);
  connectToWiFi();
  Serial.println(WiFi.localIP());
  modbus.client();
  if (modbus.connect(slaveIP, slavePORT)){
    Serial.println("Connected to Modbus Slave");
  }
  else{
    Serial.println("Failed to connect to Modbus Slave");
  }
}

void loop(){
  uint16_t registerValue = 1234;
  uint16_t registerAddress = 0;

  if (modbus.writeHreg(slaveIP, registerAddress, registerValue)){
    Serial.print("Successfully wrote value ");
    Serial.print(registerValue);
    Serial.print(" to register address ");
    Serial.println(registerAddress);
  }
  else {
    Serial.println("Failed to write register");
  }

  delay (1000);
}