/*
Things to do:
- Check for SD card being full
- Change code structure to use event loop
- Robustify
*/


/*--------------------------------------------------------------*/
/*-----------------------------CODE-----------------------------*/
/*--------------------------------------------------------------*/

/*---------------------------INCLUDES---------------------------*/
#define MAIN
#include <remoteConfig.h>
#undef MAIN

//Check to make sure compilation is in the right mode
#ifndef Gateway
#error Please put the config file into Gateway mode
#endif

#define DEBUG

#include <remoteLoRa.h>

#include <SdFat.h>
#include <NeoSWSerial.h>
#include <time.h>

/*------------------------CONSTRUCTORS--------------------------*/

remoteLoRa LoRa;
SdFat sd;
NeoSWSerial ss(cell3G_RX, cell3G_TX);


/*-----------------------GLOBAL VARIABLES-----------------------*/

float latitude = 0;
float longitude = 0;
unsigned long unixTime;
char response[80];

/*---------------------------SETUP------------------------------*/

void setup() {
    pinMode(cell3G_EN, OUTPUT);
    //Turn on the 3G chip
    digitalWrite(cell3G_EN, HIGH);
    delay(180);
    digitalWrite(cell3G_EN, LOW);
    
    #ifdef DEBUG
    Serial.begin(9600);
    #endif
    
    #ifdef DEBUG
    Serial.println(F("Initialising LoRa"));
    #endif
    while (!LoRa.writeConfig(NETWORK_ID, GATEWAY_ID)) {
        delay(2500);
    }
    
    #ifdef DEBUG
    Serial.println(F("Initialising 3G"));
    #endif
    ss.begin(9600);
        
    delay(500);
    
    ss.println("AT+CGPS=1");
    
    while (!sd.begin()) {
        delay(1000);
    }
}


/*----------------------------LOOP------------------------------*/

void loop() {
    unsigned long start = millis();
    
    float lat = latitude;
    float lon = longitude;
    
    Serial.println(F("Getting GPS data"));
    
    ss.listen();
    
    while (lat == latitude && lon == longitude) {
        while (ss.available()) ss.read(); //Empty port first
        ss.print("AT+CGPSINFO\r\n");
        
        uint8_t curr = 0; //Index for the ans char array
        
        delay(500);
        
        while (ss.available()) {
            char c = ss.read();

            if (c > 25 && curr < 79) { //If it's an actual character (not '\n' or '\r')
                response[curr++] = c;
            } else {
                if (curr != 0) { //At the end of the response
                    response[curr] = 0;
                    break;
                }
            }
        }
        
        Serial.println(response);
        if (response[10] == ',' || response[5] != 'I') { //Means data isn't ready yet
            delay(2000); //Don't flood 3G
            continue;
        }

        //Get the latitude degrees as a string
        curr = 10;
        char degStr[4] = {response[curr++], response[curr++], 0, 0};

        //Convert it to an integer
        int deg = atoi(degStr);

        //Get the latitude decimal minutes as a string
        uint8_t before = curr;
        while (response[curr] != ',') { curr++; }
        response[curr++] = 0;

        //Convert it to a float
        float minute = atof(&response[before]);

        //Add the minutes to the integer degrees to get decimal degrees
        lat = deg + (minute/60);

        //Add a - if it's needed
        if (response[curr++] == 'S') {
            lat *= -1;
        }
        curr++;

        //Get the longitude degrees as a string
        degStr[0] = response[curr++];
        degStr[1] = response[curr++];
        degStr[2] = response[curr++];
        degStr[3] = 0;

        //Convert it to an integer
        deg = atoi(degStr);

        //Get the longitude decimal minutes as a string
        before = curr;
        while (response[curr] != ',') { curr++; }
        response[curr++] = 0;

        //Convert it into a float
        minute = atof(&response[before]);

        //Add the minutes to the integer degrees to get decimal degrees
        lon = deg + (minute/60);

        //Add a - if it's needed
        if (response[curr++] == 'W') {
            lon *= -1;
        }
        curr++;

        //Get the string that represents the unix time
        before = curr;
        while (response[curr] != '.') { curr++; }
        response[curr] = 0;

        //Get the integer unix time
        unixTime = getUnixTime(&response[before]);

        break;
    }
    
    latitude = lat;
    longitude = lon;
    
    //The data of the acknowledgement
    uint8_t* ackData = (uint8_t*) malloc(sizeof(uint8_t));
    ackData[0] = 0;
    
    //Send the acknowledgement
    uint8_t* data = LoRa.sendReceive(0x0003, 1, ackData, 5000);
    
    if (data != NULL) {
        //Save to success
        Serial.println(F("Still in range"));
        saveLoc("Success.csv");
        free(data);
    } else {
        //Save to fail
        Serial.println(F("Too far away!"));
        saveLoc("Fail.csv");
    }
    
    while (millis() - start < 5000) ;
    free(ackData);
}


//Send an AT command to the 3G and return the answer
//Command is as a __FlashStringHelper
bool sendGetReply(char* command, unsigned long timeout, char* ans, uint8_t length) {
    while (ss.available()) ss.read(); //Empty port first
    
    ss.println(command); //Send the command to the 3G
    
    unsigned long start = millis(); //The time before we wait in the while loop
    
    uint8_t curr = 0; //Index for the ans char array
    
    Serial.println('\\');
    
    //While timeout time hasn't passed yet
    while ((millis() - start) < timeout) {
        if (ss.available()) {
            char c = ss.read(); //Read the available character
            
            Serial.print((uint8_t) c);
            Serial.print(' ');

            if (c > 25 && curr < length-1) { //If it's an actual character (not '\n' or '\r')
                ans[curr++] = c;
            } else {
                if (curr != 0) { //At the end of the response
                    ans[curr] = 0;
                    
                    Serial.println();
                    Serial.println('/');
                    return true;
                }
            }
        }
    }
    
    Serial.println('/');
    //If it failed
    return false;
}

//Parse string with date and time information into Unix Time
unsigned long getUnixTime(char* str) {
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

void saveLoc(char* fileName) {
    SdFile file;
    file.open(fileName, FILE_WRITE);
    
    file.print(unixTime);
    file.print(", ");
    file.print(latitude, 6);
    file.print(", ");
    file.print(longitude, 6);
    file.print('\n');
    
    file.close();
}