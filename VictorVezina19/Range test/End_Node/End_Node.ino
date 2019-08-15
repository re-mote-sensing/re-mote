
/*--------------------------------------------------------------*/
/*-----------------------------CODE-----------------------------*/
/*--------------------------------------------------------------*/

/*---------------------------INCLUDES---------------------------*/
#define MAIN
#include <remoteConfig.h>
#undef MAIN

//Check to make sure compilation is in the right mode
#ifndef End_Node
#error Please put the config file into End_Node mode
#endif

#include <remoteLoRa.h>

/*-------------------------CONSTRUCTORS-------------------------*/

remoteLoRa LoRa;

/*---------------------------SETUP------------------------------*/

void setup() {
    #ifdef DEBUG
    Serial.begin(9600); //Begin usb serial for debug print statements
    #endif
    
    #ifdef DEBUG
    Serial.println(F("Initialising LoRa"));
    #endif
    //Write the config parameters to the LoRa module
    while (!LoRa.writeConfig(NETWORK_ID, 0x0003)) {
        delay(2500); //If it fails wait 2.5 seconds then try again
    }
    
    #ifdef DEBUG
    //Serial.println(F("Waiting for input..."));
    //while (!Serial.available()) ; //Useful for testing
    #endif
    
    Serial.println(F("Entering waiting for LoRa"));
}


/*----------------------------LOOP------------------------------*/

void loop() {
    uint8_t* data = LoRa.readData();
    
    if (data != NULL) {
        Serial.println(F("Received message"));
        //The data of the acknowledgement
        uint8_t* ackData = (uint8_t*) malloc(sizeof(uint8_t));
        ackData[0] = 0;

        //Send the acknowledgement
        LoRa.sendData(GATEWAY_ID, 1, ackData);

        free(ackData);
        
        free(data);
    }
}
