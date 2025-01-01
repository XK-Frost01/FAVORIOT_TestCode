#include <WiFi.h>
#include <NetworkClientSecure.h>
#include <Adafruit_APDS9960.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char* deviceDeveloperID = "YOUR_DEVICE_DEVELOPER_ID";
const char*  serverIP= "YOUR_SERVER_IP_ADDRESS";
const uint16_t serverPORT = 9999; 

Adafruit_APDS9960 apds;

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
  if (!apds.begin()){
    Serial.println("Not detecting APDS9960 sensor");
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
      Serial.println("Proximity: " + String(apds.readProximity()));
      String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{"; 
            json += "\"proximity\":\"" + String(apds.readProximity()) + "\",";
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