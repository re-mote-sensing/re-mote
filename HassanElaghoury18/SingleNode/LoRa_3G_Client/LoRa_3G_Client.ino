// You have to hold down RST on LoRa+GPS Shields when uploading or yo uwill get init failed


/*-------------------------INCLUDES-----------------------------*/
#include <SPI.h>
#include <SoftwareSerial.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>

/*----------------------DEFINITIONS-----------------------------*/

#define CLIENT_ADDRESS 1					// Make sure you're defining the correct address on server and client sides
#define SERVER_ADDRESS 2
#define SENSOR_RX 5						// Avoid pin conflicts. LoRa shield leaves pin 0,1,3,4,5 and DIO pins 0,1,2,5 unused
#define SENSOR_TX 4

/*-------------------------GLOBALS------------------------------*/

char dataArr[10]; 
String sensorString = "";
boolean sensorStringComplete = false;

/*---------------------SENSOR CONSTRUCTORS------------------------*/

SoftwareSerial sensorSerial(SENSOR_RX, SENSOR_TX);

/*---------------------LORA CONSTRUCTORS------------------------*/

RH_RF95 driver;

RHReliableDatagram LORAmanager(driver, CLIENT_ADDRESS);		// Use reliableDatagram for addressed packet delivery

/*---------------------------SETUP------------------------------*/

void setup() {
    while(!Serial);

    sensorSerial.begin(9600);

    Serial.begin(9600);
    if (!LORAmanager.init()){
        Serial.println("init failed");
    } else {
        Serial.println("init succeeded");
    }
}

void loop() {

    sensorSerial.print("R\r");					// Command to read value from Atlas Scientific sensors

    while (sensorSerial.available() > 0 && !sensorStringComplete) {
        char inchar = (char)sensorSerial.read();
        sensorString += inchar;
        Serial.println(inchar);
        if (inchar == '\r') {
            sensorStringComplete = true;
        }
    }

    if(sensorStringComplete && isdigit(sensorString[0])) {
        sensorString.toCharArray(dataArr, sizeof(dataArr));
        if (LORAmanager.sendtoWait(dataArr, sizeof(dataArr), SERVER_ADDRESS)) {
            Serial.print("Sending: ");
            Serial.println(dataArr);
            sensorStringComplete = false;
            sensorString = "";
        } 
    }
    
    delay(20000);i						// Added to read sensor values every 20 seconds
}











