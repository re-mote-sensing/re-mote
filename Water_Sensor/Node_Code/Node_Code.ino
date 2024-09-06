/*
Main Arduino code used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Author: Victor Vezina, Ryan Tyrrell and Tianyu Zhou
Last updated: August 7, 2022
Released into the public domain
*/

#define MAIN
#include <remoteConfig.h>


/*--------------------------------------------------------------*/
/*---------------------------Gateway----------------------------*/
/*--------------------------------------------------------------*/

#if Node_Type == Gateway

/*
Things to do:
- Check for SD card being full
- Change code structure to use event loop
- Robustify
*/


/*---------------------------INCLUDES---------------------------*/

#ifdef ENABLE_LORA
#include <remoteLoRa.h>
#endif
#include <remote3G.h>
#include <remoteSensors.h>
#include <remoteGatewayData.h>

#ifdef ENABLE_RTC
#include <Wire.h>
#include <DS3231.h>
#endif

#ifdef DEBUG
#include <MemoryFree.h>
#endif

/*------------------------CONSTRUCTORS--------------------------*/

#ifdef ENABLE_LORA
remoteLoRa LoRa;
#endif
remote3G cell3G;
remoteSensors Sensors;
remoteGatewayData Data;


/*-----------------------GLOBAL VARIABLES-----------------------*/

float latitude = 0;
float longitude = 0;


/*---------------------------SETUP------------------------------*/

void setup() {

    #ifdef ENABLE_TPL5110
    pinMode(TPL5110_DONE, OUTPUT);
    digitalWrite(TPL5110_DONE, LOW);
    #endif
    
    Serial.begin(9600);
    Serial.println();
    Serial.println(F("Hello")); //If web serial is connected, tell it ready
    
    #ifdef DEBUG
    Serial.println(F("Init Sensors"));
    #endif
    Sensors.initialise();
    if (Serial.available())
      Sensors.configMode(); // Enter Config Mode when serial is available

    #ifdef ENABLE_LORA
    #ifdef DEBUG
    Serial.println(F("Init LoRa"));
    #endif
    while (!LoRa.writeConfig(NETWORK_ID, GATEWAY_ID)) {
        delay(2500);
    }
    #endif
    
    #ifdef DEBUG
    Serial.println(F("Init 3G"));
    #endif
    while (!cell3G.initialise()) {
        delay(2500);
    }
    
    #ifdef DEBUG
//    Serial.println(F("Waiting for input..."));
//    Serial.println(freeMemory());
//    while (!Serial.available()) ; //Useful for testing
    #endif
    
    #ifdef DEBUG
    //Resets the data on the sd card
    //Data.reset(true);
    #endif
    
    #ifdef DEBUG
    Serial.println(F("Init Data"));
    #endif
    Data.initialise();
    
    #ifdef DEBUG
    Serial.println(F("Gateway Register"));
    #endif
    registerThisNode();

    #ifndef DEBUG
    Serial.end();
    #endif
    
    #ifdef DEMO_MODE
    cell3G.power(true); // DEMO
    #endif
}


/*----------------------------LOOP------------------------------*/

void loop() {
    unsigned long lastPost = millis(); //Get post time before we start to read sensors and post

    #ifndef DEMO_MODE
    cell3G.power(true);
    #endif
    readSensors();
    postData(); //Post the saved data
    #ifndef DEMO_MODE
    cell3G.power(false);
    #endif

    #ifdef ENABLE_TPL5110
    pinMode(TPL5110_DONE, OUTPUT);
    digitalWrite(TPL5110_DONE, LOW);
    #ifdef DEBUG
    Serial.println(F("TPL5110 Done"));
    #endif
    while ((millis() - lastPost) < Post_Time) {
      digitalWrite(TPL5110_DONE, HIGH);
      delay(1);
      digitalWrite(TPL5110_DONE, LOW);
      delay(1);
    }
    #endif

    #ifdef ENABLE_LORA

    #ifdef DEBUG
    Serial.println(F("Wait LoRa"));
    Serial.println(freeMemory());
    #endif
    
    //Just keep checking the LoRa module until the right amount of time has passed
    while ((millis() - lastPost) < Post_Time) {
        uint8_t* loraData = LoRa.readData(); //Read the LoRa module
        
        if (loraData != NULL) { //If the LoRa module received a message
            
            #ifdef DEBUG
            //Print the data to the Serial interface
            Serial.println(F("Received LoRa"));
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
                    Serial.println(F("Unknown Message"));
                    #endif
                    sendAck(0xFE, loraData);
            }

            free(loraData); //Free allocated message memory
        }
    }
    #endif
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
    #ifdef DEBUG_NO_GPS
    time = DEBUG_NO_GPS_DEFAULT_TIME + millis()/1000;
    lat = DEBUG_NO_GPS_DEFAULT_LAT;
    lon = DEBUG_NO_GPS_DEFAULT_LON;
    #else
    cell3G.getGPSData(&time, &lat, &lon, GPS_Time);
    #endif

    #ifdef ENABLE_RTC
    Serial.println(F("Reading RTC"));
    RTClib myRTC;
    Wire.begin();
    delay(500);
    DateTime now = myRTC.now();
    time = now.unixtime(); //- 946684800UL;
    Serial.println(time);
    #endif
    
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

#ifdef ENABLE_LORA
// Parse registration message and save new node information
void parseRegistration(uint8_t* data) {
    //Parse the registration according to the saved data type
    uint8_t ack = Data.saveRegistration(data);
    
    //Send an acknowledgment
    sendAck(ack, data);
}
#endif

#ifdef ENABLE_LORA
//Save the received data into the sd card
void parseData(uint8_t* data) {
    //Parse the data according to the saved data type
    uint8_t ack = Data.saveData(data);
    
    //Send an acknowledgment
    sendAck(ack, data);
}
#endif


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

#ifdef ENABLE_LORA
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
#endif

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




/*--------------------------------------------------------------*/
/*---------------------------End Node---------------------------*/
/*--------------------------------------------------------------*/

#elif Node_Type == Sensor_End_Node

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
    Serial.begin(9600); //Begin usb serial for debug print statements
    Serial.println();
    Serial.println(F("Hello")); //If web serial is connected, tell it ready
    
    Serial.println(F("Initialising GPS"));
    GPS.initialise(); //Initialise the connected GPS
    
    Serial.println(F("Initialising sensors"));
    Sensors.initialise(); //Initialise the connected sensors
    if (Serial.available())
      Sensors.configMode(); // Enter Config Mode when serial is available
    
    Serial.println(F("Initialising data storage"));
    Data.initialise(); //Initialise the connected data storage
    
    Serial.println(F("Initialising LoRa"));
    //Write the config parameters to the LoRa module
    while (!LoRa.writeConfig(NETWORK_ID, NODE_ID)) {
        delay(2500); //If it fails wait 2.5 seconds then try again
    }
    
    #ifdef DEBUG
    // Serial.println(F("Waiting for input..."));
    // while (!Serial.available()) ; //Useful for testing
    #endif
    
    Serial.println(F("Registering node"));
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    registerNode(); //Register this node with the gateway
    digitalWrite(LED_BUILTIN, LOW);
    
    // #ifdef DEBUG
    // Resets saved data, used in testing
    Data.reset(false);
    // #endif

    Serial.println(F("Done Setup"));
    #ifndef DEBUG
    Serial.end();
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
    digitalWrite(LED_BUILTIN, HIGH);
    
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
    unsigned long time;// = 9999999 + millis();
    float lat;// = 42;
    float lon;// = -80;
    
    #ifdef DEBUG
    Serial.println();
    Serial.println(F("Getting GPS data"));
    Serial.println(freeMemory());
    
    unsigned long before = millis();
    #endif

    #ifdef DEBUG_NO_GPS
    time = 1563816782 + millis()/1000;
    lat = 43.069057;
    lon = -80.107672;
    #else
    //Try to update gps data for GPS_Time time
    GPS.getData(&time, &lat, &lon, GPS_Time);
    #endif
    
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

#else
    #error Please set Node_Type to either Sensor_End_Node, Gateway, or Turtle_Tracker
#endif //Node_Type
