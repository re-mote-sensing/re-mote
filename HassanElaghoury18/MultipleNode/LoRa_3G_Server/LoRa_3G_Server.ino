// You have to hold down RST on LoRa+GPS Shields when uploading or you will get init failed
/*-------------------------INCLUDES-----------------------------*/

#include <SoftwareSerial.h>
#include <SPI.h>

#include "Adafruit_FONA.h"
#include "RHReliableDatagram.h"
#include "RH_RF95.h"

/*----------------------DEFINITIONS-----------------------------*/

#define FONA_TX 4									// LoRa shield only leaves pins 0,1,3,4,5 unused in the digital pins, so careful to choose pins so there is no pin conflict
#define FONA_RX 5
#define FONA_RST 9									// Okay that reset pins on LoRa shield and 3G shield overlap
#define SERVER_ADDRESS 1
#define EC_ADDRESS 2									// Server/Client addresses are static and manually chosen. Must agree on all LoRa devices
#define DO_ADDRESS 3
#define MAX_VAL_LENGTH 10								// Only 10 bytes currently allocated for data sent from sensors. Add more if necessary

/*-------------------------GLOBALS------------------------------*/

uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
char ECVal[MAX_VAL_LENGTH];
char DOVal[MAX_VAL_LENGTH];
boolean ECComplete = false;
boolean DOComplete = false;

/*----------------------3G CONSTRUCTORS-------------------------*/

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

/*---------------------LORA CONSTRUCTORS------------------------*/

RH_RF95 driver;

RHReliableDatagram LORAmanager(driver, SERVER_ADDRESS);					// ReliableDatagram should be used for addressed packet delivery between LoRa devices

/*---------------------------SETUP------------------------------*/

void setup() {
    while (!Serial);

    Serial.begin(9600);

    FONAsetup();

    if (!LORAmanager.init()) {
        Serial.println("init failed");
    } else {
        Serial.println("init succeeded");
    }
}

void FONAsetup() {
    int i;

    fonaSerial->begin(4800);
    if (! fona.begin(*fonaSerial)) {
        Serial.println(F("Couldn't find FONA"));
        while (1);
    }

    Serial.println(F("Found"));

    fona.setGPRSNetworkSettings(F("pda.bell.ca"));

    fona.setHTTPSRedirect(true);
}

// The FONASendHTTP function does not currently work when trying to send sensor values (floats represented as char arrays) from multiple sensors. 
// However, it work when only using one sensor, and it works when using randomly generated float values being sent from the LoRa clients. 
// Checked that the issue was not with unwanted character terminators in the sensor. It is unclear why sending the character array of sensor values breaks this request. 
// We get the OK after sending the GET request to the server but the server does not actually get the request

void FONASendHTTP() {
    int i = 0;
    int getSize = 0;

    char getstr[130] = {0};                                // For some reason, altering size of the array breaks the program (no matter if the array is bigger or smaller)
    sprintf(getstr, "GET /update?api_key=2TQ3N7QCJFJXF7KM&field2=%s&field3=%s HTTP/1.1\r\nHost: api.thingspeak.com\r\n\r\n", (char*)ECVal, (char*)DOVal);
//    sprintf(getstr,"GET /ollie/sendData?numData=2&type1=EC&id1=02&value1=%s&type2=DO&id2=02&value2=%s HTTP/1.1\r\nHost: www.cas.mcmaster.ca\r\n\r\n", (char*)ECVal, (char*)DOVal);
                                            // GET request format for Thingspeak as well as the McMaster database

//    if (! fona.postData("www.cas.mcmaster.ca",80,"HTTP",getstr))       // Uses new postData function from FONA library to send the corrrect AT commands and make http requests. 
//        Serial.println("postData failed");                             // postData(host, port, connection type, GET string)
    if (! fona.postData("api.thingspeak.com",443,"HTTPS",getstr))
        Serial.println("postData failed");
}

void loop() {

    while (!(ECComplete && DOComplete)) {                                   // Using !(ECComplete && DOComplete) so that nothing to do with 3G is done if LoRa transmission incomplete
        Serial.println("Waiting for LoRa");
        LORAmanager.waitAvailable();					    // Waits until the LoRa manager receives a packet from any node
        if (LORAmanager.available()) {
            uint8_t len = sizeof(buf);
            uint8_t from;
            if (LORAmanager.recvfromAck(buf, &len, &from))
            {
                if (from == EC_ADDRESS) {
                    Serial.print("Got EC: ");
                    Serial.println((char*)buf);
                    strcpy(ECVal, buf);
                    ECComplete = true;
                }

                else if (from == DO_ADDRESS) {
                    Serial.print("Got DO: ");
                    Serial.println((char*)buf);
                    strcpy(DOVal, buf);
                    DOComplete = true;
                }
            }
        }
    }

    if (ECComplete && DOComplete) {					   // Only send when data is received from both nodes, so that we don't make too many time-consuming HTTP requests. However this is not a good method when you have more than just a few nodes, as the requests WILL overlap
        Serial.println("EC and DO complete");
        delay(20000);							   // The 3G shield only works with this 20 second delay. I'm not sure why 20 seconds is the magic number, but it seems to break when a shorter delay is used
        FONASendHTTP();
        ECComplete = false;
        DOComplete = false;
    }
}
