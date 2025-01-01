#include <WiFi.h>
#include <WebServer.h>
#define log_name "admin"
#define log_pass "admin"

WebServer server(80); 

const char *APssid="Esp_32";
const char *APpass="pass1234";
// THIS IS FOR INPUT DATA
String ssid ="";
String password ="";
String ID ="";
String token ="";

// HTML web page
const char login_HTML[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <title>LOGIN PAGE</title>
    <style> 
      h2 { 
        margin: 0 auto;
        text-align: center;
        font-family: verdana; 
        font-size: 300%;
      }  
      body { 
        font-family: Arial, sans-serif; 
      }  
      p,label,input { 
        display: block; 
        margin: 10px auto; 
        width: 80%; 
        max-width: 300px; 
      }  
      input[type="submit"] {
        width: 80%; 
        display: block; 
        margin: 50px auto;
        font-size:20px; 
      }
    </style>
  </head>
    
  <body>
    <h2>WELCOME TO LOGIN PAGE</h2>
    <form action='/' method='POST'>
      To log in, please use : admin/admin<br><br>
      Username:<input type='text' name='USERNAME'><br>
      Password:<input type='password' name='PASSWORD'>
      <input type='submit' value='Login'>
    </form>
  </body></html>
)rawliteral";
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
const char third_HTML[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <title>ESP32 Success</title>
    <style>
      h2 {
        background-color: LightGreen;
        text-align: center;
        font-family: verdana;
        font-size: 300%;
      }
      body {
        font-family: Arial, sans-serif;
        text-align: center;
      }
      p {
        font-size: 20px;
      }
    </style>
  </head>
  <body>
    <h2>WiFi Connected!</h2>
    <p>Welcome! Your ESP32 is now connected to the WiFi network.</p>
    <p>Visit the <a href='https://platform.favoriot.com'>FAVORIOT Platform</a> to check your data.</p>
  </body></html>
)rawliteral";

String currentline = "";

void WiFiAP_connect();
void handle_First_Interface();
void handle_Submit();
void connect_to_WiFi();
void handle_Finish();

void WiFiAP_connect(){
  if (!WiFi.softAP(APssid, APpass)){
    Serial.println("Failed to start AP");
    while (true);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("AP IP address: ");
  Serial.println(myIP);
}

void connect_to_WiFi(){
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 50) {
    delay(250);
    Serial.print(".");
    attempts++;
  }
}

void setup() {
  Serial.begin(115200);
  WiFiAP_connect();
  // Handle GET request to root (redirect to login page)
  server.on("/", HTTP_GET, [](){
    server.sendHeader("Location", "/login");
    server.send(302); // Redirect to /login
  });
  // Handle POST request to root (login form submission)
  server.on("/", HTTP_POST, handle_First_Page);
  // Serve login page on GET request to /login
  server.on("/login", HTTP_GET, [](){
    server.send(200, "text/html", login_HTML);
  });
  // Handle form submission
  server.on("/submit", HTTP_POST, handle_Submit);
  server.on("/finish", HTTP_POST, handle_Finish);
  server.begin();
  Serial.println("Access Point Started");
}

void loop(){
  server.handleClient(); //check for new client;
}

void handle_First_Page(){
  String username = server.arg("USERNAME");
  String password = server.arg("PASSWORD");
  
  Serial.println("Received Login Attempt:");
  Serial.println("Username: " + username);
  Serial.println("Password: " + password);

  if(log_name == username && log_pass == password){
    Serial.println("Login successful");
    server.send(200, "text/html", index_HTML);
  } else {
    Serial.println("Login failed. Redirecting to /login");
    server.sendHeader("Location", "/login"); 
    server.send(302); // HTTP 302 Redirect
  }
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

void handle_Finish(){
  connect_to_WiFi();
  if (WiFi.status()==WL_CONNECTED){
    Serial.println("Connected to WiFi!");
    Serial.print("New IP: ");
    Serial.println(WiFi.localIP());
    server.send(200,"text/html",third_HTML);
  } else {
    Serial.println("Failed to connect to WiFi!");
    server.send(200, "text/html", "<h2>Failed to connect to Wi-Fi.</h2><p>Please try again.</p>");
  }
}