/*
Library for sleeping, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated July 12, 2019
Released into the public domain
*/

#ifndef remoteSleep_h
#define remoteSleep_h

#include <Arduino.h>

class remoteSleep {
    public:
        remoteSleep(uint8_t inputType);
        long sleep(unsigned long time, unsigned long start);
    private:
        //Variables
        uint8_t type;
    
        //Sleep functions
        void sleep8000();
        void sleep4000();
        void sleep2000();
        void sleep1000();
        void sleep500();
        void sleep250();
        void sleep125();
        void sleep64();
        void sleep32();
        void sleep16();
};

#endif