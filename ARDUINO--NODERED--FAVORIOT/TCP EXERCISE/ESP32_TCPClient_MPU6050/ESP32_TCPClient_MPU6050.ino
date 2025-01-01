#include <WiFi.h>
#include <NetworkClientSecure.h>
#include <Adafruit_MPU6050.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char* deviceDeveloperID = "YOUR_DEVICE_DEVELOPER_ID";
const char*  serverIP= "YOUR_SERVER_IP_ADDRESS";
const uint16_t serverPORT = 9999; 

Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

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
      mpu.getEvent(&a,&g,&temp);
      Serial.println("Acceleration X:" +String(a.acceleration.x)+", Y:"+String(a.acceleration.y)+", Z:"+String(a.acceleration.z)+" m/s^2");
      Serial.println("Rotation X:"+String(g.gyro.x)+", Y:"+String(g.gyro.y)+", Z:"+String(g.gyro.z)+" rad/s");

      String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{"; 
            json += "\"accx\":\"" + String(a.acceleration.x) + "\",";
            json += "\"accy\":\"" + String(a.acceleration.y) + "\",";
            json += "\"accz\":\"" + String(a.acceleration.z) + "\",";
            json += "\"gyrox\":\"" + String(g.gyro.x) + "\",";
            json += "\"gyroy\":\"" + String(g.gyro.y) + "\",";
            json += "\"gyroz\":\"" + String(g.gyro.z) + "\"";
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