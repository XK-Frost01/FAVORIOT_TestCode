#include <WiFi.h>
#include <WiFiUdp.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
WiFiUDP udp;
const uint16_t port = 1010;

void connectToWiFi(){
  Serial.println("Connecting to ssid " + String(ssid) + "...");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("...");
    delay(250);
  }
  Serial.println("");
  Serial.println("Connected!");
}
void setup(){
  Serial.begin(115200);
  connectToWiFi();
  Serial.println(WiFI.localIP());
  udp.begin(port);
}

void loop(){
  int packetsize = udp.parsePacket();
  if (packetSize){
    char buffer[255];
    int len = udp.read(buffer, 255);
    buffer[len] = '\0';
    Serial.println("Received: " + String(buffer));
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.print("Echo: " + String(buffer));
    udp.endPacket();
  }
}