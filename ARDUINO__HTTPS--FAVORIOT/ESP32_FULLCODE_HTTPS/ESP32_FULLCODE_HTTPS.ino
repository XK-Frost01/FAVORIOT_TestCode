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
  GetData();
  SendData();
  Serial.println();
  delay (250);
}

void SendData(){
    // STEP 3: Data Ingestion - Send data to Favoriot's data stream using secure HTTP connection
    // Interval 15 seconds
  if(millis() - previousMillis > 15000){
    previousMillis = millis();

    mpu.getEvent(&a,&g,&temp);
    Serial.println("Proximity: " + String(apds.readProximity()));
    Serial.println("Relative Humidity: "+ String(bme.readHumidity()) + " %RH");
    Serial.println("Approx. Altitude: " + String(bme.readAltitude(1013.25)) + " m");
    Serial.println("Barometric Pressure: " + String(bme.readPressure()) + " Pa");
    Serial.println("Ambient Temperature: " + String(bme.readTemperature()) + " °C");
    Serial.println("Acceleration X:" + String(a.acceleration.x) + ", Y:"+String(a.acceleration.y) + ", Z:"+String(a.acceleration.z) + " m/s^2");
    Serial.println("Rotation X:" + String(g.gyro.x) + ", Y:" + String(g.gyro.y) + ", Z:" + String(g.gyro.z) + " rad/s");

    String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{";
          json += "\"proximity\":\"" + String(apds.readProximity()) + "\",";
          json += "\"humidity\":\"" + String(bme.readHumidity()) + "\",";
          json += "\"altitude\":\"" + String(bme.readAltitude(1013.25)) + "\",";
          json += "\"barometer\":\"" + String(bme.readPressure()/100.00) + "\",";
          json += "\"temperature\":\"" + String(bme.readTemperature()) + "\",";
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
      https.begin("https://apiv2.favoriot.com/v2/streams");
      https.addHeader("Content-Type", "application/json");
      https.addHeader("Apikey", Apikey);
        
      int httpCode = https.POST(json);
      if(httpCode > 0){
        Serial.print("HTTPS Request: ");
        httpCode == 201 ? Serial.print("Success, ") : Serial.print("Error, ");
        Serial.println(https.getString());
      } else{
        Serial.println("> HTTPS Request Connection Error!");
      }
      https.end();
      delete client;
    } else{
      Serial.println("Unable to create secure client connection!");
    }
  }
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
    Serial.println("HTTP Response Code: " + String(ResponseCode)); 
    if (ResponseCode > 0) {
      ReceiveResponse = https.getString();
      Serial.println("Receive Payload: " + String(ReceiveResponse));
      messageReceived();
    } else {
      Serial.println("Error on HTTPS GET request");
    }
    https.end();
    delete client;
  } else {
    Serial.println("Unable to create secure client connection");
  }
}

void messageReceived() {
  if (ReceiveResponse.indexOf("\BUZZER\":\"ON\"") >= 0){
    buzzer = "ON";
    Serial.println("Buzzzer turned ON");
  } else if (ReceiveResponse.indexOf("\"BUZZER\":\"OFF\"") >= 0){
    buzzer = "OFF";
    Serial.println("Buzzer turned OFF");
  }

  if (buzzer == "ON"){
    if (ReceiveResponse.indexOf("\"SONG_1\":\"ON\"") >= 0){
      song1();
    } else if (ReceiveResponse.indexOf("\"SONG_2\":\"ON\"") >= 0){
      song2();
    }
  }

  if (ReceiveResponse.indexOf("\"LED_ON\":\"ON\"") >= 0) {
    digitalWrite(2, LOW);
    Serial.println("LED turned ON");
  } else if(ReceiveResponse.indexOf("\"LED_OFF\":\"OFF\"") >= 0){
    digitalWrite(2, HIGH);
    Serial.println("LED turned OFF");
  }

  if (ReceiveResponse.indexOf("\"RGB\":\"ON\"") >= 0) {
    State = "ON";
    Serial.println("RGB turned ON");
  } else if(ReceiveResponse.indexOf("\"RGB\":\"OFF\"") >= 0){
    State = "OFF";
    Serial.println("RGB turned OFF");
  }

  if(State == "ON"){
    if (ReceiveResponse.indexOf("\"RGB_Red\":") != -1) {
       r = extractValue("\"RGB_Red\":");
    } else if (ReceiveResponse.indexOf("\"RGB_Green\":") != -1) { 
       gr = extractValue("\"RGB_Green\":"); // corrected the key
    } else if (ReceiveResponse.indexOf("\"RGB_Blue\":") != -1) { 
       b = extractValue("\"RGB_Blue\":"); // corrected the key
    } 
    Serial.println("Extracted Values:");
    Serial.println("Red: " + String(r));
    Serial.println("Green: " + String(gr));
    Serial.println("Blue: " + String(b));
    rgb.setPixelColor(0, r,gr,b);
  } else {
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
  tone(13, NOTE_D4);  // buzzer will sound according to NOTE_D4.
  delay(500);
  noTone(13);  // buzzer has no sound since PWM signal is 0.
  delay(500);
}
void song2(){
  tone(13, NOTE_D5);  // buzzer will sound according to NOTE_D5.
  delay(500);
  noTone(13);  // buzzer has no sound since PWM signal is 0.
  delay(500);
}