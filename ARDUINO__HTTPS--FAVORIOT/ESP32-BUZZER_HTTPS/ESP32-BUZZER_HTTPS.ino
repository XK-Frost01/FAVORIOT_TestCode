#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h> // to add secure in http protocol ~ become https
#include "FavoriotCA.h"          // Certificate Authentication... use to secure data entering favoriot platform
#include "tones.h"               // buzzer

const char ssid[]= "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperID[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char apikey[] = "YOUR_API_KEY";

String ReceiveResponse, buzzer;

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("...");
    delay (250);
  }
  Serial.println(" connected!");
}

void GetData() {
  NetworkClientSecure *client = new NetworkClientSecure;
  if (client) {
    client->setCACert(FavoriotCA);
    HTTPClient https;
    String url = "https://apiv2.favoriot.com/v2/rpc?device_developer_id=" + String(deviceDeveloperID) + "&timeout=5000";
    https.begin(*client, url);
    https.addHeader("Content-Type", "application/json");
    https.addHeader("Apikey",apikey);
    int ResponseCode = https.GET();
    if (ResponseCode > 0) {
      ReceiveResponse = https.getString();
      Serial.println("Receive Payload: " + String(ReceiveResponse));
      Serial.println("");
      messageReceived();
    } else {
     Serial.println("Error on HTTPS GET request");
    }
    https.end();
    delete client; // Properly delete the client to free resources
  } else {
    Serial.println("Unable to create secure client connection");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2,HIGH);
  rgb.begin(); 
  connectToWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  GetData();
  delay(250);
}

void messageReceived() {
    // Based on data received...
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