#include <WiFi.h>
#include <WebServer.h>

WebServer server(80); 

const char *APssid="Esp_32";
const char *APpass="pass1234";

// THIS IS FOR INPUT DATA
String ssid ="";
String password ="";
String ID ="";
String token ="";
// HTML web page

const char index_HTML[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
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
      label,input { 
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
        font-size:20px; 
      }
    </style>
  </head>
  <body>
    <h2>WELCOME TO ESP32 INTERFACE</h2>
    <br><hr>
    <form action="/submit" method="POST">
      <label for="ssid">WIFI SSID:</label>
      <input type="text" name ="ssid"><br><br>
      <label for="password">WIFI PASSWORD:</label>
      <input type="text" name="password"><br><br>
      <label for="dev_id">DEVICE DEVELOPER ID:</label>
      <input type="text" name="dev_id"><br><br>
      <label for="access_token">DEVICE ACCESS TOKEN:</label>
      <input type="text" name="access_token"><br>    
      <hr><br>
      <input type="submit" value="Submit" >
    </form> 
  </body></html>
)rawliteral";
const char secondFront_HTML[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html lang=en>
  <head> 
    <title>DATA SUBMITTED</title>
    <style>
      h2 {
        background-color: DodgerBlue;
        padding: 15px 30px; 
        text-align: center; 
        font-family: verdana; 
        font-size: 300%;
      }
      body { 
        font-family: Arial, sans-serif; 
      }
      p {
        display: block; 
        margin: 10px 0; 
        margin-left: 10%;
        width: 80%; 
        max-width: 600px; 
      }
      input[type="submit"], a{
        background-color: DodgerBlue; 
        width: auto; 
        display: inline-block;
        margin: 50px auto;
        text-align:center;
        font-size:20px;
        text-decoration: none;   /* Removes underline for <a> */
        border: none;
        color:black;
        padding: 10px 20px;  
      }
      div.mycontainer {
        display: flex;
        justify-content: center; /* Center the items horizontally */
        gap: 10px; /* Add space between items */
      }
    </style>
  </head>

  <body>
    <h2>Data Received!</h2>)rawliteral";

const char secondEnd_HTML[] PROGMEM = R"rawliteral(
  <br><hr>
  <div class="mycontainer">
    <div>
      <a href='/'>Return to Form</a>
    </div>
  
    <div>
      <form action = "/finish" method="POST">
        <input type="submit" value= "Finish Configure">
      </form>
    </div>
  </div> 
  <hr><br>
  </body></html>
)rawliteral";

String currentline = "";

void WiFiAP_connect();
void handle_First_Interface();
void handle_Submit();

void WiFiAP_connect(){
  if (!WiFi.softAP(APssid, APpass)){
    Serial.println("Failed to start AP");
    while (true);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("AP IP address: ");
  Serial.println(myIP);
}

void setup() {
  Serial.begin(115200);
  WiFiAP_connect();
  server.on("/", HTTP_GET, [](){
    server.send(200, "text/html", index_HTML);
  });
  server.on("/submit", HTTP_POST, handle_Submit);
  server.begin();
  Serial.println("Access Point Started");
}

void loop(){
  server.handleClient(); //check for new client;
}

void handle_Submit(){
  ssid = server.arg("ssid");
  password = server.arg("password");
  ID = server.arg("dev_id");
  token = server.arg("access_token");

  Serial.println("Received Data:");
  Serial.println("SSID: " + ssid);
  Serial.println("Password: " + password);
  Serial.println("ID: " + ID);
  Serial.println("Access Token: " + token);

  // Respond back to the client
                        // Append static HTML start
  String fullResponse = FPSTR(secondFront_HTML);
                        // Append dynamic content
        fullResponse += "<p>WiFi SSID: " + ssid + "</p>";
        fullResponse += "<p>WiFi Password: " + password + "</p>";
        fullResponse += "<p>Device ID: " + ID + "</p>";
        fullResponse += "<p>Access Token: " + token + "</p>";
                        // Append static HTML end
        fullResponse += FPSTR(secondEnd_HTML);

  server.send(200, "text/html", fullResponse);
}