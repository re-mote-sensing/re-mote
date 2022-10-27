/*
Library for saving the data of an end node, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated July 25, 2019
Released into the public domain
*/

/* To do:
- Add other kinds of data storage options (SD card)
*/

#include <Arduino.h>

#ifndef remoteEndData_h
#define remoteEndData_h

class remoteEndData {
    public:
        remoteEndData();
        void initialise();
        void reset(bool hard);
        void saveData(uint8_t* data);
        uint8_t* getDataMessage();
        void messageSuccess();
    private:
        //Variables
        unsigned int lastAdd = 3;
    
        //EEPROM functions
        void initialiseEEPROM();
        void resetEEPROM(bool hard);
        void saveEEPROM(uint8_t* data);
        uint8_t* getEEPROMMessage();
        void getEEPROMMessageInfo(unsigned int validToAddress, unsigned int validFromAddress, uint8_t* messageSize, uint8_t* numLocations);
        void readEEPROM(uint8_t* message, uint8_t* messageIndex, unsigned int* memAddress, uint8_t numBytes);
        void EEPROMSuccess();
};

#endif
