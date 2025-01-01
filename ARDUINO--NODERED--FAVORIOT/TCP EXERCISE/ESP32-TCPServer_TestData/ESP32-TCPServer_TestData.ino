// if another esp32 is set as server
// the use of tcp will be as below
#include <WiFi.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
WiFiServer server(9999); // set server port as 9999 (default TCP port)

void connectToWiFi(){
  Serial.println("Connecting to WiFi " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(...);
    delay(250);
  }
  Serial.println("");
  Serial.println("Connected!");
}

void setup(){
  Serial.begin(115200);
  connectToWiFi();
  serial.println(WiFi.localIP());
  server.begin();
  }
}

void loop(){
  // client object give result of true/false
  WiFiClient client = server.available();
  // check if there any connection in server
  if (client){
    Serial.println("New Client Connected");
    // while connection is establish
    while (client.connected()){
      if (client.available()){
        String message = client.readStringUntil('\n');
        Serial.println("Received: " + message);
        client.print("Echo: " + message);
      }
    }
    //stop the connection establish
    client.stop();
    Serial.println("Client disconnected");
  }
}
