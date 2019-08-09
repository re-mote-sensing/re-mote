/*
Things to do:
- Change code structure to use event loop
- Add proper error ack handling
- Robustify
- When fail wait random time to offset
*/

/* Event loop general idea:

float sensorData[NUMBER_SENSORS]
uint8_t sensorState[NUMBER_SENSORS]
float? lat
float? lon
unsigned long unixTime
unsigned long timeLastUpdated
uint8_t state

same setup?

loop {
    call sensors
    
    read sensors

    read gps

    save data

    send data
    
    sleep
}

callSensors {
    for i in number_sensors
        if sensorState[i] == 0 and state == 0
            callSensor(i)             // just call that sensor, this will deal with drivers and bla bla bla
            sensorState[i] = 1
}

readSensors {
    for i in number_sensors
        if sensorState[i] == 1
            readSensor(i)
}

            readSensor(i) {
                if sensor[i].available
                    sensorData[i] = readSensorData(i)          // readSensorData deals with drivers and different sensors
                    // on success sensorState[i] = 2, on fail sensorState = 0
            }

readGPS {
    //This should be more abstract
    if feedGPS one char //Works if gps then gets fix object
        set lat, lon, unixTime, timeLastUpdated
}

saveData {
    if all sensorState == 2
        save all sensorData + lat + lon + time
        set sensorstate all to 0
        ???? state = 1
}

sendData {
    exactly as it is now I think
    if state = 1 ?? and time passed = gps time lock ??
        state = 2
}

sleep {
    if state = 2
        turn off gps
        go to sleep
        turn on gps
        state = 0
}

*/

/*--------------------------------------------------------------*/
/*-----------------------------CODE-----------------------------*/
/*--------------------------------------------------------------*/

/*---------------------------INCLUDES---------------------------*/
#define MAIN
#include <remoteConfig.h>
#undef MAIN

//Check to make sure compilation is in the right mode
#ifndef End_Node
#error Please put the config file into End_Node mode
#endif

#include <remoteLoRa.h>
#include <remoteGPS.h>
#include <remoteSensors.h>
#include <remoteEndData.h>
#include <remoteSleep.h>

#ifdef DEBUG
#include <MemoryFree.h>
#endif

/*-------------------------CONSTRUCTORS-------------------------*/

remoteLoRa LoRa;
remoteGPS GPS;
remoteSensors Sensors;
remoteEndData Data;
remoteSleep Sleep;

/*-----------------------GLOBAL VARIABLES-----------------------*/

float latitude = 0;
float longitude = 0;

/*---------------------------SETUP------------------------------*/

void setup() {
    #ifdef DEBUG
    Serial.begin(9600); //Begin usb serial for debug print statements
    #endif
    
    #ifdef DEBUG
    Serial.println(F("Initialising GPS"));
    #endif
    GPS.initialise(); //Initialise the connected GPS
    
    #ifdef DEBUG
    Serial.println(F("Initialising sensors"));
    #endif
    Sensors.initialise(); //Initialise the connected sensors
    
    #ifdef DEBUG
    Serial.println(F("Initialising data storage"));
    #endif
    Data.initialise(); //Initialise the connected data storage
    
    #ifdef DEBUG
    Serial.println(F("Initialising LoRa"));
    #endif
    //Write the config parameters to the LoRa module
    while (!LoRa.writeConfig(NETWORK_ID, NODE_ID)) {
        delay(2500);
    }
    
    #ifdef DEBUG
    Serial.println(F("Waiting for input..."));
    #endif
    while (!Serial.available()) ; //Useful for testing
    
    #ifdef DEBUG
    Serial.println(F("Registering node"));
    #endif
    registerNode(); //Register this node with the gateway
    
    #ifdef DEBUG
    //Resets saved data, used in testing
    Data.reset(false);
    #endif
}


/*----------------------------LOOP------------------------------*/

void loop() {
    #ifdef DEBUG
    Serial.print(F("Free memory: "));
    Serial.println(freeMemory());
    #endif
    
    unsigned long start = millis(); //The time this sensor read is starting
    
    uint8_t* data = readGPSSensors(); //Read the sensors and GPS
    Data.saveData(data); //Save the data to memory
    free(data);
    
    //Send the saved data from the EEPROM to the gateway
    uint8_t received = 0;
    
    //Send data until it's not successful
    while (received == 0) {
        #ifdef DEBUG
        Serial.println(F("Sending data"));
        #endif
        received = sendSensorData();
        #ifdef DEBUG
        Serial.print(F("Received ack: "));
        Serial.println(received);
        #endif
        delay(250); //To not flood gateway
    }
    
    //Delay the appropriate amount of time before the next sensor read
    if (received >= 2) {
        //If sending the data wasn't successful, wait the fail amount of time
        long time = Sleep.sleep(Fail_Sleep_Time, start);
        GPS.offsetTime(time);
    } else {
        //If sending was successful, wait the normal amount of time
        long time = Sleep.sleep(Sleep_Time, start);
        GPS.offsetTime(time);
    }
}


/*-----------------------ALL FUNCTIONS--------------------------*/

/*---------------Node Registration Function----------------*/

//Registers this end node with the gateway
void registerNode() {
    //Get the length of the registration message
    uint8_t arrLength = 1 + 1 + strlen(NAME);
    for (uint8_t i = 0; i < NUMBER_SENSOR_NAMES; i++) {
        arrLength += 1 + strlen(sensorNames[i]);
    }
    
    //If the length is longer than one LoRa message
    //Should handle this better
    if (arrLength > 111) {
        Serial.println(F("ERROR: Name and sensor types too long for LoRa message"));
        while (true) ;
    }

    uint8_t* dataArr = (uint8_t*) malloc(sizeof(uint8_t) * arrLength); //Allocate the array for the LoRa message

    dataArr[0] = ((uint8_t) NUMBER_SENSOR_NAMES) & 0b00001111; //Set first byte of message; registration type and number of sensors
    
    uint8_t curr = 1; //Current posistion in the LoRa message
    
    //Copy the length of the node name into the message
    uint8_t currLen = strlen(NAME);
    dataArr[curr++] = currLen;
    
    //Copy the node name into the message
    for (uint8_t i = 0; i < strlen(NAME); i++) {
        dataArr[curr++] = NAME[i];
    }
    
    //Go through each connected sensor and add it's name to the LoRa message
    for (uint8_t i = 0; i < NUMBER_SENSOR_NAMES; i++) {
        //Copy the length of this sensor type into the message
        currLen = strlen(sensorNames[i]);
        dataArr[curr++] = currLen;
        
        //Copy the sensor name into the LoRa message
        memcpy(&dataArr[curr], sensorNames[i], sizeof(char) * currLen);
        curr += currLen;
    }
    
    uint8_t ack = 0xFF;
    
    //Loop until an acknowledgement is received
    do {
        #ifdef DEBUG
        Serial.println(F("Sending registration")); //Debugging print statement
        #endif
        
        uint8_t* ackMessage = LoRa.sendReceive(GATEWAY_ID, arrLength, dataArr, 10000); //Send the registration message
        
        //Get acknowledgment byte
        if (ackMessage != NULL) {
            ack = ackMessage[3];
            free(ackMessage);
        }
        
        #ifdef DEBUG
        if (ack == 0 || ack == 1) { //If the registration was a success
            Serial.println(F("Received ack"));
        } else { //Should deal with different failure codes
            Serial.print(F("Didn't receive ack: "));
            Serial.println(ack);
        }
        #endif
    } while (ack > 1);
    
    free(dataArr); //Free allocated memory
}


/*-----------------Sensor Read Function------------------*/

//Read data from the GPS and the sensors
uint8_t* readGPSSensors() {
    unsigned long time;
    float lat;
    float lon;
    
    #ifdef DEBUG
    Serial.println();
    Serial.println(F("Getting GPS data"));
    Serial.println(freeMemory());
    
    unsigned long before = millis();
    #endif
    
    //Try to update gps data for GPS_Time time
    GPS.getData(&time, &lat, &lon, GPS_Time);
    
    #ifdef DEBUG
    Serial.println(F("GPS data:"));
    Serial.println(freeMemory());
    Serial.print(F("Took: "));
    Serial.println(millis() - before);
    Serial.print(F("Time: "));
    Serial.println(time);
    Serial.print(F("Latitude: "));
    Serial.println(lat);
    Serial.print(F("Longitude: "));
    Serial.println(lon);
    #endif
    
    uint8_t* ans; //Array that will be returned
    uint8_t curr = 0; //Current position in the array
    
    //Check if latitude or longitude has changed enough since it was last sent
    if (abs(lat - latitude) > 0.00005 || abs(lon - longitude) > 0.00005) {
        //Update last sent latitude and longitude
        latitude = lat;
        longitude = lon;
        
        //Allocate memory for answer, including latitude and longitude
        ans = (uint8_t*) malloc(sizeof(uint8_t) * (1 + (4 * (NUMBER_SENSOR_NAMES + 3))));
        
        ans[curr++] = 1; //Means that this data point has location data
        
        //Copy time into array
        memcpy(&ans[curr], &time, sizeof(unsigned long));
        curr +=4 ;
        
        //Copy location into array
        memcpy(&ans[curr], &latitude, sizeof(float));
        memcpy(&ans[curr + 4], &longitude, sizeof(float));
        curr += 8;
    } else {
        //Allocate memory for answer, not including latitude and longitude
        ans = (uint8_t*) malloc(sizeof(uint8_t) * (1 + (4 * (NUMBER_SENSOR_NAMES + 1))));
        
        ans[curr++] = 0; //Means that this data point doesn't have location data
        
        //Copy time into array
        memcpy(&ans[curr], &time, sizeof(unsigned long));
        curr += 4;
    }
    
    #ifdef DEBUG
    //Serial.print(F("Free memory: "));
    //Serial.println(freeMemory());
    #endif
    
    Sensors.read(&ans[curr]); //Read the connected sensors
    
    #ifdef DEBUG
    Serial.println();
    #endif
    
    return ans;
}


/*---------------Send Sensor Data Function----------------*/

//Sends saved data from EEPROM through LoRa
uint8_t sendSensorData() {
    uint8_t* dataArr = Data.getDataMessage(); //Get LoRa message
    
    //Check if there's no data to send
    if (dataArr == NULL) {
        return 1;
    }
    
    uint8_t* ackMessage = LoRa.sendReceive(GATEWAY_ID, dataArr[0], &dataArr[1]); //Send data to the gateway
    
    //Get acknowledgment byte
    uint8_t ack = 0xFF;
    if (ackMessage != NULL) {
        ack = ackMessage[3];
        free(ackMessage);
    }
    
    //Free allocated memory
    free(dataArr);
    
    if (ack == 0) { //If it receives a success acknowledgement
        Data.messageSuccess();
        return 0;
    } else { //Should handle different failure codes correctly
        return 0xFF;
    }
}
