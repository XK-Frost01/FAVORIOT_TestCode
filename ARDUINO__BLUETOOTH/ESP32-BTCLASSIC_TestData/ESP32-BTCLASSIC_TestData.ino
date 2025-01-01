#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

String device_name = "ESP32-BT-Slave";
String ReceiveResponse = ""; // Declare the variable to store received data

void SendData();
void setup(){
  Serial.begin (115200);
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
    } else{
      ReceiveResponse = "";
    }
    Serial.write(incomingChar);  
  }
}

void SendData(){
    String message = "Hello From ESP32";
    SerialBT.println(message);
}