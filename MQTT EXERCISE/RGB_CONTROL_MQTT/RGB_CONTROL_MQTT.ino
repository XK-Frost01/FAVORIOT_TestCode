#include <WiFi.h>
#include <MQTT.h>
#include <NetworkClientSecure.h>
#include <Adafruit_NeoPixel.h>
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
const char* clientID = "Syakila01-Esp32-RGB";

long previousMillis = 0;
String State,ReceiveResponse;
int r = 0, gr = 0, b = 0;
Adafruit_NeoPixel rgb(1,16);
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
  rgb.begin();
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
  if (ReceiveResponse.indexOf("\"RGB\":\"ON\"") >= 0) {
    State = "ON";
    Serial.println("RGB turned ON");
  } 
  else if(ReceiveResponse.indexOf("\"RGB\":\"OFF\"") >= 0){
    State = "OFF";
    Serial.println("RGB turned OFF");
  }
  else if (ReceiveResponse.indexOf("\"RGB_Red\":") != -1) {
    r = extractValue(ReceiveResponse, "\"RGB_Red\":");
  }
  else if (ReceiveResponse.indexOf("\"RGB_Green\":") != -1) { 
    gr = extractValue(ReceiveResponse, "\"RGB_Green\":"); 
  }
  else if (ReceiveResponse.indexOf("\"RGB_Blue\":") != -1) { 
    b = extractValue(ReceiveResponse, "\"RGB_Blue\":"); 
  }
    Serial.println("Extracted Values:");
    Serial.println("Red: " + String(r));
    Serial.println("Green: " + String(gr));
    Serial.println("Blue: " + String(b));
  
  if(State == "ON"){
    rgb.setPixelColor(0, r, gr, b);  // Update RGB color
  }
  else {
    rgb.setPixelColor(0, 0, 0, 0);  // Turn off RGB when OFF
  }
  rgb.show();  // Apply changes
}

int extractValue(String payload, String key) {
  int startIndex = payload.indexOf(key) + key.length();
  int endIndex = payload.indexOf(",", startIndex);
  if (endIndex == -1) {
    endIndex = payload.indexOf("}", startIndex);  // Handle last item case
  }
  String value = payload.substring(startIndex, endIndex);
  value.trim(); 
  value.replace("\"", ""); 
  Serial.println("Extracted value for " + key + ": " + value);  // Debug value extraction

  return value.toInt(); 
}