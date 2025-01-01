#include <WiFi.h>
#include <WifiUdp.h>
#include <NetworkClientSecure.h>
#include <Adafruit_APDS9960.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char*  serverIP= "YOUR_SERVER_IP_ADDRESS";
const char* deviceDeveloperID = "YOUR_DEVICE_DEVELOPER_ID";
const uint16_t serverPORT = 9090; //default port for nodered UDP

Adafruit_APDS9960 apds;

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
    Serial.println("Proximity: " + String(apds.readProximity()));
    String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{"; 
          json += "}}";    

    udp.beginPacket(serverIP, serverPORT);
    udp.print(json);
    udp.endPacket();
    delay(3000);
  }
}