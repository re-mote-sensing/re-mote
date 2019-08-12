/*
Library for using a Fona module (SIM5320), used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last modified on August 12, 2019
Released into the public domain
*/

#ifndef remoteFona_h
#define remoteFona_h

#include <Arduino.h>

class remoteFona {
    public:
        remoteFona();
        bool initialise();
        bool startHTTPS();
        bool post(char* request, const char* host, int portNum);
        void stopHTTPS();
        void offsetTime(long offset);
        bool getGPSData(unsigned long* time, float* lat, float* lon, unsigned long timeout = 0);
    private:
        unsigned long unixTime = 0;
        unsigned long timeLastUpdated = 0;
        float latitude = 0.0;
        float longitude = 0.0;
        bool toggle(bool on = false);
        bool checkFona(Stream& port);
        void closeHTTPS(Stream& port);
        bool SCRTries(Stream& port, const __FlashStringHelper* command, const char* reply, unsigned long timeout, uint8_t tries = 3);
        bool SCRTries(Stream& port, const char* command, const char* reply, unsigned long timeout, uint8_t tries = 3);
        bool sendCheckReply(Stream& port, const __FlashStringHelper* command, const char* reply, unsigned long timeout);
        bool sendCheckReply(Stream& port, const char* command, const char* reply, unsigned long timeout);
        bool sendGetReply(Stream& port, const __FlashStringHelper* command, unsigned long timeout, char* ans, uint8_t length);
        unsigned long getUnixTime(char* str);
};

#endif