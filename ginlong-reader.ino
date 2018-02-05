
#include <SPI.h>

#include <pins_arduino.h>

//display drivers
//#include <gfxfont.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_ST7735.h>
#include <TFT_ILI9163C.h>

//wifi headers
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
 
//RFID headers
#include "MFRC522.h"

//constants
//Screen
#define   TFTDC   D2
#define   TFTCS   D8
#define   TFTBL   D1
#define   TFTRST  D0


#define POWERSW D4

//RFID
#define   RFID_RST_PIN  D4
#define   RFID_SS_PIN   D3

//SD Card
#define   SDCARD_CS_PIN  D9

//initialisation functions
//screen
//Adafruit_ST7735 tft = Adafruit_ST7735(TFTCS, TFTDC, TFTRST);

TFT_ILI9163C tft = TFT_ILI9163C(TFTCS, TFTDC, TFTRST);

//RFID
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);



// WiFi parameters to be configured
const char* ssid = "porcini";
const char* password = "";
 IPAddress me(192, 168, 1,5);    
IPAddress gateway(192, 168, 1, 1); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
float setPoint=0;

//WiFiServer server;
WiFiUDP udp;
const int MAX_SIZE=500;
char packetBuffer[MAX_SIZE];

void setup() {
  //initialize the serial port
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  while(!Serial);
  Serial.println("Serial line configured");
  
  //initialise screen
  pinMode(TFTDC, OUTPUT);
  pinMode(TFTCS, OUTPUT);
  pinMode(TFTBL, OUTPUT);
  pinMode(TFTRST, OUTPUT);

//  tft.initR(INITR_BLACKTAB); //change to INITR_GREENTAB if this doesn't work
  tft.begin();
  analogWrite(TFTBL, 2024); //backlight at 50%

  
  pinMode(POWERSW,OUTPUT);
  powerOff();  
  
  //RFID initialisation
  pinMode(RFID_SS_PIN, OUTPUT);
  pinMode(RFID_RST_PIN, OUTPUT);
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial(); //dump the version of the nfc reader to serial

  //wifi init
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);

  
  //tft init
  tft.setRotation(2);
//  tft.fillScreen(ST7735_BLACK);
  tft.fillScreen(BLACK);
  tft.setCursor(0,0);
  tft.setTextColor(WHITE,BLACK);
  // put your setup code here, to run once:
  tft.print("Connecting to porcini");
  // Connect to WiFi
 WiFi.config(me, gateway, subnet);
 WiFi.begin(ssid, password);

  // while wifi not connected yet, print '.'
  // then after it connected, get out of the loop
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
    tft.println("WiFi connected");
  // Print the IP address
  tft.println(WiFi.localIP());
     //server.begin(2300);
     udp.begin(3030);


     
}

void loop() {
//  tft.fillScreen(BLACK);
//  tft.setCursor(0,0);
  int packetSize= udp.parsePacket();
  if( packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(udp.remotePort());
    // read the packet into packetBufffer
    udp.read(packetBuffer,packetSize);

//    Serial.println("Contents:");
//    for( int i=0; i < packetSize; i++){
//      Serial.print((int)packetBuffer[i],HEX);
//      Serial.print(" ");
//    }
//    Serial.println();
    if( packetBuffer[3]==0xB1){
      return;
    }
    int x1 = packetBuffer[59]&0xff;
    int x2 = packetBuffer[60]&0xff;
    int watts = (x1*256) + x2;

      tft.setCursor(0,80);

    tft.print("watts:" );
    tft.print(watts);
    tft.println("   "); // erase line
    Serial.println(watts);
    Serial.println("watts:");
    int tempI =analogRead(A0);

  tft.setCursor(0,160);

    tft.print("temp(raw)" );
    tft.print(tempI);
    tft.println("    ");
    float temp = ((tempI / 4096.0) * 100.0)-273.4;
    tft.print("temp:" );
    tft.print(temp);
    tft.println("    ");
    
    if ( watts > 3000 && temp < setPoint) {
        powerOn();
    } else {
        powerOff();
    }
  }
}


void powerOn(){
    Serial.println("power on");
      digitalWrite(POWERSW,HIGH);  // have power
}
void powerOff(){
    digitalWrite(POWERSW,LOW); //not so much
    Serial.println("power off");
}


