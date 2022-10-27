// You have to hold down RST on LoRa+GPS Shields when uploading or yo uwill get init failed


/*-------------------------INCLUDES-----------------------------*/
#include <SPI.h>
#include <SoftwareSerial.h>

#include <RHReliableDatagram.h>
#include <RH_RF95.h>

/*----------------------DEFINITIONS-----------------------------*/

#define SERVER_ADDRESS 1
#define EC_ADDRESS 2 						// Make sure you're defining the correct address on server side and client side!!
#define SENSOR_RX 5						// Avoid pin conflicts. LoRa shield leaves pin 0,1,3,4,5 unused
#define SENSOR_TX 4

/*-------------------------GLOBALS------------------------------*/

//String sensorString = "";
byte dataIndex = 0;
char buf[48];
char dataArr[48];
double data;
boolean sensorStringComplete = false;

/*--------------------SENSOR CONSTRUCTORS-----------------------*/

SoftwareSerial sensorSerial(SENSOR_RX, SENSOR_TX);

/*---------------------LORA CONSTRUCTORS------------------------*/

RH_RF95 driver;

RHReliableDatagram LORAmanager(driver, EC_ADDRESS);		// Use reliableDatagram for addressed packet delivery

/*---------------------------SETUP------------------------------*/

void setup() {
    while (!Serial);

    sensorSerial.begin(9600);

    Serial.begin(9600);

    if (!LORAmanager.init()) {
        Serial.println("init failed");
    } else {
        Serial.println("init succeeded");
    }

}

void loop() {

    sensorSerial.print("R\r");					// Command to read value from Atlas Scientific Sensors

    while (sensorSerial.available() > 0 && !sensorStringComplete) {
        dataIndex = sensorSerial.readBytesUntil(13, buf, 48);
        dataArr[dataIndex] = 0;
        if (dataIndex != 0) sensorStringComplete = true;
        data = atof(buf);					// The conversion to float and then back to character array was added to try to solve the problem of
								// sensor values causing the GET request to stop working
								// (so that this client follows the same procedure the Randnum client follows). However this did not solve the problem. 
    }

    while (sensorStringComplete && ((buf[0] >= 48) && (buf[0] <= 57))) {
        dtostrf(data, 1, 2, dataArr);
        if (LORAmanager.sendtoWait(dataArr, sizeof(dataArr), SERVER_ADDRESS)) {
            sensorStringComplete = false;
        }
    }

    delay(20000);						// Added to read sensor values every 20 seconds

}











