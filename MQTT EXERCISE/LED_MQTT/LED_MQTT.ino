#include <WiFi.h>
#include <MQTT.h>
#include <NetworkClientSecure.h>
#include "FavoriotCA_MQTT.h"

const char ssid[] = "favoriot@unifi";
const char password[] = "fav0r10t2017";
// const char ssid[] = "derichktan18@unifi";
// const char password[] = "A287@SFERA";
const char deviceDeveloperID[] = "Hibiscus_Sense@nuraizatulsyakila01";
const char deviceAccessToken[] = "eH9cGiG974pyWOnDKcm7TRjDz8vEIP3G";
const char publishTopic[] = "/v2/streams";
const char statusTopic[] = "/v2/streams/status";
const char rpcTopic[] = "/v2/rpc";  // To Get data from dashboard
const char* clientID = "Syakila01-Esp32-LED";

long previousMillis = 0;
NetworkClientSecure client;
MQTTClient mqtt(4096);

void connectToWiFi() {        // for easier calling in the loop
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("...");
    delay (250);
  }
  Serial.println(" connected!");
}

// for easier calling in the loop
// if want to enter this line in the publish function also can... 
// but harder to call and it will constantly created and disconnected
void connectToFavoriotMQTT() {    
  Serial.print("Connecting to Favoriot MQTT ...");
  client.setCACert(FavoriotCA_MQTT);
  mqtt.begin("mqtt.favoriot.com", 8883, client);
  mqtt.onMessage(GetData);

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

// Handle incoming messages
void GetData(String &topic, String &ReceiveResponse) {
  Serial.println("Message received on topic: " + topic);
  Serial.println("Payload: " + ReceiveResponse);
// Handle the data or command from the topic
  if (ReceiveResponse.indexOf("\"LED\":\"ON\"") >= 0) {
    digitalWrite(2, LOW);
    Serial.println("LED turned ON");
  } 
  else if(ReceiveResponse.indexOf("\"LED\":\"OFF\"") >= 0){
    digitalWrite(2, HIGH);
    Serial.println("LED turned OFF");
  }
}