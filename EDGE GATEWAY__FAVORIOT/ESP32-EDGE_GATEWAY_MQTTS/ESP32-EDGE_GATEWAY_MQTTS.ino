#include <WiFi.h>
#include <MQTT.h>
#include <NetworkClientSecure.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>
#include "FavoriotCA_MQTT.h"

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char gatewayDeveloperID[] = "YOUR_GATEWAY_DEVELOPER_ID";
const char gatewayAccessToken[] = "YOUR_GATEWAY_ACCESS_TOKEN";
const char publishTopic[] = "/v2/gateway/streams";
const char statusTopic[] = "/v2/gateway/streams/status";
const char* clientID = "@XK-Esp32-GATEWAY";

Adafruit_APDS9960 apds;
Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

float hPaSeaLevel = 1015.00;
unsigned long lastMillis = 0;
NetworkClientSecure client;
MQTTClient mqtt(4096);

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("...");
    delay (250);
  }
  Serial.println(" connected!");
}

void GetData(String &topic, String &payload) {
  Serial.println("Incoming Status: " + payload);
  Serial.println();
}

void connectToFavoriotMQTT() {
  Serial.print("Connecting to Favoriot MQTT ...");
  client.setCACert(FavoriotCA_MQTT);
  mqtt.begin("mqtt.favoriot.com", 8883, client);
  mqtt.onMessage(GetData);

  // to create the client id can be a random configuration but its better 
  // to custom it for better organization
  // like right now im already customize it in the universal line (const char*)
  // String uniqueString = String(ssid) + "-" + String(random(1, 98)) + String(random(99, 999));
  // char ClientID[uniqueString.length() + 1];
  // uniqueString.toCharArray(ClientID, uniqueString.length() + 1);
  while (!mqtt.connect(clientID, deviceAccessToken, deviceAccessToken)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" connected!");  
  Serial.println("Subscribe to: " + String(statusTopic));
  mqtt.subscribe(String(statusTopic));
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  if (!apds.begin() || !bme.begin() || !mpu.begin()){
    Serial.println("Not detecting the Hibiscus Sense sensor");
  }
  apds.enableProximity(true); 
  Serial.println();
  connectToWiFi();
  connectToFavoriotMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  if (!mqtt.connected()) {
    connectToFavoriotMQTT();
  }
  mqtt.loop();
  delay(10);
  SendData();
  Serial.println();
  delay (250);
}

void SendData() {
  int proximity = apds.readProximity();
  float humidity = bme.readHumidity();
  float temperature = bme.readTemperature();
  float barometer = bme.readPressure() / 100.00;
  float altitude = bme.readAltitude(hPaSeaLevel);
  mpu.getEvent(&a, &g, &temp);
  float accx = a.acceleration.x;
  float accy = a.acceleration.y;
  float accz = a.acceleration.z;
  float gyrx = g.gyro.x;
  float gyry = g.gyro.y;
  float gyrz = g.gyro.z;

  Serial.println("Proximity: " + String(proximity));
  Serial.println("Relative Humidity: " + String(humidity) + " %RH");
  Serial.println("Approx. Altitude: " + String(altitude) + " m");
  Serial.println("Barometric Pressure: " + String(barometer) + " Pa");
  Serial.println("Ambient Temperature: " + String(temperature)+" °C");
  Serial.print("Acceleration X:" + String(accx) + ", Y:" + String(accy));
  Serial.println(", Z:" + String(accz) + " m/s^2");
  Serial.print("Rotation X:" + String(gyrx) + ", Y:" + String(gyry));
  Serial.println(", Z:" + String(gyrz) + " rad/s");

  if(millis() - lastMillis > 2000){
    lastMillis = millis();
    // Prepare the payload data to send
    String payload = "{\"gateway_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{";
    payload += "\"proximity\":\"" + String(proximity) + "\",";
    payload += "\"humidity\":\"" + String(humidity) + "\",";
    payload += "\"altitude\":\"" + String(altitude) + "\",";
    payload += "\"barometer\":\"" + String(barometer) + "\",";
    payload += "\"temperature\":\"" + String(temperature) + "\",";
    payload += "\"accx\":\"" + String(accx) + "\",";
    payload += "\"accy\":\"" + String(accy) + "\",";
    payload += "\"accz\":\"" + String(accz) + "\",";
    payload += "\"gyrox\":\"" + String(gyrx) + "\",";
    payload += "\"gyroy\":\"" + String(gyry) + "\",";
    payload += "\"gyroz\":\"" + String(gyrz) + "\"";
    payload += "}}";

  // Publishing to Favoriot
  Serial.println("\nSending data to Favoriot's Data Stream ...");
  Serial.println("Data to Publish: " + payload);
  Serial.println("Publish to: " + String(gatewayAccessToken)+ String(publishTopic));
    // Check if the publish was successful
    bool success = mqtt.publish(String(gatewayDeviceToken)+String(publishTopic), payload);
    if (success) {
        Serial.println("Data successfully sent to Favoriot!");
    } else {
        Serial.println("Failed to send data to Favoriot.");
    }
  }
}