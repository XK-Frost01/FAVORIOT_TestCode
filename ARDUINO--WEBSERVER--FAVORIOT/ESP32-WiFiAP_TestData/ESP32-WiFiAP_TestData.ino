#include <WiFi.h>

const char *APssid="Esp_32";
const char *APpass="pass1234";

WiFiServer server(80); 

void WiFiAP_connect(){
  if (!WiFi.softAP(APssid, APpass)){
    log_e("Soft AP creation failed.");
    while(1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("AP IP address: ");
  Serial.println(myIP);
}
void setup() {
  Serial.begin(115200);
  Serial.println("\nConfiguring access point...");
  WiFiAP_connect();
  server.begin();
  Serial.println("Server started");
}

void loop(){
  WiFiClient client  = server.available();
  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
    client.print("ESP32 says HELLO");
    client.println();  
    client.stop();
    Serial.println("Client Disconnected");
  }
}