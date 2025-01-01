#include <WiFi.h>
#include <ArduinoJson.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char*  serverIP= "YOUR_SERVER_IP_ADDRESS";
const uint16_t serverPORT = 9999; 

WiFiClient client;
// Prepare the data
String data = "Hello, Server!"; 
void connectToWiFi(){
  Serial.print("Connecting to ssid " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    Serial.println("...");
    delay(250);
  }
  Serial.println("Connected!");
}

void serverConnect(){
  //using serverIP address and serverPORT to establish connection
  if (client.connect(serverIP, serverPORT)){
    Serial.println("Connected to Node-RED");
    // Only send the data if it's not empty
    if (data.length() > 0) {
      client.print(data);  // Send data to Node-RED
      Serial.println("data sent to Node-RED: " + data);
    } else {
      Serial.println("data is empty, not sending.");
    }
  } else {
    Serial.println("Failed to connect to Node-RED");
  }
}
void setup(){
  Serial.begin(115200);
  connectToWiFi();
  Serial.println(WiFi.localIP());
  //establish connection between server and client
  serverConnect();
}

void loop(){
  //sending data to server and ending the connection
  while (client.available()){
    String message = client.readStringUntil('\n');
    Serial.println("Message from Node-RED: " + message); // Print it
  }
  client.stop(); 
  if (WiFi.status() != WL_CONNECTED){
    connectToWiFi();
  }
  serverConnect();
  establish connection every 3 second
  delay (3000);
}