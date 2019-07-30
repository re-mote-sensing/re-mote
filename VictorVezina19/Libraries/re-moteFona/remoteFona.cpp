/*
Library for using a Fona module (SIM5320), used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last modified on July 30, 2019
Released into the public domain
*/

#include "remoteFona.h"
#include <remoteConfig.h>
#include <NeoSWSerial.h>
#include <time.h>

#ifdef DEBUG
#include <MemoryFree.h>
#endif

//Start the Fona
remoteFona::remoteFona() {
    pinMode(FONA_EN, OUTPUT);
}

//Initialise the Fona module
void remoteFona::initialise() {
    #if Fona_Make == Tinysine
    //Turn on the 3G chip on the Tinysine Fona
    digitalWrite(FONA_EN, HIGH);
    delay(180);
    digitalWrite(FONA_EN, LOW);
    
    #elif Fona_Make == Adafruit
    //Set the key pin to the default, the 3G chip will stay off
    digitalWrite(FONA_EN, HIGH);
    
    #endif
}

//Post a given HTTP request through the Fona
bool remoteFona::post(char* request, const char* host, int portNum) {
    #if Fona_Make == Adafruit
    toggle();
    #endif
    
    //Initialise the FONA software serial
    NeoSWSerial fonaSS(FONA_RX, FONA_TX);
    fonaSS.begin(9600);
    
    //First send an AT command to make sure the FONA is working
    bool err = checkFona(fonaSS);
    
    closeHTTPS(fonaSS); //Make sure there isn't an active HTTPS session
    
    //Send HTTP start command
    if (!err) err = SCRTries(fonaSS, "AT+CHTTPSSTART", "OK", 10000);
    
    delay(500);
    
    //Build HTTP open session command with the defined url and port
    char* httpsArr = (char*) malloc(sizeof(char) * (20 + strlen(host) + 5));
    if (httpsArr == NULL) {
        err = true;
    }
    sprintf(httpsArr, "AT+CHTTPSOPSE=\"%s\",%d,1", host, portNum);
    
    //Send HTTP open session command
    if (!err) err = SCRTries(fonaSS, httpsArr, "OK", 10000);

    free(httpsArr);
    
    
    //Build HTTP send command with size of given request
    httpsArr = (char*) malloc(sizeof(char) * 20);
    if (httpsArr == NULL) {
        err = true;
    }
    sprintf(httpsArr, "AT+CHTTPSSEND=%d", strlen(request));
    
    //Send HTTP send command
    if (!err) err = SCRTries(fonaSS, httpsArr, ">", 20000);
    
    free(httpsArr);
    
    
    //Send HTTP request
    if (!err) err = SCRTries(fonaSS, request, "OK", 10000);
    
    closeHTTPS(fonaSS);
    
    fonaSS.end();
    
    #if Fona_Make == Adafruit
    toggle();
    #endif
    
    return err; //Return if there was an error or not
}

//Offset the time that the GPS last updated (needed if you are sleeping the Arduino and millis() isn't incremented)
void remoteFona::offsetTime(long offset) {
    unixTime += offset/1000;
}

//Get GPS data from the Fona, returns if it was successful
bool remoteFona::getGPSData(unsigned long* time, float* lat, float* lon, unsigned long timeout) {
    unsigned long start = millis();
    
    #if Fona_Make == Adafruit
    toggle();
    #endif
    
    //Start Fona
    NeoSWSerial fonaSS(FONA_RX, FONA_TX);
    fonaSS.begin(9600);
    
    //The way the commands are sent are all the same, so I'll only comment one
    
    //First send an AT command to make sure the Fona is working
    bool err = checkFona(fonaSS);
    
    char response[80];
    
    //Check if GPS is already on
    if (!err && sendGetReply(fonaSS, F("AT+CGPS?"), 2000, response, 80)) {
        if (response[7] != '1') {
            //Turn on GPS
            err = SCRTries(fonaSS, "AT+CGPS=1", "OK", 5000);
        }
    } else {
        err = true;
    }
    
    #ifdef DEBUG
    Serial.println(F("About to wait for Fona GPS"));
    Serial.println(freeMemory());
    #endif
    
    //If it's the first run, it will force all data to be updated
    bool force = unixTime == 0;
    
    //Try to get GPS data for timeout amount of time
    while (!err && (force || ((millis() - start) < timeout))) {
        if (sendGetReply(fonaSS, F("AT+CGPSINFO"), 5000, response, 80)) {
            if (response[10] == ',' || response[5] != 'I') { //Means data isn't ready yet
                delay(2000); //Don't flood FONA
                continue;
            }
            
            uint8_t curr = 10;
            char degStr[4] = {response[curr++], response[curr++], 0, 0};
            
            int deg = atoi(degStr);
            
            uint8_t before = curr;
            while (response[curr] != ',') { curr++; }
            response[curr++] = 0;
            
            float minute = atof(&response[before]);
            
            latitude = deg + (minute/60);
            
            if (response[curr++] == 'S') {
                latitude *= -1;
            }
            curr++;
            
            degStr[0] = response[curr++];
            degStr[1] = response[curr++];
            degStr[2] = response[curr++];
            degStr[3] = 0;
            deg = atoi(degStr);
            
            before = curr;
            while (response[curr] != ',') { curr++; }
            response[curr++] = 0;
            
            minute = atof(&response[before]);
            
            longitude = deg + (minute/60);
            
            if (response[curr++] == 'W') {
                longitude *= -1;
            }
            curr++;
                        
            before = curr;
            while (response[curr] != '.') { curr++; }
            response[curr] = 0;
            
            unixTime = getUnixTime(&response[before]);
            timeLastUpdated = millis();
            
            #ifdef DEBUG
            Serial.print(F("Time: "));
            Serial.println(unixTime);
            Serial.print(F("Latitude: "));
            Serial.println(latitude, 6);
            Serial.print(F("Longitude: "));
            Serial.println(longitude, 6);
            #endif
            
            break;
        } else {
            err = true;
        }
    }
    
    err = SCRTries(fonaSS, "AT+CGPS=0", "OK", 5000);
    
    fonaSS.end();
    
    (*lat) = latitude;

    (*lon) = longitude;

    (*time) = unixTime + (millis() - timeLastUpdated)/1000;
    
    #if Fona_Make == Adafruit
    toggle();
    #endif
    
    return !err;
}

/*---------------------------------PRIVATE---------------------------------*/
#if Fona_Make == Adafruit
//Toggle the Fona on/off
void remoteFona::toggle() {
    digitalWrite(FONA_EN, LOW);
    delay(4000);
    digitalWrite(FONA_EN, HIGH);
}
#endif

//Check if Fona has started correctly
bool remoteFona::checkFona(Stream& port) {
    return SCRTries(port, "AT", "OK", 1000, 20);
}

//Close and stop the HTTPS session
void remoteFona::closeHTTPS(Stream& port) {
    sendCheckReply(port, "AT+CHTTPSCLSE", "OK", 2000); //Send close HTTP session command
    sendCheckReply(port, "AT+CHTTPSSTOP", "OK", 2000); //Send stop HTTP command
}

//Call sendCheckReply tries number of times and return if there was an error or not
bool remoteFona::SCRTries(Stream& port, const char* command, const char* reply, unsigned long timeout, uint8_t tries) {
    for (uint8_t i = 0; i < tries; i++) {
        if (sendCheckReply(port, command, reply, timeout)) {
            return false;
        }
    }
    return true;
}

//Send an AT command to the Fona and wait for a specific reply
//Command is as a char*
bool remoteFona::sendCheckReply(Stream& port, const char* command, const char* reply, unsigned long timeout) {
    #ifdef DEBUG
    Serial.print(F("Sending to fona: "));
    Serial.println(command);
    #endif
    
    port.flush();
    
    port.println(command); //Send the command to the Fona
    
    unsigned long start = millis(); //The time before we wait in the while loop
    
    char response[20]; //Char array for the response from the Fona
    uint8_t curr = 0; //Index for the response char array
    bool reset = false; //If the current command is done
    
    //While timeout time hasn't passed yet
    while ((millis() - start) < timeout) {
        if (port.available()) {
            char c = port.read(); //Read the available character
            
            if (c > 25 && curr < 19) { //If it's an actual character (not '\n' or '\r')
                response[curr++] = c;
            } else {
                reset = true;
            }
            
            response[curr] = 0; //Add a terminating 0

            if (strcmp(response, reply) == 0) { //If it's the response we're waiting for
                #ifdef DEBUG
                Serial.println(response);
                #endif
                
                return true;
            }
            
            if (reset) {
                #ifdef DEBUG
                if (curr != 0) Serial.println(response);
                #endif
                
                curr = 0;
                reset = false;
            }
        }
    }
    
    return false;
}

//Send an AT command to the Fona and return the answer
//Command is as a __FlashStringHelper
bool remoteFona::sendGetReply(Stream& port, const __FlashStringHelper* command, unsigned long timeout, char* ans, uint8_t length) {
    #ifdef DEBUG
    Serial.print(F("Sending to fona: "));
    Serial.println(command);
    #endif
    
    while (port.available()) port.read(); //Empty port first
    
    port.println(command); //Send the command to the Fona
    
    unsigned long start = millis(); //The time before we wait in the while loop
    
    uint8_t curr = 0; //Index for the ans char array
    
    //While timeout time hasn't passed yet
    while ((millis() - start) < timeout) {
        if (port.available()) {
            char c = port.read(); //Read the available character

            if (c > 25 && curr < length-1) { //If it's an actual character (not '\n' or '\r')
                ans[curr++] = c;
            } else {
                if (curr != 0) {
                    ans[curr] = 0;
                    
                    return true;
                }
            }
        }
    }
    
    return false;
}

//Parse string with date and time information into Unix Time
unsigned long remoteFona::getUnixTime(char* str) {
    struct tm timeStruct;
    char arr[3];
    arr[2] = 0;
    
    //Get day of the month
    arr[0] = str[0];
    arr[1] = str[1];
    timeStruct.tm_mday = atoi(arr);
    
    //Get month
    arr[0] = str[2];
    arr[1] = str[3];
    timeStruct.tm_mon = atoi(arr) - 1;
    
    //Get year
    arr[0] = str[4];
    arr[1] = str[5];
    timeStruct.tm_year = atoi(arr) + 100;
    
    //Get hour
    arr[0] = str[7];
    arr[1] = str[8];
    timeStruct.tm_hour = atoi(arr);
    
    //Get minutes
    arr[0] = str[9];
    arr[1] = str[10];
    timeStruct.tm_min = atoi(arr);
    
    //Get seconds
    arr[0] = str[11];
    arr[1] = str[12];
    timeStruct.tm_sec = atoi(arr);
    
    return mktime(&timeStruct) + 946684800; //Add number for difference between Jan 1 1970 and Y2K
}
