/*
Library for using a LoRa mesh module (https://www.dfrobot.com/product-1670.html), used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Spencer Park, Ryan Tyrell, and Victor Vezina, last modified on July 12, 2019
Released into the public domain
*/

#ifndef remoteLoRa_h
#define remoteLoRa_h

#include <Arduino.h>

#define PORT_READ_TIMEOUT 5000

class remoteLoRa {
    public:
        remoteLoRa(uint8_t RX, uint8_t TX);
        void readConfig();
        void writeConfig(uint16_t netID, uint16_t nodeID);
        uint8_t* sendReceive(uint16_t target, uint8_t dataLen, uint8_t* data, unsigned long timeout = 5000);
        uint8_t* readData();
        void sendData(uint16_t target, uint8_t dataLen, uint8_t* data);
    private:
        uint8_t rx;
        uint8_t tx;
        uint8_t* readData(Stream& port);
        void sendData(Stream& port, uint16_t target, uint8_t dataLen, uint8_t* data);
        int readFrame(Stream& port, byte* rFrameType, byte* rCmdType, byte** rPayload);
        void writeFrame(Stream& port, uint8_t frameType, uint8_t cmdType, uint8_t payloadLen, uint8_t* payload);
        uint8_t readByte(Stream& port, bool* error);
        void printByte(uint8_t b);
        void printShort(uint16_t s);
};

#endif