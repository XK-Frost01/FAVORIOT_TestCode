#include <WiFi.h>
#include <WebServer.h>

WebServer server(80); 

const char *APssid = "Esp_32";
const char *APpass = "pass1234";

// HTML web page
const char index_HTML[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en-us">
  <head>
    <title>ESP32 INTERFACE</title>
    <style>
      h2 { 
        background-color: DodgerBlue;
        margin: 0 auto; 
        text-align: center; 
        font-family: verdana; 
        font-size: 300%;
      }
      body { 
        font-family: Arial, sans-serif; 
      }
      label, input { 
        display: block; 
        margin: 10px auto; 
        width: 80%; 
        max-width: 300px; 
      }
      input[type="submit"] {
        background-color: DodgerBlue; 
        width: 80%; 
        display: block; 
        margin: 50px auto;
        font-size: 20px; 
      }
    </style>
  </head>
  <body>
    <h2>WELCOME TO ESP32 INTERFACE</h2>
    <br><hr>
    <form action="/submit" method="POST">
      <label for="ssid">WIFI SSID:</label>
      <input type="text" name="ssid"><br><br>
      <label for="password">WIFI PASSWORD:</label>
      <input type="text" name="password"><br><br>
      <label for="dev_id">DEVICE DEVELOPER ID:</label>
      <input type="text" name="dev_id"><br><br>
      <label for="access_token">DEVICE ACCESS TOKEN:</label>
      <input type="text" name="access_token"><br>
      <hr><br>
      <input type="submit" value="Submit">
    </form> 
  </body>
  </html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  // Start the Access Point
  if (!WiFi.softAP(APssid, APpass)) {
    Serial.println("Failed to start AP");
    while (true);
  }
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Define route and send HTML directly in setup
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_HTML);
  });
  
  server.begin();
  Serial.println("Access Point Started");
}

void loop() {
  server.handleClient(); // Check for new client
}