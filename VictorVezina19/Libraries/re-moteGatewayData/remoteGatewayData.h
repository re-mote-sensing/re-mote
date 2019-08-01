/*
Library for saving the data of a gateway, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated August 1, 2019
Released into the public domain
*/

/* To do:
- Add other kinds of data storage options (EEPROM)
*/

#ifndef remoteGatewayData_h
#define remoteGatewayData_h

#include <Arduino.h>

#if Data_Type == SD_Type
#include <SdFat.h>
#endif

class remoteGatewayData {
    public:
        remoteGatewayData();
        void initialise();
        void reset(bool hard);
        uint8_t saveRegistration(uint8_t* data);
        uint8_t saveData(uint8_t* data);
        char* getPost(unsigned int arg);
        void messageSuccess();
    
    private:
        /*----------SD Card----------*/
        #if Data_Type == SD_Type
            uint8_t loops = 0;
            void initialiseSD();
            void resetSD(bool hard);
            uint8_t saveRegSD(uint8_t* data);
            uint8_t saveDataSD(uint8_t* data);
            uint8_t filePrintFloat(SdFile* file, uint8_t* arr, uint8_t num);
            uint32_t fileReadInt(SdFile* file, char end = ',', int8_t move = 1);
            char* getPostSD(unsigned int numLoops);
            unsigned int getSendInfoSize(unsigned int numLoops);
            bool getToSendData(SdFile* file, unsigned int numLoops, uint8_t* numNodes = NULL, uint8_t* lastNum = NULL);
            bool getSendInfo(uint8_t* ans, unsigned int numLoops);
            uint8_t getNumberLocations(SdFile* file, uint8_t numData, unsigned long position);
            unsigned int getDataSize(uint8_t* sendInfo);
            bool getTypesInfo(uint8_t* typesInfo, uint16_t id);
            void skipConstTypes(SdFile* file);
            bool buildRequest(char* request, uint8_t* sendInfo);
            bool getNodeData(char* request, uint16_t* curr, uint16_t id, uint8_t numData, uint8_t locations, uint32_t position);
            float fileReadFloat(SdFile* file, char end = ',', uint8_t bufferSize = 15);
            void messageSuccessSD();
        #endif
    
        /*----------General----------*/
        uint32_t pow10(uint8_t pow);
        void strPrintShort(char* str, uint16_t data);
        void strPrintByte(char* str, uint8_t data);
        void strPrintLongP(char* str, uint32_t* data);
    
    
        void printMem();
};

#endif
