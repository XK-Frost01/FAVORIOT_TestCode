#include <WiFi.h>
#include <MQTT.h>
#include <NetworkClientSecure.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_NeoPixel.h>
#include "tones.h"
#include "FavoriotCA_MQTT.h"

const char ssid[] = "favoriot@unifi";
const char password[] = "fav0r10t2017";
// const char ssid[] = "derichktan18@unifi";
// const char password[] = "A287@SFERA";
const char deviceDeveloperID[] = "Hibiscus_Sense@nuraizatulsyakila01";
const char deviceAccessToken[] = "eH9cGiG974pyWOnDKcm7TRjDz8vEIP3G";
const char publishTopic[] = "/v2/streams";
const char statusTopic[] = "/v2/streams/status";
const char rpcTopic[] = "/v2/rpc";  // To Get data from dashboard
const char* clientID = "Syakila01-Esp32-PUBLISH";

Adafruit_NeoPixel rgb(1, 16);
Adafruit_APDS9960 apds;
Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

long previousMillis = 0;
NetworkClientSecure client;
MQTTClient mqtt(4096);

void connectToWiFi() {        // for easier calling in the loop
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("...");
    delay (250);
  }
  Serial.println(" connected!");
}

void GetData(String &topic, String &ReceiveResponse) {
  Serial.println("Incoming Status: " + ReceiveResponse);
  Serial.println();
}

// for easier calling in the loop
// if want to enter this line in the publish function also can... 
// but harder to call and it will constantly created and disconnected
void connectToFavoriotMQTT() {    
  Serial.print("Connecting to Favoriot MQTT ...");
  client.setCACert(rootCACertificate);
  mqtt.begin("mqtt.favoriot.com", 8883, client);
  mqtt.onMessage(GetData);

  // to create the client id can be a random configuration but its better 
  // to custom it for better organization
  // like right now im already customize it in the universal line (const char*)
  // String uniqueString = String(ssid) + "-" + String(random(1, 98)) + String(random(99, 999));
  // char uniqueClientID[uniqueString.length() + 1];
  // uniqueString.toCharArray(uniqueClientID, uniqueString.length() + 1);

  while (!mqtt.connect(clientID, deviceAccessToken, deviceAccessToken)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println(" connected!");
  Serial.println("Subscribe to: " + String(deviceAccessToken) + String(statusTopic));
  mqtt.subscribe(String(deviceAccessToken) + String(statusTopic));
  Serial.println();
}

void setup(){
  Serial.begin (115200);
  pinMode(2,OUTPUT); 
  digitalWrite(2, HIGH);

  if (!apds.begin() || !bme.begin() || !mpu.begin()){
    Serial.println("Not detecting the Hibiscus Sense sensor");
  }   
  apds.enableProximity(true);  
  rgb.begin();
  connectToWiFi();
  connectToFavoriotMQTT();
}

void loop() {
  // Check Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  // Check MQTT connection
  if (!mqtt.connected()) {
    connectToFavoriotMQTT();
  }

  mqtt.loop();
  delay(10);  // <- fixes some issues with WiFi stability
  SendData();
  Serial.println("=============================================");
  delay(3000);
}
void SendData(){
    // STEP 3: Data Ingestion - Send data to Favoriot's data stream using secure HTTP connection
    // Interval 15 seconds
  if(millis() - previousMillis > 15000){
    previousMillis = millis();

    Serial.print("Proximity: ");
    Serial.println(apds.readProximity());

    Serial.print("Relative Humidity: ");
    Serial.print(bme.readHumidity());
    Serial.println(" %RH");

    Serial.print("Approx. Altitude: ");
    Serial.print(bme.readAltitude(1013.25));
    Serial.println(" m");

    Serial.print("Barometric Pressure: ");
    Serial.print(bme.readPressure());
    Serial.println(" Pa");

    Serial.print("Ambient Temperature: ");
    Serial.print(bme.readTemperature());
    Serial.println(" °C");

    String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{";
    
    json += "\"proximity\":\"" + String(apds.readProximity()) + "\",";
    json += "\"humidity\":\"" + String(bme.readHumidity()) + "\",";
    json += "\"altitude\":\"" + String(bme.readAltitude(1013.25)) + "\",";
    json += "\"barometer\":\"" + String(bme.readPressure()/100.00) + "\",";
    json += "\"temperature\":\"" + String(bme.readTemperature()) + "\",";
    
    mpu.getEvent(&a,&g,&temp);
    
    Serial.print("Acceleration X:");
    Serial.print(a.acceleration.x);
    Serial.print(", Y:");
    Serial.print(a.acceleration.y);
    Serial.print(", Z:");
    Serial.print(a.acceleration.z);
    Serial.println(" m/s^2");

    Serial.print("Rotation X:");
    Serial.print(g.gyro.x);
    Serial.print(", Y:");
    Serial.print(g.gyro.y);
    Serial.print(", Z:");
    Serial.print(g.gyro.z);
    Serial.println(" rad/s");

    json += "\"accx\":\"" + String(a.acceleration.x) + "\",";
    json += "\"accy\":\"" + String(a.acceleration.y) + "\",";
    json += "\"accz\":\"" + String(a.acceleration.z) + "\",";
    json += "\"gyrox\":\"" + String(g.gyro.x) + "\",";
    json += "\"gyroy\":\"" + String(g.gyro.y) + "\",";
    json += "\"gyroz\":\"" + String(g.gyro.z) + "\"";
    
    json += "}}";

    Serial.println("\nSending data to Favoriot's Data Stream ...");
    Serial.println("Data to Publish: " + json);
    Serial.println("Publish to: " + String(deviceAccessToken) + String(publishTopic));
    // Publish to /v2/streams
    mqtt.publish(String(deviceAccessToken) + String(publishTopic), json);
  }
}