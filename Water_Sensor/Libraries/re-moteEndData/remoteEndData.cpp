/*
Library for saving the data of an end node, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated August 12, 2019
Released into the public domain
*/

#include "remoteEndData.h"
#include <EEPROM.h>
#include <remoteConfig.h>

//Start the data saving method
remoteEndData::remoteEndData() {
}

//Initialise the data storage
void remoteEndData::initialise() {
    #if Data_Type == EEPROM_Type
    initialiseEEPROM();
    #endif
}

//Reset the data storage
void remoteEndData::reset(bool hard) {
    #if Data_Type == EEPROM_Type
    resetEEPROM(hard);
    #endif
}

//Save some data to the storage
void remoteEndData::saveData(uint8_t* data) {
    #if Data_Type == EEPROM_Type
    saveEEPROM(data);
    #endif
}

//Get a LoRa data message
uint8_t* remoteEndData::getDataMessage() {
    #if Data_Type == EEPROM_Type
    return getEEPROMMessage();
    #endif
    return NULL;
}

//Set the data according to a successful message sent
void remoteEndData::messageSuccess() {
    #if Data_Type == EEPROM_Type
    EEPROMSuccess();
    #endif
}

/*--------------------------PRIVATE--------------------------*/

#if Data_Type == EEPROM_Type
/*--------------------------EEPROM---------------------------*/
//Initialise the EEPROM
void remoteEndData::initialiseEEPROM() {
    //Get the valuse at the begining of the EEPROM
    unsigned int initialCheck = 0;
    EEPROM.get(0, initialCheck);
    
    //Check to see if that value makes sense or if the EEPROM has never been used
    if (initialCheck >= EEPROM.length()) {
        //Put the default values in
        unsigned int address = 4;
        EEPROM.put(0, address);
        EEPROM.put(2, address - 1);
    }
}

//Reset the EEPROM
void remoteEndData::resetEEPROM(bool hard) {
    //Set the default values at the begining of the EEPROM
    unsigned int i = 4;
    EEPROM.put(0, i);
    EEPROM.put(2, i - 1);
    
    //If it's a hard reset
    if (hard) {
        //Go through every memory address in the EEPROM and set it to 255
        for (unsigned int i = 5; i < EEPROM.length(); i++) {
            EEPROM.update(i, 255);
        }
    }
}

//Save a data point to EEPROM
void remoteEndData::saveEEPROM(uint8_t* data) {
    unsigned int currAddress = 4;
    EEPROM.get(0, currAddress); //Get the current EEPROM address of the last saved data point
    
    unsigned int validToAddress = 3;
    EEPROM.get(2, validToAddress); //Get the current EEPROM address where valid data starts
    
    bool loop = currAddress <= validToAddress; //If the data saving has looped the EEPROM memory
    bool doubleLoop = false; //If the data has looped the EEPROM memory twice
    
    if (validToAddress == 3) { //This is needed for the weird case of the default state
        validToAddress++;
    }
    
    uint8_t size = 1 + (data[0] * 8) + (4 * (NUMBER_SENSOR_NAMES + 1)); //Get the size of the data array
    
    //Loop through each byte of the current value
    for (uint8_t i = 0; i < size; i++) {
        EEPROM.update(currAddress++, data[i]); //Write current byte

        //Check for looping/double looping in the EEPROM
        if (currAddress == EEPROM.length()) {
            currAddress = 4;
            doubleLoop = loop;
            loop = true;
        }
    }
    
    EEPROM.put(0, currAddress); //Change address of last saved data point
    
    //If the address where valid data starts has changed, put that change into the EEPROM
    if (doubleLoop || (loop && (currAddress > validToAddress))) {
        EEPROM.put(2, validToAddress + (4 * (NUMBER_SENSOR_NAMES + 1)));
    } else {
        EEPROM.put(2, validToAddress);
    }
}

//Get a LoRa data message from the EEPROM
uint8_t* remoteEndData::getEEPROMMessage() {
    //Get EEPROM address of last data point
    unsigned int validToAddress = 4;
    EEPROM.get(0, validToAddress);
    
    //Get EEPROM address of where valid data starts
    unsigned int currAddress = 3;
    EEPROM.get(2, currAddress);
    
    if (currAddress == 3) { //Default state, means no data
        return NULL;
    }
    
    //Get info for making LoRa data message
    uint8_t messageSize = 0;
    uint8_t numLocations = 0;
    getEEPROMMessageInfo(validToAddress, currAddress, &messageSize, &numLocations);
    
    uint8_t* dataArr = (uint8_t*) malloc(sizeof(uint8_t) * (messageSize + 1)); //Allocate the array for the LoRa message
    
    dataArr[0] = messageSize;
    dataArr[1] = 0b00010000 | ((uint8_t) NUMBER_SENSOR_NAMES & 0b00001111); //Set the first byte of the message; the message type and the number of sensors
    dataArr[2] = numLocations; //Number of locations
    
    //Variables for keeping track of important message information
    uint8_t currLocPtr = 3;
    uint8_t currDataPtr = 3 + numLocations;
    uint8_t currDataNum = 0;
    
    #ifdef DEBUG
    //Serial.println(F("Reading from EEPROM:"));
    #endif
    
    //Read data from EEPROM into the LoRa message
    while (currDataPtr < messageSize) {
        currDataNum++; //Reading new data point
        
        //Read has location byte
        uint8_t hasLoc;
        uint8_t temp = 0;
        readEEPROM(&hasLoc, &temp, &currAddress, 1);
        
        readEEPROM(dataArr, &currDataPtr, &currAddress, 4); //Read time
        
        if (hasLoc) {
            readEEPROM(dataArr, &currDataPtr, &currAddress, 8); //Read location
            dataArr[currLocPtr++] = currDataNum; //Put data point number into locations part of message
        }
        
        readEEPROM(dataArr, &currDataPtr, &currAddress, 4 * NUMBER_SENSOR_NAMES); //Read sensor data
    }
    
    #ifdef DEBUG
    Serial.println();
    Serial.println();
    #endif

    lastAdd = currAddress;
    
    return dataArr;
}

//Gets the info of a data LoRa message from EEPROM
void remoteEndData::getEEPROMMessageInfo(unsigned int validToAddress, unsigned int validFromAddress, uint8_t* messageSize, uint8_t* numLocations){
    unsigned int curr = validFromAddress;
    (*messageSize) = 2;
    uint8_t lastSize; //Last valid size
    uint8_t lastLocations; //Number of locations at last valid size

    //Go through data to find max size that can be sent
    while ((*messageSize) <= 111 && curr != validToAddress) {
        lastSize = (*messageSize);
        lastLocations = (*numLocations);

        //Has location byte
        uint8_t hasLocation = EEPROM.read(curr++);

        //Time
        curr += 4;
        (*messageSize) += 4;

        if (hasLocation) {
            //Location
            curr += 8;
            (*messageSize) += 9;

            (*numLocations)++; //Number of locations counter
        }

        //Sensor data
        curr += 4 * NUMBER_SENSOR_NAMES;
        (*messageSize) += 4 * NUMBER_SENSOR_NAMES;

        if (curr >= EEPROM.length()) { //EEPROM loop check
            curr -= (EEPROM.length() - 4);
        }
    }

    if ((*messageSize) > 111) { //Check what caused loop exit
        (*messageSize) = lastSize;
        (*numLocations) = lastLocations;
    }
}

//Read numBytes from EEPROM starting at *memAddress into message starting at *messageIndex, incrementing everything accordingly and checking for EEPROM looping
void remoteEndData::readEEPROM(uint8_t* message, uint8_t* messageIndex, unsigned int* memAddress, uint8_t numBytes) {
    //Loop through the number of bytes provided
    for (uint8_t i = 0; i < numBytes; i++) {
        //Read the EEPROM and put the value into the message
        message[(*messageIndex)++] = EEPROM.read((*memAddress)++);
        
        #ifdef DEBUG
        //Serial.print(EEPROM.read((*memAddress) - 1));
        //Serial.print(F(" "));
        #endif

        //Check if the address of the EEPROM has looped around
        if (*memAddress > EEPROM.length()) {
            *memAddress -= (EEPROM.length() - 4);
        }
    }
}

//Set the EEPROM according to a successful message sent
void remoteEndData::EEPROMSuccess() {
    #ifdef DEBUG
    Serial.println(F("EEPROM Success"));
    #endif
    
    //Get EEPROM address of last data point
    unsigned int validToAddress = 4;
    EEPROM.get(0, validToAddress);
    
    if (lastAdd == validToAddress) { //If all valid data has been read, reset the addresses
        resetEEPROM(false);
    } else {
        EEPROM.put(2, lastAdd); //Set the address of the last data point correctly
    }
}
#endif