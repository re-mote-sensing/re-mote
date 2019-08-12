/*
Library for using a 3G module (SIM5320), used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last modified on August 12, 2019
Released into the public domain
*/

#include "remote3G.h"
#include <remoteConfig.h>
#include <NeoSWSerial.h>
#include <time.h>

#ifdef DEBUG
#include <MemoryFree.h>
#endif

//Start the 3G
remote3G::remote3G() {
    pinMode(cell3G_EN, OUTPUT);
}

//Initialise the 3G module
bool remote3G::initialise() {
    #if cell3G_Make == Tinysine
    //Turn on the 3G chip
    digitalWrite(cell3G_EN, HIGH);
    delay(180);
    digitalWrite(cell3G_EN, LOW);
    
    #elif cell3G_Make == Adafruit
    //Turn the 3G chip on
    digitalWrite(cell3G_EN, HIGH);
    
    //Check to see if the 3G chip is on
    NeoSWSerial ss(cell3G_RX, cell3G_RX);
    ss.begin(9600);
    
    if (check3G(ss)) { //If not
        toggle();
        if (check3G(ss)) { //If it still isn't working
            ss.end();
            return false;
        }
    }
    
    //Turn the chip off
    ss.end();
    toggle();
    
    #endif
    
    return true;
}

bool remote3G::startHTTPS() {
    #if cell3G_Make == Adafruit
    bool err = toggle(true);
    #else
    bool err = false;
    #endif
    
    //Initialise the 3G software serial
    NeoSWSerial ss(cell3G_RX, cell3G_RX);
    ss.begin(9600);
    
    //First send an AT command to make sure the 3G is working
    if (!err) err = check3G(ss);
    
    closeHTTPS(ss); //Close HTTPS session
    
    //Send HTTP start command
    if (!err) err = SCRTries(ss, "AT+CHTTPSSTART", "OK", 10000);
    
    delay(500);
    
    ss.end();
    
    return err;
}

//Post a given HTTP request through the 3G
bool remote3G::post(char* request, const char* host, int portNum) {
    //Initialise the 3G software serial
    NeoSWSerial ss(cell3G_RX, cell3G_RX);
    ss.begin(9600);
    
    //First send an AT command to make sure the 3G is working
    bool err = check3G(ss);
    
    //Build HTTP open session command with the defined url and port
    char* httpsArr = (char*) malloc(sizeof(char) * (20 + strlen(host) + 5));
    if (httpsArr == NULL) {
        err = true;
    }
    sprintf(httpsArr, "AT+CHTTPSOPSE=\"%s\",%d,1", host, portNum);
    
    //Send HTTP open session command
    if (!err) err = SCRTries(ss, httpsArr, "OK", 10000);

    free(httpsArr);
    
    
    //Build HTTP send command with size of given request
    httpsArr = (char*) malloc(sizeof(char) * 20);
    if (httpsArr == NULL) {
        err = true;
    }
    sprintf(httpsArr, "AT+CHTTPSSEND=%d", strlen(request));
    
    //Send HTTP send command
    if (!err) err = SCRTries(ss, httpsArr, ">", 20000);
    
    free(httpsArr);
    
    
    //Send HTTP request
    if (!err) err = SCRTries(ss, request, "OK", 10000);
    
    sendCheckReply(ss, "AT+CHTTPSCLSE", "OK", 1000); //Send close HTTPS session command
    
    ss.end();
    
    return err; //Return if there was an error or not
}

void remote3G::stopHTTPS() {
    //Initialise the 3G software serial
    NeoSWSerial ss(cell3G_RX, cell3G_RX);
    ss.begin(9600);
    
    closeHTTPS(ss); //Close HTTPS session
    
    ss.end();
    
    #if cell3G_Make == Adafruit
    toggle();
    #endif
}

//Offset the time that the GPS last updated (needed if you are sleeping the Arduino and millis() isn't incremented)
void remote3G::offsetTime(long offset) {
    unixTime += offset/1000;
}

//Get GPS data from the 3G, returns if it was successful
bool remote3G::getGPSData(unsigned long* time, float* lat, float* lon, unsigned long timeout) {
    unsigned long start = millis();
    
    #if cell3G_Make == Adafruit
    bool err = toggle(true);
    #else
    bool err = false;
    #endif
    
    //Start 3G
    NeoSWSerial ss(cell3G_RX, cell3G_RX);
    ss.begin(9600);
    
    //The way the commands are sent are all the same, so I'll only comment one
    
    //First send an AT command to make sure the 3G is working
    if (!err) err = check3G(ss);
    
    char response[80];
    
    //Check if GPS is already on
    if (!err && sendGetReply(ss, F("AT+CGPS?"), 2000, response, 80)) {
        if (response[7] != '1') {
            //Turn on GPS
            err = SCRTries(ss, "AT+CGPS=1", "OK", 5000);
        }
    } else {
        err = true;
    }
    
    #ifdef DEBUG
    Serial.println(F("About to wait for 3G GPS"));
    Serial.println(freeMemory());
    #endif
    
    //If it's the first run, it will force all data to be updated
    bool force = unixTime == 0;
    
    //Try to get GPS data for timeout amount of time
    while (!err && (force || ((millis() - start) < timeout))) {
        if (sendGetReply(ss, F("AT+CGPSINFO"), 5000, response, 80)) {
            
            #ifdef DEBUG
            Serial.println(response);
            #endif
            
            if (response[10] == ',' || response[5] != 'I') { //Means data isn't ready yet
                delay(2000); //Don't flood 3G
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
    
    err = SCRTries(ss, "AT+CGPS=0", "OK", 5000);
    
    ss.end();
    
    (*lat) = latitude;

    (*lon) = longitude;

    (*time) = unixTime + (millis() - timeLastUpdated)/1000;
    
    #if cell3G_Make == Adafruit
    toggle();
    #endif
    
    return !err;
}

/*---------------------------------PRIVATE---------------------------------*/
#if cell3G_Make == Adafruit
//Toggle the 3G on/off
bool remote3G::toggle(bool on) {
    digitalWrite(cell3G_EN, LOW);
    delay(4000);
    digitalWrite(cell3G_EN, HIGH);
    
    bool err = false;
    if (on) {
        //Initialise the 3G software serial
        NeoSWSerial ss(cell3G_RX, cell3G_RX);
        ss.begin(9600);
        
        err = check3G(ss);
        if (!err) err = SCRTries(ss, "ATE0", "OK", 2000);
    }
    
    return err;
}
#endif

//Check if 3G has started correctly
bool remote3G::check3G(Stream& port) {
    return SCRTries(port, "AT", "OK", 1000, 20);
}

//Close and stop the HTTPS session
void remote3G::closeHTTPS(Stream& port) {
    sendCheckReply(port, "AT+CHTTPSCLSE", "OK", 1000); //Send close HTTP session command
    sendCheckReply(port, "AT+CHTTPSSTOP", "OK", 2000); //Send stop HTTP command
}

//Call sendCheckReply tries number of times and return if there was an error or not
//Command is as a __FlashStringHelper*
bool remote3G::SCRTries(Stream& port, const __FlashStringHelper* command, const char* reply, unsigned long timeout, uint8_t tries) {
    for (uint8_t i = 0; i < tries; i++) {
        if (sendCheckReply(port, command, reply, timeout)) {
            return false;
        }
    }
    return true;
}

//Call sendCheckReply tries number of times and return if there was an error or not
//Command is as a char*
bool remote3G::SCRTries(Stream& port, const char* command, const char* reply, unsigned long timeout, uint8_t tries) {
    for (uint8_t i = 0; i < tries; i++) {
        if (sendCheckReply(port, command, reply, timeout)) {
            return false;
        }
    }
    return true;
}

//Send an AT command to the 3G and wait for a specific reply
//Command is as a __FlashStringHelper*
bool remote3G::sendCheckReply(Stream& port, const __FlashStringHelper* command, const char* reply, unsigned long timeout) {
    #ifdef DEBUG
    Serial.print(F("Sending to 3G: "));
    Serial.println(command);
    #endif
    
    port.flush();
    
    port.println(command); //Send the command to the 3G
    
    unsigned long start = millis(); //The time before we wait in the while loop
    
    char response[20]; //Char array for the response from the 3G
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

//Send an AT command to the 3G and wait for a specific reply
//Command is as a char*
bool remote3G::sendCheckReply(Stream& port, const char* command, const char* reply, unsigned long timeout) {
    #ifdef DEBUG
    Serial.print(F("Sending to 3G: "));
    Serial.println(command);
    #endif
    
    port.flush();
    
    port.println(command); //Send the command to the 3G
    
    unsigned long start = millis(); //The time before we wait in the while loop
    
    char response[20]; //Char array for the response from the 3G
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

//Send an AT command to the 3G and return the answer
//Command is as a __FlashStringHelper
bool remote3G::sendGetReply(Stream& port, const __FlashStringHelper* command, unsigned long timeout, char* ans, uint8_t length) {
    #ifdef DEBUG
    Serial.print(F("Sending to 3G: "));
    Serial.println(command);
    #endif
    
    while (port.available()) port.read(); //Empty port first
    
    port.println(command); //Send the command to the 3G
    
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
unsigned long remote3G::getUnixTime(char* str) {
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
