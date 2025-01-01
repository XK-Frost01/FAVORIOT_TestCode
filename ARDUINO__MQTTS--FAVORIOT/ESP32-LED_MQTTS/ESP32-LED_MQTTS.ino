#include <WiFi.h>
#include <MQTT.h>
#include <NetworkClientSecure.h>
#include "FavoriotCA_MQTT.h"

const char ssid[]= "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperID[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char publishTopic[] = "/v2/streams";
const char statusTopic[] = "/v2/streams/status";
const char rpcTopic[] = "/v2/rpc";  // To Get data from dashboard
const char* clientID = "@XK-Esp32-LED";

long previousMillis = 0;
NetworkClientSecure client;
MQTTClient mqtt(4096);

void connectToWiFi();
void connectToFavoriotMQTT();
void GetData();

void connectToWiFi() {        // for easier calling in the loop
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("...");
    delay (250);
  }
  Serial.println(" connected!");
}

void connectToFavoriotMQTT() {    
  Serial.print("Connecting to Favoriot MQTT ...");
  client.setCACert(FavoriotCA_MQTT);
  mqtt.begin("mqtt.favoriot.com", 8883, client);
  mqtt.onMessage(GetData);
  
  // to create the client id can be a random configuration but its better 
  // to custom it for better organization
  // like right now im already customize it in the universal line (const char*)
  // String uniqueString = String(ssid) + "-" + String(random(1, 98)) + String(random(99, 999));
  // char ClientID[uniqueString.length() + 1];
  // uniqueString.toCharArray(ClientID, uniqueString.length() + 1);
  while (!mqtt.connect(clientID, deviceAccessToken, deviceAccessToken)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println(" connected!");
  Serial.println("Subscribe to: " + String(deviceAccessToken) + String(rpcTopic));
  mqtt.subscribe(String(deviceAccessToken) + String(rpcTopic));
  Serial.println();
}

void setup(){
  Serial.begin (115200);
  pinMode(2,OUTPUT); 
  digitalWrite(2,HIGH);
  connectToWiFi();
  connectToFavoriotMQTT();
}
void loop() {
  // Check Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  // Ensure MQTT connection remains active
  if (!mqtt.connected()) {
    connectToFavoriotMQTT();
  }
  // Listen for incoming messages
  mqtt.loop();
  delay(2000);
}

void GetData(String &topic, String &ReceiveResponse) {
  Serial.println("Message received on topic: " + topic);
  Serial.println("Payload: " + ReceiveResponse);
  if (ReceiveResponse.indexOf("\"LED\":\"ON\"") >= 0) {
    digitalWrite(2, LOW);
    Serial.println("LED turned ON");
  } 
  else if(ReceiveResponse.indexOf("\"LED\":\"OFF\"") >= 0){
    digitalWrite(2, HIGH);
    Serial.println("LED turned OFF");
  }
}