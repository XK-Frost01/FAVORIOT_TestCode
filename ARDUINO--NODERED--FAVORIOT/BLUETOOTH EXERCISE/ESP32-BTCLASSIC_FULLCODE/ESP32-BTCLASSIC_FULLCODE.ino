#include <Adafruit_APDS9960.h>      
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>
#include "FavoriotCA.h"             // cerification authentication untuk security (https)
#include "BluetoothSerial.h"

/* buat object untuk setiap library (naming) */
Adafruit_APDS9960 apds;
Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;
BluetoothSerial SerialBT;

sensors_event_t  a, g, temp;
const char deviceDeveloperID[] = "YOUR_DEVICE_DEVELOPER_ID";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

String device_name = "ESP32-BT-Slave";
String ReceiveResponse;
char incomingChar;
long previousMillis = 0;

void setup(){
  Serial.begin (115200);
  pinMode(2,OUTPUT); 
  digitalWrite(2, HIGH);

  if (!apds.begin() || !bme.begin() || !mpu.begin()){
    Serial.println("Not detecting the Hibiscus Sense sensor");
  }    
  apds.enableProximity(true);  
  SerialBT.begin(device_name);  //Bluetooth device name
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
} 

void loop(){
  GetData();
  SendData();
  delay (250);
}

void GetData() {
// Read received messages (LED control command)
  if (SerialBT.available()){
    char incomingChar = SerialBT.read();
    if (incomingChar != '\n'){
      ReceiveResponse += String(incomingChar);
      messageReceived();
    } else{
      ReceiveResponse = "";
    }
    Serial.write(incomingChar);  
  }
}

void messageReceived() {
  if (ReceiveResponse == "LED_ON"){
    digitalWrite(2, LOW);
    SerialBT.println("LED turned ON");
  } else if (ReceiveResponse == "LED_OFF"){
    digitalWrite(2, HIGH);
    SerialBT.println("LED turned OFF");
  }
}

void SendData(){
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
    SerialBT.println(json);
  }
}