#include <WiFi.h>
#include <NetworkClientSecure.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char* deviceDeveloperID = "YOUR_DEVICE_DEVELOPER_ID";
const char*  serverIP= "YOUR_SERVER_IP_ADDRESS";
const uint16_t serverPORT = 9999; 

Adafruit_APDS9960 apds;
Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

long previousMillis = 0;
WiFiClient client;
String json = "";

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
  if (!apds.begin() || !bme.begin() || !mpu.begin()){
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
    sendData();
    if (json.length() > 0) {
      // Interval 3 second
      if(millis() - previousMillis > 3000){
        previousMillis = millis();
        client.print(json);   // Send data to Node-RED
        Serial.println("data sent to Node-RED: " + json);
      } 
    } else {
        Serial.println("data is empty, not sending.");
    } 
  } else {
    Serial.println("Failed to connect to Node-RED");
  }
}

void sendData(){
  Serial.println("Proximity: " + String(apds.readProximity()));
  Serial.println("Relative Humidity: " + String(bme.readHumidity()) + " %RH");
  Serial.println("Approx. Altitude: "+String(bme.readAltitude(1013.25))+" m");
  Serial.println("Barometric Pressure: "+String(bme.readPressure())+" Pa");
  Serial.println("Ambient Temperature: "+String(bme.readTemperature())+" °C");
  mpu.getEvent(&a,&g,&temp);
  Serial.println("Acceleration X:" +String(a.acceleration.x)+", Y:"+String(a.acceleration.y)+", Z:"+String(a.acceleration.z)+" m/s^2");
  Serial.println("Rotation X:"+String(g.gyro.x)+", Y:"+String(g.gyro.y)+", Z:"+String(g.gyro.z)+" rad/s");

  json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{"; 
  json += "\"proximity\":\"" + String(apds.readProximity()) + "\",";
  json += "\"humidity\":\"" + String(bme.readHumidity()) + "\",";
  json += "\"altitude\":\"" + String(bme.readAltitude(1013.25)) + "\",";
  json += "\"barometer\":\"" + String(bme.readPressure()/100.00) + "\",";
  json += "\"temperature\":\"" + String(bme.readTemperature()) + "\",";

  mpu.getEvent(&a,&g,&temp);

  json += "\"accx\":\"" + String(a.acceleration.x) + "\",";
  json += "\"accy\":\"" + String(a.acceleration.y) + "\",";
  json += "\"accz\":\"" + String(a.acceleration.z) + "\",";
  json += "\"gyrox\":\"" + String(g.gyro.x) + "\",";
  json += "\"gyroy\":\"" + String(g.gyro.y) + "\",";
  json += "\"gyroz\":\"" + String(g.gyro.z) + "\"";

  json += "}}"; 
}