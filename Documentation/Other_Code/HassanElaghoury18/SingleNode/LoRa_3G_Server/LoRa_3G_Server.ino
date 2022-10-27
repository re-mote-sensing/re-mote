// Make sure to plug in Server BEFORE client
/*-------------------------INCLUDES-----------------------------*/

#include <SoftwareSerial.h>
#include <SPI.h>

#include "Adafruit_FONA.h"
#include "RHReliableDatagram.h"
#include "RH_RF95.h"

/*----------------------DEFINITIONS-----------------------------*/

#define FONA_TX 4							// LoRa shield only leaves pins 0,1,3,4,5 and DIO pins 0,1,2,5 unused. Careful with which arduino pins the DIO pins map to
#define FONA_RX 5
#define FONA_RST 9							// Reset pins on LoRa shield and 3G shield overlap
#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2						// Server/Client addresses are static and manually chosen. Must agree on all LoRa devices

/*-------------------------GLOBALS------------------------------*/

uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

/*----------------------3G CONSTRUCTORS-------------------------*/

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX,FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

/*---------------------LORA CONSTRUCTORS------------------------*/

RH_RF95 driver;

RHReliableDatagram LORAmanager(driver, SERVER_ADDRESS);			// reliableDatagram allows for addressed packet delivery

/*---------------------------SETUP------------------------------*/

void setup() {
    while(!Serial);

    Serial.begin(9600);

    FONAsetup();
    
    if (!LORAmanager.init()){
        Serial.println("init failed");
    } else {
        Serial.println("init succeeded");
    } 
}

void FONAsetup() {
    int i;
    
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

    delay(3000);

//    for(i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSSTART"), F("OK"), 10000);i++);
}

void FONASendHTTP() {
    int i=0;
    
// Uncomment depending on which database you're sending to.
    char getstr[130] = {0};
//    char getstr1[] = "GET /update?api_key=2TQ3N7QCJFJXF7KM&field2=";
    char getstr1[] = "GET /ollie/sendData?numData=1&type1=EC&id1=02&value1=";
//    char getstr2[] = " HTTP/1.1\r\nHost: api.thingspeak.com\r\n\r\n";
    char getstr2[] = " HTTP/1.1\r\nHost: www.cas.mcmaster.ca\r\n\r\n";

    strcat(getstr, getstr1);
    strcat(getstr, (char*)buf);
    strcat(getstr, getstr2);
    
    for(i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSSTART"), F("OK"), 10000);i++);				// Acquires HTTPS protocol stack
//    for(i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSOPSE=\"api.thingspeak.com\",443,2"), F("OK") ,10000);i++);
    for(i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSOPSE=\"www.cas.mcmaster.ca\",443,2"), F("OK") ,10000);i++);		//Opens HTTPS session with the specified host
    for(i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSSEND=130"),F(">"), 10000);i++);			// Specifies the maximum length of the GET request to be sent
    for(i=0;i<3 && !fona.sendCheckReply(getstr, F("OK"), 10000);i++);				// Sends the GET request
    for(i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSCLSE"),F("OK"),10000);i++);			// Close HTTPS session with the specified host
}

void loop() {
    
    Serial.println("Check if LoRa received");
    LORAmanager.waitAvailable();					// Waits until the LoRa receiver gets a packet
    if(LORAmanager.available()) {
            uint8_t len = sizeof(buf);
            uint8_t from;
            if (LORAmanager.recvfromAck(buf, &len, &from))
            {
                delay(20000);						// The 3G shield only works with this 20 second delay. Not sure why 20 seconds is th magic number, but it seems to break when a shorter delay is used
                Serial.print("Got data: ");
                Serial.println((char*)buf);
                Serial.println("Intiating HTTP request: ");
                FONASendHTTP();
            }
    }
}
