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
#include <Adafruit_NeoPixel.h>      // Untuk RGB 
#include "tones.h"                  // untuk buzzer
#include "FavoriotCA.h"             // cerification authentication untuk security (hhtps)

/* buat object untuk setiap library (naming) */
Adafruit_NeoPixel rgb(1,16);
Adafruit_APDS9960 apds;
Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;

sensors_event_t  a, g, temp;
 
// set WiFi
// const char ssid[]= "favoriot@unifi";
// const char password[] = "fav0r10t2017";
const char ssid[] = "derictanhk18@unifi";
const char password[] = "A287@SFERA";
const char deviceDeveloperID[] = "Hibiscus_Sense@nuraizatulsyakila01";
const char deviceAccessToken[] = "w0YAyn8HyozA5G0wPIRbpLwKG9JkwmWc";
const char Apikey[] = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6Im51cmFpemF0dWxzeWFraWxhMDEiLCJyZWFkX3dyaXRlIjp0cnVlLCJpYXQiOjE3Mjg4NzUwMjl9.X8Iw7N1002rj8wt0FbwP9KVrLR1llOAXr782JSPg1CQ";

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
  GetData();
  SendData();
  Serial.println("=============================================");
  delay (250);
}

void GetData() {
  NetworkClientSecure *client = new NetworkClientSecure;
  if (client) {
    client->setCACert(FavoriotCA);
    HTTPClient https;
    String url = "https://apiv2.favoriot.com/v2/rpc?device_developer_id=" + String(deviceDeveloperID) + "&timeout=5000";
    https.begin(*client, url);
    https.addHeader("Content-Type", "application/json");
    https.addHeader("Apikey", String(Apikey));

    int ResponseCode = https.GET();
    
    // Print out the response code and check if it's valid
    Serial.println("HTTP Response Code: " + String(ResponseCode)); 

    if (ResponseCode > 0) {
      ReceiveResponse = https.getString();
      Serial.println("Receive Payload: " + String(ReceiveResponse));
      messageReceived();
    } else {
      Serial.println("Error on HTTPS GET request");
    }
    https.end();
    delete client; // Properly delete the client to free resources
  } 
  else {
    Serial.println("Unable to create secure client connection");
  }
}
void messageReceived() {
    // Based on data received...
  if (ReceiveResponse.indexOf("\BUZZER\":\"ON\"") >= 0){
    buzzer = "ON";
    Serial.println("Buzzzer turned ON");
  }
  else if (ReceiveResponse.indexOf("\"BUZZER\":\"OFF\"") >= 0){
    buzzer = "OFF";
    Serial.println("Buzzer turned OFF");
  }
  if (buzzer == "ON"){
    if (ReceiveResponse.indexOf("\"SONG_1\":\"ON\"") >= 0){
      song1();
    }
    else if (ReceiveResponse.indexOf("\"SONG_2\":\"ON\"") >= 0){
      song2();
    }
  }
  if (ReceiveResponse.indexOf("\"LED_ON\":\"ON\"") >= 0) {
    digitalWrite(2, LOW);
    Serial.println("LED turned ON");
  } 
  else if(ReceiveResponse.indexOf("\"LED_OFF\":\"OFF\"") >= 0){
    digitalWrite(2, HIGH);
    Serial.println("LED turned OFF");
  }
  if (ReceiveResponse.indexOf("\"RGB\":\"ON\"") >= 0) {
    State = "ON";
    Serial.println("RGB turned ON");
  } 
  else if(ReceiveResponse.indexOf("\"RGB\":\"OFF\"") >= 0){
    State = "OFF";

    Serial.println("RGB turned OFF");
  }
  if(State == "ON"){
    if (ReceiveResponse.indexOf("\"RGB_Red\":") != -1) {
       r = extractValue("\"RGB_Red\":");
    }
    if (ReceiveResponse.indexOf("\"RGB_Green\":") != -1) { 
       gr = extractValue("\"RGB_Green\":"); // corrected the key
    }
    if (ReceiveResponse.indexOf("\"RGB_Blue\":") != -1) { 
       b = extractValue("\"RGB_Blue\":"); // corrected the key
    }
    Serial.println("Extracted Values:");
    Serial.println("Red: " + String(r));
    Serial.println("Green: " + String(gr));
    Serial.println("Blue: " + String(b));
    rgb.setPixelColor(0, r,gr,b);
  }
  else {
    rgb.setPixelColor(0,0,0,0);
  }
    rgb.show();
}


int extractValue(String Slider) {
  int startIndex = ReceiveResponse.indexOf(Slider) + Slider.length();
  int endIndex = ReceiveResponse.indexOf(",",startIndex);
  if (endIndex == -1) {
    endIndex = ReceiveResponse.indexOf("}", startIndex);
  }
  String SliderValue = ReceiveResponse.substring(startIndex, endIndex);
  SliderValue.trim(); // Trim whitespace in place
  SliderValue.replace("\"", ""); // Remove any quotes
  return SliderValue.toInt();
}

void song1(){
  // tone() function will generate PWM signal based on given tone frequency.
  // 1st argument: GPIO number.
  // 2nd argument: Tone frequency.
  tone(13, NOTE_D4);  // buzzer will sound according to NOTE_D4.
  delay(500);

  noTone(13);  // buzzer has no sound since PWM signal is 0.
  delay(500);
}
void song2(){
  // tone() function will generate PWM signal based on given tone frequency.
  // 1st argument: GPIO number.
  // 2nd argument: Tone frequency.
  tone(13, NOTE_D4);  // buzzer will sound according to NOTE_D4.
  delay(500);

  noTone(13);  // buzzer has no sound since PWM signal is 0.
  delay(500);
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