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
#include <remote3G.h>
#include <remoteSensors.h>
#include <remoteGatewayData.h>

#ifdef DEBUG
#include <MemoryFree.h>
#endif

/*------------------------CONSTRUCTORS--------------------------*/

remoteLoRa LoRa;
remote3G cell3G;
remoteSensors Sensors;
remoteGatewayData Data;


/*-----------------------GLOBAL VARIABLES-----------------------*/

float latitude = 0;
float longitude = 0;


/*---------------------------SETUP------------------------------*/

void setup() {
    #ifdef DEBUG
    Serial.begin(9600);
    #endif
    
    #ifdef DEBUG
    Serial.println(F("Initialising Sensors"));
    #endif
    Sensors.initialise();
    
    #ifdef DEBUG
    Serial.println(F("Initialising LoRa"));
    #endif
    while (!LoRa.writeConfig(NETWORK_ID, GATEWAY_ID)) {
        delay(2500);
    }
    
    #ifdef DEBUG
    Serial.println(F("Initialising 3G"));
    #endif
    while (!cell3G.initialise()) {
        delay(2500);
    }
    
    #ifdef DEBUG
    Serial.println(F("Waiting for input..."));
    Serial.println(freeMemory());
    while (!Serial.available()) ; //Useful for testing
    #endif
    
    #ifdef DEBUG
    //Resets the data on the sd card
    //Data.reset(true);
    #endif
    
    #ifdef DEBUG
    Serial.println(F("Initialising Data"));
    #endif
    Data.initialise();
    
    #ifdef DEBUG
    Serial.println(F("Registering this node"));
    #endif
    registerThisNode();
}


/*----------------------------LOOP------------------------------*/

void loop() {
    unsigned long lastPost = millis(); //Get post time before we start to read sensors and post
    readSensors();
    postData(); //Post the saved data
    
    #ifdef DEBUG
    Serial.println(F("Waiting for LoRa messages"));
    Serial.println(freeMemory());
    #endif

    //Just keep checking the LoRa module until the right amount of time has passed
    while ((millis() - lastPost) < Post_Time) {
        uint8_t* loraData = LoRa.readData(); //Read the LoRa module
        
        if (loraData != NULL) { //If the LoRa module received a message
            
            #ifdef DEBUG
            //Print the data to the Serial interface
            Serial.println(F("Received LoRa message"));
            printByte(loraData[0]);
            printByte(loraData[1]);
            printByte(loraData[2]);
            for (int i = 0; i <= loraData[2]; i++) {
                printByte(loraData[i+3]);
            }
            Serial.println();
            #endif

            //Once we encrypt data, it will be decrypted here

            uint8_t type = loraData[3] >> 4; //Get the type of the message
            switch (type) { //Handle the message based on the type
                case 0: //Registration message
                    parseRegistration(loraData);
                    break;

                case 1: //Data message
                    parseData(loraData);
                    break;

                default: //Unknown message
                    #ifdef DEBUG
                    Serial.println(F("Unknown Message Type Received"));
                    #endif
                    sendAck(0xFE, loraData);
            }

            free(loraData); //Free allocated message memory
        }
    }
}


/*------------------------ALL FUNCTIONS-------------------------*/

//Registers this gateway with itself
void registerThisNode() {
    //Get the length of the registration message
    uint8_t arrLength = 1 + 1 + strlen(NAME);
    for (uint8_t i = 0; i < NUMBER_SENSOR_NAMES; i++) {
        arrLength += 1 + strlen(sensorNames[i]);
    }

    uint8_t* dataArr = (uint8_t*) malloc(sizeof(uint8_t) * (arrLength + 3)); //Allocate the array for the LoRa message

    uint16_t add = GATEWAY_ID;
    memcpy(dataArr, &add, sizeof(uint16_t));
    
    dataArr[3] = ((uint8_t) NUMBER_SENSOR_NAMES) & 0x0F; //Set first byte of message; registration type and number of sensors
    
    uint8_t curr = 4; //Current posistion in the LoRa message
    
    //Copy the length of the node name into the message
    uint8_t currLen = strlen(NAME);
    dataArr[curr++] = currLen;
    
    //Copy the node name into the message
    for (uint8_t i = 0; i < currLen; i++) {
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
    
    Data.saveRegistration(dataArr); //Save registration
    
    free(dataArr); //Free allocated memory
}

//Read the sensors and the GPS and save the data
void readSensors() {
    #ifdef DEBUG
    Serial.println(F("Reading sensors"));
    #endif
    
    unsigned long time;// = 1563816782 + millis()/1000;
    float lat;// = 43;
    float lon;// = -79;
    
    //Get GPS data from the 3G chip
    cell3G.getGPSData(&time, &lat, &lon, GPS_Time);
    
    uint8_t* data; //Array of data
    uint8_t timeIndex = 5; //Where in the array time should be inserted
    uint8_t dataIndex = 9; //Where in the array data should be inserted
    
    if (abs(lat - latitude) > 0.00005 || abs(lon - longitude) > 0.00005) {
        //Update last saved latitude and longitude
        latitude = lat;
        longitude = lon;
        
        //Allocate memory for data, including latitude and longitude
        data = (uint8_t*) malloc(sizeof(uint8_t) * (18 + (4 * NUMBER_SENSOR_NAMES)));
        
        data[4] = 1; //Means that this data point has location data
        data[5] = 1;
        
        //Copy location into array
        memcpy(&data[10], &latitude, sizeof(float));
        memcpy(&data[14], &longitude, sizeof(float));
        
        //Update where time and data need to be inserted
        dataIndex = 18;
        timeIndex++;
    } else {
        //Allocate memory for data, not including latitude and longitude
        data = (uint8_t*) malloc(sizeof(uint8_t) * (9 + (4 * NUMBER_SENSOR_NAMES)));
        
        data[4] = 0; //Means that this data point doesn't have location data
    }
    
    uint16_t add = GATEWAY_ID;
    memcpy(data, &add, sizeof(uint16_t));
    
    //The length of the array
    data[2] = dataIndex - 3 + (4 * NUMBER_SENSOR_NAMES);
    
    //The message type
    data[3] = 0x10 | (uint8_t) NUMBER_SENSOR_NAMES;
    
    //Copy time into array
    memcpy(&data[timeIndex], &time, sizeof(unsigned long));
    
    //Read the sensors
    Sensors.read(&data[dataIndex]);
    
    Data.saveData(data); //Save the data
    
    free(data);
}

//Parse registration message and save new node information
void parseRegistration(uint8_t* data) {
    //Parse the registration according to the saved data type
    uint8_t ack = Data.saveRegistration(data);
    
    //Send an acknowledgment
    sendAck(ack, data);
}

//Save the received data into the sd card
void parseData(uint8_t* data) {
    //Parse the data according to the saved data type
    uint8_t ack = Data.saveData(data);
    
    //Send an acknowledgment
    sendAck(ack, data);
}


/*-----------------Saved Data Posting Functions-----------------*/
//Post collected data to webserver
void postData() {
    #ifdef DEBUG
    Serial.println(F("Posting data"));
    #endif
    
    //Start the HTTPS module on the 3G chip
    if (cell3G.startHTTPS()) return;
    
    //Loop until there's no data left to send
    while (true) {
        //If it fails, then it will try to send less data
        for (unsigned int loops = 0; ; loops++) {
            #ifdef DEBUG
            Serial.println(F("Getting new request"));
            Serial.println(freeMemory());
            #endif
            
            void* toFree; //Data that needs to be freed after the request
            
            //Get the request
            char* request = Data.getPost(loops, &toFree);
            
            //If it ran out of RAM space
            if (request == NULL) {
                #ifdef DEBUG
                Serial.println(F("Ran out of space making post"));
                #endif
                continue;
            }
            
            //If there's no data left to send
            if (request[0] == 0) {
                #ifdef DEBUG
                Serial.println(F("No data to post"));
                #endif
                free(request);
                cell3G.stopHTTPS(); //Stop the HTTPS
                return;
            }
            
            #ifdef DEBUG
            Serial.println(F("About to post request"));
            Serial.println(freeMemory());
            #endif

            //Post the valid post through the 3G
            bool error = cell3G.post(request, URL_Host, URL_Port);
            
            #ifdef DEBUG
            Serial.print(F("Error: "));
            Serial.println(error);
            Serial.println(freeMemory());
            #endif
            
            //Free allocated data
            free(request);
            free(toFree);
            
            #ifdef DEBUG
            Serial.println(F("After freeing"));
            Serial.println(freeMemory());
            #endif

            //If it was successful, let the storage know and restart the posting process
            if (!error) {
                Data.messageSuccess();
                break;
            }
        }
    }
}


//Send an acknowledgement
void sendAck(uint8_t ack, uint8_t* data) {
    //The address to send the acknowledgement to
    uint16_t add;
    memcpy(&add, data, sizeof(uint16_t));
    
    //The data of the acknowledgement
    uint8_t* ackData = (uint8_t*) malloc(sizeof(uint8_t));
    ackData[0] = ack;
    
    //Send the acknowledgement
    LoRa.sendData(add, 1, ackData);
    
    free(ackData);
}

/*---------------------------TESTING----------------------------*/
/*
//Function to test the 3G, not needed
void test3G() {
    NeoSWSerial ss(cell3G_RX, cell3G_TX);
    
    ss.begin(9600);
    delay(250);
    
    while (true) {
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '-') {
                toggle();
            } else if (c == '\r') {
                ss.write(c);
                Serial.println();
            } else {
                ss.write(c);
                Serial.write(c);
            }
        }
        while (ss.available()) {
            Serial.write(ss.read());
        }
    }
    
    ss.end();
}

void toggle() {
    digitalWrite(cell3G_EN, LOW);
    delay(4000);
    digitalWrite(cell3G_EN, HIGH);
}*/

#ifdef DEBUG
//Prints a byte as hex to the Serial port
void printByte(uint8_t b) {
  Serial.print(F(" 0x"));
  if (b <= 0xF)
	Serial.print(F("0"));
  Serial.print(b, HEX);
}
#endif