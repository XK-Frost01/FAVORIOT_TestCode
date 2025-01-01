#include <WiFi.h>
#include <WifiUdp.h>
#include <NetworkClientSecure.h>
#include <Adafruit_BME280.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char*  serverIP= "YOUR_SERVER_IP_ADDRESS";
const char* deviceDeveloperID = "YOUR_DEVICE_DEVELOPER_ID";
const uint16_t serverPORT = 9090; //default port for nodered UDP

Adafruit_BME280 bme;

long previousMillis = 0;
WiFiUDP udp;

void connectToWiFi(){
  Serial.print("Connecting to ssid " + String(ssid) + "...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print("...");
    delay(250);
  }
  Serial.println("Connected!");
}
void setup(){
  Serial.begin(115200);
  if (!apds.begin() || !bme.begin() || !mpu.begin()){
    Serial.println("Not detecting the Hibiscus Sense sensor");
  }   
  apds.enableProximity(true); 
  connectToWiFi();
  Serial.println(WiFi.localIP());
  udp.begin(0);
}

void loop(){
  if(millis() - previousMillis > 3000){
    previousMillis = millis();
    Serial.println("Relative Humidity: " + String(bme.readHumidity()) + " %RH");
    Serial.println("Approx. Altitude: "+String(bme.readAltitude(1013.25))+" m");
    Serial.println("Barometric Pressure: "+String(bme.readPressure())+" Pa");
    Serial.println("Ambient Temperature: "+String(bme.readTemperature())+" °C");

    String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{"; 
          json += "\"humidity\":\"" + String(bme.readHumidity()) + "\",";
          json += "\"altitude\":\"" + String(bme.readAltitude(1013.25)) + "\",";
          json += "\"barometer\":\"" + String(bme.readPressure()/100.00) + "\",";
          json += "\"temperature\":\"" + String(bme.readTemperature()) + "\",";
          json += "}}";    

    udp.beginPacket(serverIP, serverPORT);
    udp.print(json);
    udp.endPacket();
    delay(3000);
  }
}