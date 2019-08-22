// You have to hold down RST on LoRa+GPS Shields when uploading or you will get init failed

/*-------------------------INCLUDES-----------------------------*/
#include <SPI.h>

#include <RHReliableDatagram.h>
#include <RH_RF95.h>

/*----------------------DEFINITIONS-----------------------------*/

#define SERVER_ADDRESS 1
#define DO_ADDRESS 3 						// Make sure you're defining the correct address on server side and client side!!

/*-------------------------GLOBALS------------------------------*/

char dataArr[7];						// Data array currently only holds 7 bytes. Change if sending more information than just the value read 

/*---------------------LORA CONSTRUCTORS------------------------*/

RH_RF95 driver;

RHReliableDatagram LORAmanager(driver, DO_ADDRESS);		// Use reliableDatagram for addressed packet delivery

/*---------------------------SETUP------------------------------*/

void setup() {
    while(!Serial);

    Serial.begin(9600);
    if (!LORAmanager.init()){
        Serial.println("init failed");
    } else {
        Serial.println("init succeeded");
    }
}

void loop() {
    
    double data = (random(999))/100.0;
//    itoa(data,dataArr,10);
    dtostrf(data,1,2,dataArr);

    if (LORAmanager.sendtoWait(dataArr, sizeof(dataArr), SERVER_ADDRESS)) {
        Serial.print("sent: ");
        Serial.println(dataArr);
    } 
    
    delay(20000);						// Simulating collecting data every 20 seconds
}

