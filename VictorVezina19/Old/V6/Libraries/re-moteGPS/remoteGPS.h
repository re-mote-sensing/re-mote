/*
Library for reading from a GPS, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated July 11, 2019
Released into the public domain
*/

/* To do:
- Add other kinds of GPS support (notably FONA GPS)
- Add support to use built-in RTC
*/

#include <Arduino.h>

#ifndef remoteGPS_h
#define remoteGPS_h

class remoteGPS {
    public:
        remoteGPS(uint8_t RX, uint8_t TX, uint8_t EN);
        void initialise();
        void offsetTime(long offset);
        void getData(unsigned long* time, float* lat, float* lon, unsigned long timeout = 0);
    private:
        uint8_t rx;
        uint8_t tx;
        uint8_t enable;
        float latitude = 0.0;
        float longitude = 0.0;
        unsigned long unixTime = 0;
        unsigned long timeLastUpdated = 0;
        long timeOffset = 0;
};

#endif
