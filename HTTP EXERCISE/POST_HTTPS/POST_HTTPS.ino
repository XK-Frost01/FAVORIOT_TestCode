/* Coding ni untuk terima arahan dari dashboard favoriot dan dihantar ke hibiscus sense 
   dan hantar data dari hibiscus sense ke favoriot data stream dan didisplay ke favoriot
   dashboard.

   Sensor yang ada dalam hibiscus sense:
    1. LED
    2. RGB
    3. APDS 9960    - Proximity
                    - Gesture
    4. BME 280      - Humidity
                    - Altitude
                    - Pressure
                    - Temperature
    5. MPU 6050     - Accelerometer
                    - Gyroscope
*/

/* Semua library yang kan digunakan */  
#include <WiFi.h>                   // Untuk WiFi connection
#include <HTTPClient.h>             // untuk hantar dan terima data guna http protocol
#include <NetworkClientSecure.h>    // Untuk secure data yang dihantar
#include <Adafruit_APDS9960.h>      
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>
#include "FavoriotCA.h"             // cerification authentication untuk security (hhtps)

/* buat object untuk setiap library (naming) */
Adafruit_APDS9960 apds;
Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;

sensors_event_t  a, g, temp;
 
// set WiFi
const char ssid[]= "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperID[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char Apikey[] = "YOUR_API_KEY";

String ReceiveResponse, State, buzzer;
long previousMillis = 0;
int r = 0, gr = 0, b = 0;

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("...");
    delay (250);
  }
  Serial.println(" connected!");
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
} 

void loop(){
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  SendData();
  Serial.println("=============================================");
  delay (250);
}

void SendData(){
    // STEP 3: Data Ingestion - Send data to Favoriot's data stream using secure HTTP connection
    // Interval 15 seconds
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
    
  if(millis() - previousMillis > 15000){
    previousMillis = millis();
    String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{";
    
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

    Serial.println("\nSending data to Favoriot's Data Stream ...");
    NetworkClientSecure *client = new NetworkClientSecure;
    if(client) {
      client -> setCACert(FavoriotCA);
      HTTPClient https;
      https.begin(*client, "https://apiv2.favoriot.com/v2/streams");
      https.addHeader("Content-Type", "application/json");
      https.addHeader("Apikey", Apikey);
        
      int httpCode = https.POST(json);
      if(httpCode > 0){
        Serial.print("HTTPS Request: ");
        httpCode == 201 ? Serial.print("Success, ") : Serial.print("Error, ");
        Serial.println(https.getString());
      }
      else{
        Serial.println("> HTTPS Request Connection Error!");
      }
      https.end();
      Serial.println();
      delete client;
    }
    else{
      Serial.println("Unable to create secure client connection!");
      Serial.println();
    }
  }
}