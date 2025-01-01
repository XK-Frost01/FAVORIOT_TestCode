#include <WiFi.h>
#include <MQTT.h>
#include <NetworkClientSecure.h>
#include <Adafruit_NeoPixel.h>
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
const char* clientID = "@XK-Esp32-SUBSCRIBE";

Adafruit_NeoPixel rgb(1, 16);
long previousMillis = 0;
String State,ReceiveResponse, buzzer;
int r = 0, gr = 0, b = 0;
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
  digitalWrite(2, HIGH);
  rgb.begin();
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
  // Handle the data or command from the topic
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

  if (ReceiveResponse.indexOf("\"LED\":\"ON\"") >= 0) {
    digitalWrite(2, LOW);
    Serial.println("LED turned ON");
  } else if(ReceiveResponse.indexOf("\"LED\":\"OFF\"") >= 0){
    digitalWrite(2, HIGH);
    Serial.println("LED turned OFF");
  }

  if (ReceiveResponse.indexOf("\"RGB\":\"ON\"") >= 0) {
    State = "ON";
    Serial.println("RGB turned ON");
  } else if(ReceiveResponse.indexOf("\"RGB\":\"OFF\"") >= 0){
    State = "OFF";
    Serial.println("RGB turned OFF");
  }
  
  if (ReceiveResponse.indexOf("\"RGB_Red\":") != -1) {
    r = extractValue(ReceiveResponse, "\"RGB_Red\":");
  } else if (ReceiveResponse.indexOf("\"RGB_Green\":") != -1) { 
    gr = extractValue(ReceiveResponse, "\"RGB_Green\":"); 
  } else if (ReceiveResponse.indexOf("\"RGB_Blue\":") != -1) { 
    b = extractValue(ReceiveResponse, "\"RGB_Blue\":"); 
  }
    Serial.println("Extracted Values:");
    Serial.println("Red: " + String(r));
    Serial.println("Green: " + String(gr));
    Serial.println("Blue: " + String(b));
  
  if(State == "ON"){
    rgb.setPixelColor(0, r, gr, b);  // Update RGB color
  } else {
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