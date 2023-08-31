#include <stdio.h>
#include "Adafruit_FONA.h"

#define FONA_TX 4
#define FONA_RX 5
#define FONA_RST 9

#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX,FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

void setup() {
  // put your setup code here, to run once:
  while (!Serial);

  Serial.begin(115200);
  Serial.println(F("FONA basic test"));
  Serial.println(F("Initializing..."));

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
     Serial.println(F("Couldn't find FONA"));
     while (1);
  }

  Serial.println(F("FONA is OK"));
  Serial.print(F("Found"));

  fona.setGPRSNetworkSettings(F("pda.bell.ca"));

  fona.setHTTPSRedirect(true);
}

void loop() {
  int i = 0;
  String randnum = (String)random(999);
//  char randnumstr[3];
//  itoa(randnum, randnumstr, 10);

        
//  char getstr[100] = {0};
//  char getstr1[] = "GET /update?api_key=2TQ3N7QCJFJXF7KM&field2=";
//  char getstr2[] = " HTTP/1.1\r\nHost: api.thingspeak.com\r\n\r\n";
  
//  strcat(getstr, getstr1);
//  strcat(getstr, randnumstr);
//  strcat(getstr, getstr2);

  String GETStr = "GET /ollie/sendData?type=ammonia&id=02&value="+randnum+" HTTP/1.1\r\nHost: www.cas.mcmaster.ca\r\n\r\n";
//  String GETStr = "GET /update?api_key=2TQ3N7QCJFJXF7KM&field2="+randnum+" HTTP/1.1\r\nHost: api.thingspeak.com\r\n\r\n"; 
  char GETStrArr[100];
  GETStr.toCharArray(GETStrArr,sizeof(GETStrArr));  
     
  Serial.print(F("FONA> "));
  while (! Serial.available() ) {
        if (fona.available()) {
            Serial.write(fona.read());
        }
  }

  if(Serial.available()) {
     String usercmd = Serial.readString();
     Serial.print(usercmd);
     if(usercmd=="SEND\r\n"){
        for(i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSSTART"), F("OK"), 30000);i++);
//        for(i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSOPSE=\"api.thingspeak.com\",443,2"), F("OK") ,10000);i++);
        for(i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSOPSE=\"www.cas.mcmaster.ca\",80,1"), F("OK") ,10000);i++);
        for(i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSSEND=120"),F(">"), 10000);i++);
        for(i=0;i<3 && !fona.sendCheckReply(GETStrArr, F("OK"), 10000);i++);
        for(i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSCLSE"),F("OK"),10000);i++);
     }else{
        fona.println(usercmd);
     }
  }
}








