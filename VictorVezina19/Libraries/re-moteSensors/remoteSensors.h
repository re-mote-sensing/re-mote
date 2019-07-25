/*
Library for reading from various sensors, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated July 25, 2019
Released into the public domain
*/

/* To do:
- Add support for EC enable pin
- Add temperature and salinity compensation to Atlas Scientific sensors
*/

#ifndef remoteSensors_h
#define remoteSensors_h

#include <Arduino.h>

class remoteSensors {
    public:
        remoteSensors();
        void initialise();
        void read(uint8_t* dataArr);
    private:
        //Initialisation functions
        void initialiseASDO(uint8_t index);
        void initialiseASEC(uint8_t index);
        void initialiseAtlas(Stream& sensor);
        void initialiseDFTB(uint8_t index);
        void initialiseDFTemp(uint8_t index);
        void initialiseDHT22(uint8_t index);
    
        //Reading functions
        uint8_t readASDO(uint8_t index, uint8_t* data);
        uint8_t readASEC(uint8_t index, uint8_t* data);
        float readAtlas(Stream& sensor);
        uint8_t readDFTB(uint8_t index, uint8_t* data);
        uint8_t readDFTemp(uint8_t index, uint8_t* data);
        uint8_t readDHT22(uint8_t index, uint8_t* data);
};

#endif