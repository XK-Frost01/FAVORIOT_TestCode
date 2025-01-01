#include <WiFi.h>
#include <MQTT.h>
#include <NetworkClientSecure.h>
#include "tones.h"
#include "FavoriotCA_MQTT.h"

const char ssid[]= "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperID[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char apikey[] = "YOUR_API_KEY";
const char publishTopic[] = "/v2/streams";
const char statusTopic[] = "/v2/streams/status";
const char rpcTopic[] = "/v2/rpc";  // To Get data from dashboard
const char* clientID = "@XK-Esp32-BUZZER";

long previousMillis = 0;
String buzzer,ReceiveResponse;
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
  connectToWiFi();
  connectToFavoriotMQTT();
}

void loop() {
  // Check Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  // Check MQTT connection
  if (!mqtt.connected()) {
    connectToFavoriotMQTT();
  }
  mqtt.loop();
  delay(250);
}

void GetData(String &topic, String &ReceiveResponse) {
  Serial.println("Message received on topic: " + topic);
  Serial.println("Payload: " + ReceiveResponse);

  if (ReceiveResponse.indexOf("\BUZZER\":\"ON\"") >= 0){
    buzzer = "ON";
    Serial.println("Buzzzer turned ON");
  } else if (ReceiveResponse.indexOf("\"BUZZER\":\"OFF\"") >= 0){
    buzzer = "OFF";
    Serial.println("Buzzer turned OFF");
  }

  if (buzzer == "ON"){
    if (ReceiveResponse.indexOf("\"SONG_1\":\"ON\"") >= 0){
      song1();
    } else if (ReceiveResponse.indexOf("\"SONG_2\":\"ON\"") >= 0){
      song2();
    }
  }
}

void song1(){
  tone(13, NOTE_D4);  // buzzer will sound according to NOTE_D4.
  delay(500);
  noTone(13);  // buzzer has no sound since PWM signal is 0.
  delay(500);
}
void song2(){
  tone(13, NOTE_D5);  // buzzer will sound according to NOTE_D5.
  delay(500);
  noTone(13);  // buzzer has no sound since PWM signal is 0.
  delay(500);
}