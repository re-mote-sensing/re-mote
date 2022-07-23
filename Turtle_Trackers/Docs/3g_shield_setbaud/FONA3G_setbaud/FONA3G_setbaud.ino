/*************************************************** 
  This is an example for our Adafruit FONA Cellular Module
  since the FONA 3G does not do auto-baud very well, this demo 
  fixes the baud rate to 4800 from the default 9600
  
  Designed specifically to work with the Adafruit FONA 3G
  ----> http://www.adafruit.com/products/2691
  ----> http://www.adafruit.com/products/2687

  These cellular modules use TTL Serial to communicate, 2 pins are 
  required to interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_FONA.h"

#define FONA_TX 4
#define FONA_RX 5
#define FONA_RST A0

// this is a large buffer for replies
char replybuffer[255];

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX,FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Hardware serial is also possible!
//  HardwareSerial *fonaSerial = &Serial1;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

#define SIM3G_EN A1
#define SIM3G_POWER_ON_TIME 180       // 3G Power on time for enable pin (ms)

void setup() {
  while (!Serial);

  pinMode(SIM3G_EN, OUTPUT);
  delay(1000);
  digitalWrite(SIM3G_EN, HIGH);
  delay(SIM3G_POWER_ON_TIME);
  digitalWrite(SIM3G_EN, LOW);
  delay(8000);

  Serial.begin(9600);
  Serial.println(F("FONA set baudrate"));

  Serial.println(F("First trying 4800 baud"));
  // start at 4800 baud
  fonaSerial->begin(4800);
  fona.begin(*fonaSerial);
  
  // send the command to reset the baud rate to 9600
  fona.setBaudrate(9600); 
  
  // restart with 9600 baud
  fonaSerial->begin(9600);
  Serial.println(F("Initializing @ 9600 baud..."));
  
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while(1);
  }
  Serial.println(F("FONA is OK"));

  // Print module IMEI number.
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

}

void loop() {
}
