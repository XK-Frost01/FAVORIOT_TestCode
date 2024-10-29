#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <Adafruit_NeoPixel.h>      // Untuk RGB 
#include "FavoriotCA.h"

Adafruit_NeoPixel rgb(1, 16); 
String ReceiveResponse, State;
int r =  0, g = 0, b = 0;

const char ssid[] = "favoriot@unifi";
const char password[] = "fav0r10t2017";
//const char ssid[] = "derictanhk18@unifi";
//const char password[] = "A287@SFERA";
const char deviceDeveloperID[] = "Hibiscus_Sense@nuraizatulsyakila01";
const char deviceAccessToken[] = "eH9cGiG974pyWOnDKcm7TRjDz8vEIP3G";
const char Apikey[] = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6Im51cmFpemF0dWxzeWFraWxhMDEiLCJyZWFkX3dyaXRlIjp0cnVlLCJpYXQiOjE3Mjg4NzUwMjl9.X8Iw7N1002rj8wt0FbwP9KVrLR1llOAXr782JSPg1CQ";

void setup() {
  Serial.begin(115200);
  rgb.begin();
  // Initialize Wi-Fi connectivity
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(250);
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  GetData();
  delay(2000);
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
      https.addHeader("Apikey",String(Apikey));

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

void messageReceived() {
  if (ReceiveResponse.indexOf("\"RGB\":\"ON\"") >= 0) {
    State = "ON";
    Serial.println("RGB turned ON");
  } 
  else if(ReceiveResponse.indexOf("\"RGB\":\"OFF\"") >= 0){
    State = "OFF";

    Serial.println("RGB turned OFF");
  }
  if(State == "ON"){
    if (ReceiveResponse.indexOf("\"RGB_Red\":") != -1) {
      r = extractValue("\"RGB_Red\":");
    }
    if (ReceiveResponse.indexOf("\"RGB_Green\":") != -1) { 
      g = extractValue("\"RGB_Green\":"); // corrected the key
    }
    if (ReceiveResponse.indexOf("\"RGB_Blue\":") != -1) { 
      b = extractValue("\"RGB_Blue\":"); // corrected the key
    }
    Serial.println("Extracted Values:");
    Serial.println("Red: " + String(r));
    Serial.println("Green: " + String(g));
    Serial.println("Blue: " + String(b));
    rgb.setPixelColor(0, r,g,b);
  }
  // else if (ReceiveResponse.indexOf("\"RGB\"") == -1 && State == "ON"){
  else {
    rgb.setPixelColor(0,0,0,0);
  }
    rgb.show();
}

int extractValue(String Slider) {
  int startIndex = ReceiveResponse.indexOf(Slider) + Slider.length();
  int endIndex = ReceiveResponse.indexOf(",", startIndex);
  if (endIndex == -1) {
    endIndex = ReceiveResponse.indexOf("}", startIndex);
  }
  
  // Extract the value, trimming spaces and quotes
  String valueSlider = ReceiveResponse.substring(startIndex, endIndex);
  valueSlider.trim(); // Remove any extra spaces
  valueSlider.replace("\"", ""); // Remove any quotes
  
  return valueSlider.toInt(); // Convert to integer
}