#include <WiFi.h>                   // Untuk WiFi connection
#include <HTTPClient.h>             // untuk hantar dan terima data guna http protocol
#include <NetworkClientSecure.h>    // Untuk secure data yang dihantar 
#include "FavoriotCA.h"             // cerification authentication untuk security (hhtps)
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
 
const char ssid[]= "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperID[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char Apikey[] = "YOUR_API_KEY";

String ReceiveResponse;
long previousMillis = 0;

void connectToWiFi();
void SendData();

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("...");
    delay (250);
  }
  Serial.println(" connected!");
}

void setup(){
  Serial.begin (115200);
  if (!bme.begin()){
    Serial.println("Not detecting the BME280 sensor");
  }    
  connectToWiFi();
} 

void loop(){
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  SendData();
  delay (250);
}

void SendData(){
  if(millis() - previousMillis > 15000){
    previousMillis = millis();

    Serial.println("Relative Humidity: "+String(bme.readHumidity())+" %RH");
    Serial.println("Approx. Altitude: "+String(bme.readAltitude(1013.25))+" m");
    Serial.println("Barometric Pressure: "+String(bme.readPressure())+" Pa");
    Serial.println("Ambient Temperature: "+String(bme.readTemperature())+" °C");

    String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{";
          json += "\"humidity\":\"" + String(bme.readHumidity()) + "\",";
          json += "\"altitude\":\"" + String(bme.readAltitude(1013.25)) + "\",";
          json += "\"barometer\":\"" + String(bme.readPressure()/100.00) + "\",";
          json += "\"temperature\":\"" + String(bme.readTemperature()) + "\",";
          json += "}}";

    Serial.println("\nSending data to Favoriot's Data Stream ...");
    NetworkClientSecure *client = new NetworkClientSecure;
    if(client) {
      client -> setCACert(FavoriotCA);
      HTTPClient https;
      https.begin("https://apiv2.favoriot.com/v2/streams");
      https.addHeader("Content-Type", "application/json");
      https.addHeader("Apikey", Apikey);
        
      int httpCode = https.POST(json);
      if(httpCode > 0){
        Serial.print("HTTPS Request: ");
        httpCode == 201 ? Serial.print("Success, ") : Serial.print("Error, ");
        Serial.println(https.getString());
      } else{
        Serial.println("> HTTPS Request Connection Error!");
      }
      https.end();
      delete client;
    } else{
      Serial.println("Unable to create secure client connection!");
    }
  }
}