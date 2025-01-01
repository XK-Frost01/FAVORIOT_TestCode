#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h> // to add secure in http protocol ~ become https
#include "FavoriotCA.h"          // Certificate Authentication... use to secure data entering favoriot platform

const char ssid[]= "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperID[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char apikey[] = "YOUR_API_KEY";

String ReceiveResponse;

void connectToWiFI();
void GetData();
void messageReceived();

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("...");
    delay (250);
  }
  Serial.println(" connected!");
}

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2,HIGH);
  connectToWiFi();
}
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  GetData();
  delay(250);
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
        Serial.println("HTTPS Response Code: " + String(ResponseCode));
        Serial.println("Receive Payload: " + String(ReceiveResponse));
        messageReceived();
      } 
      else {
        Serial.println("Error on HTTPS GET request");
      }
      https.end();
    delete client; // Properly delete the client to free resources
  } else {
    Serial.println("Unable to create secure client connection");
  }
}

void messageReceived() {
  if (ReceiveResponse.indexOf("\"LED_ON\":\"ON\"") >= 0) {
    digitalWrite(2, LOW);
    Serial.println("LED turned ON");
  } else if(ReceiveResponse.indexOf("\"LED_OFF\":\"OFF\"") >= 0){
    digitalWrite(2, HIGH);
    Serial.println("LED turned OFF");
  }
}