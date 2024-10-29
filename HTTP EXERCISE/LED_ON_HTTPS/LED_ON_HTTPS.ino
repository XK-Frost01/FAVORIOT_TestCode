#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h> // to add secure in http protocol ~ become https
#include "FavoriotCA.h"          // Certificate Authentication... use to secure data entering favoriot platform

//Set WiFi
const char ssid[] = "favoriot@unifi";
const char password[] = "fav0r10t2017";
//const char ssid[] = "derictanhk18@unifi";
//const char password[] = "A287@SFERA";
const char deviceDeveloperID[] = "Hibiscus_Sense@nuraizatulsyakila01";
const char deviceAccessToken[] = "eH9cGiG974pyWOnDKcm7TRjDz8vEIP3G";
const char apikey[] = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6Im51cmFpemF0dWxzeWFraWxhMDEiLCJyZWFkX3dyaXRlIjp0cnVlLCJpYXQiOjE3Mjg4NzUwMjl9.X8Iw7N1002rj8wt0FbwP9KVrLR1llOAXr782JSPg1CQ";

String ReceiveResponse;

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");
  // Initialize Wi-Fi connectivity
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
    {
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
    }
    delete client; // Properly delete the client to free resources
  } 
  else {
    Serial.println("Unable to create secure client connection");
  }
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

void messageReceived() {
  if (ReceiveResponse.indexOf("\"LED_ON\":\"ON\"") >= 0) {
    digitalWrite(2, LOW);
    Serial.println("LED turned ON");
  } 
  else if(ReceiveResponse.indexOf("\"LED_OFF\":\"OFF\"") >= 0){
    digitalWrite(2, HIGH);
    Serial.println("LED turned OFF");
  }
}
