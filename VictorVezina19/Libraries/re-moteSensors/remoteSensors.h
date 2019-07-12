/*
Library for reading from various sensors, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated July 9, 2019
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
        remoteSensors(int numberSensors, char* sensorTypes[], uint8_t sensorPorts[][2]);
        void initialise();
        void read(uint8_t* dataArr);
    private:
        //Variables
        uint8_t _numberSensors;
        char** _sensorTypes;
        uint8_t (*_sensorPorts)[2];
    
        //Initialisation functions
        void initialiseDO(uint8_t index);
        void initialiseConductivity(uint8_t index);
        void initialiseAtlas(Stream& sensor);
        void initialiseTurbidity(uint8_t index);
        void initialiseWaterTemp(uint8_t index);
        void initialiseDHT22(uint8_t index);
    
        //Reading functions
        float readDO(uint8_t index);
        float readConductivity(uint8_t index);
        float readAtlas(Stream& sensor);
        float readTurbidity(uint8_t index);
        float readWaterTemp(uint8_t index);
        float readAirTemp(uint8_t index);
        float readHumidity(uint8_t index);
};

#endif