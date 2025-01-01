#include <WiFi.h>
#include <NetworkClientSecure.h>
#include <Adafruit_BME280.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char* deviceDeveloperID = "YOUR_DEVICE_DEVELOPER_ID";
const char*  serverIP= "YOUR_SERVER_IP_ADDRESS";
const uint16_t serverPORT = 9999; 

Adafruit_BME280 bme;

long previousMillis = 0;
WiFiClient client;

void connectToWiFi(){
  Serial.print("Connecting to ssid " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    Serial.println("...");
    delay(250);
  }
  Serial.println("Connected!");
}

void setup(){
  Serial.begin(115200);
  if (!bme.begin()){
    Serial.println("Not detecting the Hibiscus Sense sensor");
  }   
  apds.enableProximity(true); 
  connectToWiFi();
  Serial.println(WiFi.localIP());
  serverConnect();
}

void loop(){
  while (client.available()){
    String message = client.readStringUntil('\n');
    Serial.println("Message from Node-RED: " + message); // Print it
  }
  client.stop(); 
  if (WiFi.status() != WL_CONNECTED){
    connectToWiFi();
  }
  serverConnect();
  delay (3000);
}

void serverConnect(){
  //establish connection in between the client and server 
  if (client.connect(serverIP, serverPORT)){
    Serial.println("Connected to Node-RED");  
    // Interval 3 second
    if(millis() - previousMillis > 3000){
      previousMillis = millis();
      Serial.println("Relative Humidity: " + String(bme.readHumidity()) + " %RH");
      Serial.println("Approx. Altitude: "+String(bme.readAltitude(1013.25))+" m");
      Serial.println("Barometric Pressure: "+String(bme.readPressure())+" Pa");
      Serial.println("Ambient Temperature: "+String(bme.readTemperature())+" °C");

      String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{"; 
            json += "\"humidity\":\"" + String(bme.readHumidity()) + "\",";
            json += "\"altitude\":\"" + String(bme.readAltitude(1013.25)) + "\",";
            json += "\"barometer\":\"" + String(bme.readPressure()/100.00) + "\",";
            json += "\"temperature\":\"" + String(bme.readTemperature()) + "\",";
            json += "}}"; 
      if (json.length() > 0) {
        client.print(json);   // Send data to Node-RED
        Serial.println("data sent to Node-RED: " + json);
      } else {
        Serial.println("data is empty, not sending.");
      } 
    }
  } else {
    Serial.println("Failed to connect to Node-RED");
  }
}