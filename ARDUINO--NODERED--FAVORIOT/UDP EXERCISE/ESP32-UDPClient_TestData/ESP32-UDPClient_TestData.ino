#include <WiFi.h>
#include <WiFiUdp.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char*  serverIP= "YOUR_SERVER_IP_ADDRESS";
const uint16_t serverPort = 1010; // The port of the server to communicate with
WiFiUDP udp;

String message = "Hello from Client";  // The message you want to send to the server
  
void connectToWiFi(){
  Serial.println("Connecting to ssid " + String(ssid) + "...");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("...");
    delay(250);
  }
  Serial.println("");
  Serial.println("Connected!");
}

void setup(){
  Serial.begin(115200);
  connectToWiFi();
  Serial.println(WiFi.localIP());
  udp.begin(0); // Bind to any available port
}

void loop(){
  SendData();
  delay(3000); // Delay before sending the next message
}

void SendData(){
  // Send a message to the server
  udp.beginPacket(serverIP, serverPort); 
  udp.print(message); 
  udp.endPacket(); 
  // Wait for a response from the server
  delay(1000); // Wait a moment for server to respond
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char buffer[255];
    int len = udp.read(buffer, 255);
    buffer[len] = '\0'; // Null terminate the string
    Serial.println("Received from server: " + String(buffer));
  } else {
    Serial.println("No response from server");
  }
}