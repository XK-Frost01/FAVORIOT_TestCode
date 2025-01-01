#include <WiFi.h>
#include <MQTT.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>

const char ssid[]= "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperID[] = "YOUR_DEVICE_DEVELOPER_ID";
const char* broker = "192.168.0.43";
const char* publishTopic = "arduino/nodered";
const char* clientID = "@XK-Esp32-PUBLISH";
Adafruit_APDS9960 apds;
Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

long previousMillis = 0;
String ReceiveResponse;
WiFiClient client;
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
void connectToFavoriotMQTT() {    
  Serial.print("Connecting to MQTT mosquitto...");
  mqtt.begin(broker, 1883, client);
  while (!mqtt.connect(clientID)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n connected!");
}
void setup(){
  Serial.begin (115200);
  if (!apds.begin() || !bme.begin() || !mpu.begin()){
    Serial.println("Not detecting the Hibiscus Sense sensor");
  }   
  apds.enableProximity(true);  
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
  delay(250);
}

void SendData(){
  if(millis() - previousMillis > 3000){
    previousMillis = millis();

    Serial.println("Proximity: "+String(apds.readProximity()));
    Serial.println("Relative Humidity: "+String(bme.readHumidity())+" %RH");
    Serial.println("Approx. Altitude: "+String(bme.readAltitude(1013.25))+" m");
    Serial.println("Barometric Pressure: "+String(bme.readPressure())+" Pa");
    Serial.println("Ambient Temperature: "+String(bme.readTemperature())+" °C");

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
    Serial.println("Publish to: " +String(publishTopic));
    // Publish to /v2/streams
    mqtt.publish(String(publishTopic), json);
  }
}