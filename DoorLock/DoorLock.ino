#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include "FirebaseESP32.h"
#include <NTPClient.h>
#include <LiquidCrystal.h>
#include <time.h>
#include "Esp.h"

//listfungsi
void wifiSetup();
void firebaseSetup();
void rfidSetup();
void firebaseGetStatus();
void firebaseCheckStatus();
void firebaseReport(int);
void firebaseUserReport(int);
void relayOn();
void relayOff();
void pinModeDefined();
void CheckCardIdentity();
void CheckNewCard();


//user_class

//Devined Variable

//--Setting WIfi
#define WIFI_SSID "House2White"               
#define WIFI_PASSWORD "listrikm4h4l"

//--Setting Database
#define FIREBASE_HOST "bandd-27b59.firebaseio.com"   
#define FIREBASE_AUTH "XEsldSIWaJIl1nXTeAsrGYNEFFp7CYWAADaodLTe"  

//--Setting RFID
#define RFID_SDA_PIN 21
#define RFID_RST_PIN 22

#define RELAY_IN 13
#define BUZZER 15
#define LED_BUILTIN 2

//LCD pins
#define LCD_DATA7 14
#define LCD_DATA6 27
#define LCD_DATA5 26
#define LCD_DATA4 25
#define LCD_ENABLE 33
#define LCD_REGISTER_SELECT 32

//--Setting Device
const String DeviceID = "pintu_1";

//Variable
String database_door_status;
String content;
String users_id[] = {"079B5E62","1C9669DE","07A77A62"};
String users_data[][2] = {{"079B5E62","irzam"},{"1C9669DE","mazri"},{"07A77A62","zamir"}};
String report;
String deviceAddress = "/devices/"+ DeviceID +"/status";
String username,timenow,userID;
int database_device_status ;
int user_number = (sizeof(users_id) / 8) - 1;
int i ;
int n ;

//Defined Class
FirebaseData firebaseData;
MFRC522 mfrc522(RFID_SDA_PIN, RFID_RST_PIN);
FirebaseJson  jsonFile;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//Defined Pin
#define RELAY_IN 13
#define RFID_SDA_PIN 21
#define RFID_RST_PIN 22

void setup() {
  Serial.begin(9600);
  pinModeDefined();
  wifiSetup();
  firebaseSetup();
  rfidSetup();  
}

void loop() {
 if (n<4){
  firebaseGetStatus();
  firebaseCheckStatus();
  firebaseCheckStatus();
  deviceControl();
  CheckNewCard();
  n++;}
 else
  esp.restart;
}


//------------------------------------------------------------------------------------------------------------------------//

void wifiSetup(){
  Serial.print("Wifi Connecting :");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                  
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
}

void firebaseSetup(){
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); 
}

void rfidSetup(){
  SPI.begin();
  mfrc522.PCD_Init();  
}

void timeAPI(){
  timeClient.begin();  
}

//------------------------------------------------------------------------------------------------------------------------//

void firebaseGetStatus(){
  
  Firebase.getString(firebaseData, deviceAddress);
  database_door_status = firebaseData.stringData();  
  //Serial.println(database_door_status);
  delay(500);
}


void firebaseReport(int i){
  firebaseUserReport(i);
  //Firebase.pushString(firebaseData,"/report/","panji");
  if(Firebase.pushJSON(firebaseData,"/reports/",jsonFile) == true){
    Serial.println("Sukses Menulis Database");  
  }else{
    Serial.println("Gagal");  
  }
}

void firebaseCheckStatus(){
  if(database_door_status.compareTo("true")){
    database_device_status = 0;
  } else if(database_door_status.compareTo("false")){
    database_device_status = 1;
  }
}

void deviceControl(){
  if(database_device_status){
    relayOn();
    //Serial.println("hidup");  
  } else {
    relayOff();
    //Serial.println("mati");
  }  
}


void relayOn(){
  digitalWrite(RELAY_IN,HIGH);
}

void relayOff(){
  digitalWrite(RELAY_IN,LOW);
}

void pinModeDefined(){
  pinMode(RELAY_IN,OUTPUT);
}

void CheckNewCard(){
  if ( ! mfrc522.PICC_IsNewCardPresent())return;
  if ( ! mfrc522.PICC_ReadCardSerial())return;
  content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     //Serial.print(mfrc522.uid.uidByte[i], HEX);
     content+=String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
     content+=String(mfrc522.uid.uidByte[i], HEX);
     //content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : ""));
     //content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println(content);
  CheckCardIdentity();
}

void CheckCardIdentity(){

  for(i=0;i<user_number;i++){
    Serial.println(users_id[i]);
    Serial.println(content.compareTo(users_id[i]));
    if (content.compareTo(users_id[i])== 0){
        firebaseReport(i);
        Serial.println("Authorized access");
        Serial.println();
        digitalWrite (RELAY_IN,HIGH);
        delay(3000);
        digitalWrite (RELAY_IN,LOW);
        delay(3000);
        return;
      }
  }
  Serial.println(" Access denied");
  delay(1000);
}

void firebaseUserReport(int n){
  timeClient.update();
  username = users_data[n][1];
  timenow = timeClient.getEpochTime();
  userID = users_data[n][0];
  
  jsonFile.set("status", "oke");
  jsonFile.set("time", timenow);
  jsonFile.set("userID", userID);
  jsonFile.set("username", username);
}
