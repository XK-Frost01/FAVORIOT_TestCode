#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h> // to add secure in http protocol ~ become https
#include "FavoriotCA.h"          // Certificate Authentication... use to secure data entering favoriot platform
#include <Adafruit_NeoPixel.h>   // RGB 

const char ssid[]= "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperID[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char apikey[] = "YOUR_API_KEY";

Adafruit_NeoPixel rgb(1,16);
String ReceiveResponse, State;
int r = 0, gr = 0, b = 0;

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
  if (ReceiveResponse.indexOf("\"RGB\":\"ON\"") >= 0) {
    State = "ON";
    Serial.println("RGB turned ON");
  } else if(ReceiveResponse.indexOf("\"RGB\":\"OFF\"") >= 0){
    State = "OFF";
    Serial.println("RGB turned OFF");
  }

  if(State == "ON"){
    if (ReceiveResponse.indexOf("\"RGB_Red\":") != -1) {
       r = extractValue("\"RGB_Red\":");
    } else if (ReceiveResponse.indexOf("\"RGB_Green\":") != -1) { 
       gr = extractValue("\"RGB_Green\":"); // corrected the key
    } else if (ReceiveResponse.indexOf("\"RGB_Blue\":") != -1) { 
       b = extractValue("\"RGB_Blue\":"); // corrected the key
    }
    Serial.println("");
    Serial.println("Extracted Values:");
    Serial.println("Red: " + String(r));
    Serial.println("Green: " + String(gr));
    Serial.println("Blue: " + String(b));
    rgb.setPixelColor(0, r,gr,b);
  }
  else {
    rgb.setPixelColor(0,0,0,0);
  }
  rgb.show();
}

int extractValue(String Slider) {
  int startIndex = ReceiveResponse.indexOf(Slider) + Slider.length();
  int endIndex = ReceiveResponse.indexOf(",",startIndex);
  if (endIndex == -1) {
    endIndex = ReceiveResponse.indexOf("}", startIndex);
  }
  String SliderValue = ReceiveResponse.substring(startIndex, endIndex);
  SliderValue.trim(); // Trim whitespace in place
  SliderValue.replace("\"", ""); // Remove any quotes
  return SliderValue.toInt();
}