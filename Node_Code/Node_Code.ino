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
    Serial.println(F("Init Sensors"));
    #endif
    Sensors.initialise();
    
    #ifdef DEBUG
    Serial.println(F("Init LoRa"));
    #endif
    while (!LoRa.writeConfig(NETWORK_ID, GATEWAY_ID)) {
        delay(2500);
    }
    
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
}


/*----------------------------LOOP------------------------------*/

void loop() {
    unsigned long lastPost = millis(); //Get post time before we start to read sensors and post
    
    cell3G.power(true);
    readSensors();
    postData(); //Post the saved data
    cell3G.power(false);
    
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
    time = 1563816782 + millis()/1000;
    lat = 43;
    lon = -79;
    #else
    cell3G.getGPSData(&time, &lat, &lon, GPS_Time);
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
    registerNode(); //Register this node with the gateway
    
    #ifdef DEBUG
    // Resets saved data, used in testing
    // Data.reset(false);
    #endif

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
    lat = 42;
    lon = -80;
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




/*-----------------------------------------------------------*/
/*-----------------------Turtle_Tracker----------------------*/
/*-----------------------------------------------------------*/

#elif Node_Type == Turtle_Tracker

/*----------------------------------QUESTIONS--------------------------------------*/


/*----------------------------------TODO---------------------------------------------*/


// Add support for different GPS Modules & GPS protocols
// Algorithm that takes into account time spent underwater to determine allowable time to aquire GPS + aided data

// Add a better check for if underwater
// Check for when Underwater (use LoRa?)
// Counter the keep track of time sent underwater - can report this value during next successful transmission

// save data AND resend data if route not found
// add code to also sleep the rfm95

// Issue - Possible for MESH Netwrok to briefly turn off while sending saved data

/*-----------------------------------NOTES------------------------------------------*/

// uint_8 = char		(1 byte)
// uint16_t = short		(2 bytes)
// uint32_t = int		(4 bytes)
// uint64_t = long		(8 bytes)

// When EN pin is LOW, RTC is not retained. Must use the V_bAT pin (The datasheet made it sound as if this was not necessary
// EN Pin and V_BAT Pin work as expected
// Having V_BAT Pin powered while EN Pin is LOW (i.e.GPS is at full power) does not cause excess current drawn.So the V_BAT pin can be left HIGH indefinitely(may require additional tests to confirm)
// V_BAT Pin output HIGH when Arduino In Low Power? - Yes
// GPS needs full Position data before it saves the RTC and position. After that, Enable and Backup work as expected

// Check output of I/O pins in Low Power Mode - Output stays as what it was set to
// LowPower will NOT work if GPS is left on


// Put GPSLoop() in do...while loop so the data is updated before being checked
	// Nah; still chance of not being updated. Need to have it wait untill .avaialble returns true so fix is updated
	// Edit GPSLoop itself. Make sure it stays there for set amount of time (2 seconds seems good)

// Altered GPSfix_cfg.h in an attempt to reduce RAM usage
// Modified RHMesh.h to include getter/setter for Mesh Route discovery timeout
// Modified RHMesh.cpp - recvfromAck - line 223; additional else statment for if only receiving a broadcast

// Make the code prepar to reset at any point

/*------------------------------INCLUDES---------------------------*/

#include <NMEAGPS.h>
#include <TurtleTracker_LowPower.h>
#include <TurtleTracker_UBX.h>
#include <TurtleTracker_EEPROM.h>
#include <TurtleTrackers_LoRa_Flooding.h>


/*---------------------------------NEO GPS CONFIGURATION CHECK------------------------------*/

#if !defined(GPS_FIX_TIME) | !defined(GPS_FIX_DATE)
#error You must define GPS_FIX_TIME and DATE in GPSfix_cfg.h!
#endif

#if !defined(NMEAGPS_PARSE_RMC) & !defined(NMEAGPS_PARSE_ZDA)
#error You must define NMEAGPS_PARSE_RMC or ZDA in NMEAGPS_cfg.h!
#endif

#if !defined( GPS_FIX_LOCATION )
#error You must uncomment GPS_FIX_LOCATION in GPSfix_cfg.h!
#endif

#if !defined( GPS_FIX_SATELLITES )
#error You must uncomment GPS_FIX_SATELLITES in GPSfix_cfg.h!
#endif

#ifdef NMEAGPS_INTERRUPT_PROCESSING
#error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
#endif

/*______NEOGPS TIME ZONE SETUP (Configured for the US)_____*/

// Set these values to the offset of your timezone from GMT

static const int32_t          zone_hours = -5L; // EDT
static const int32_t          zone_minutes = 0L; // usually zero
static const NeoGPS::clock_t  zone_offset =
zone_hours * NeoGPS::SECONDS_PER_HOUR +
zone_minutes * NeoGPS::SECONDS_PER_MINUTE;

static const uint8_t springMonth = 3;
static const uint8_t springDate = 14; // latest 2nd Sunday
static const uint8_t springHour = 2;
static const uint8_t fallMonth = 11;
static const uint8_t fallDate = 7; // latest 1st Sunday
static const uint8_t fallHour = 2;
#define CALCULATE_DST
/*-------------------------------------SERIAL CONFIGURATION---------------------------------*/
// Software Serial baud rate must be very large
// Hardware Serial cannot be larger than Software Serial
// AltSoftwaSerial must be 10x smaller than Software Serial
// GPS default baudrate is 9600
// NeoSWSerial - Supported baud rates are 9600 (default), 19200 and 38400.


// Serial Setup
#define DEBUG_PORT Serial
#define PC_BAUDRATE 115200L

#define GPS_PORT_NAME "gpsPort"
NeoSWSerial gpsPort(GPS_RX, GPS_TX);
#define GPS_BAUDRATE 9600L

/*--------------------------------------------GPS SETUP-------------------------------------*/

static NMEAGPS  gps;									// This parses received characters from the GPS
static gps_fix  fix;									// This contains all the parsed pieces

/*--------------------------------------------LORA SETUP-------------------------------------*/

RH_RF95 rf95(RFM95_SLAVE, RFM95_INTERUPT);						// Singleton instance of the radio driver
//RH_RF95 rf95;														// Singleton instance of the radio driver
RHReliableDatagram *RHReliableDatagramManager;												// Class to manage message delivery and receipt, using the driver declared above

char data[RH_MESSAGE_LEN];										// This is the payload that is tranmistted

/*-----------------------------------------LIBRARY DEFINITIONS------------------------------------*/
TurtleTracker_UBX turtleTracker_UBX(DEBUG_PORT, gpsPort);
TurtleTracker_LowPower turtleTracker_LowPower(DEBUG_PORT);
TurtleTrackers_LoRa_Flooding turtleTracker_LoRa_Flooding(DEBUG_PORT);
TurtleTracker_EEPROM turtleTracker_EEPROM;

/*---------------------------------ADDITIONAL GPS METHODS--------------------------*/

// Converts UTC time to the time of the predefined time zone
void adjustTime(NeoGPS::time_t& dt)
{
	NeoGPS::clock_t seconds = dt; // convert date/time structure to seconds

#ifdef CALCULATE_DST
  //  Calculate DST changeover times once per reset and year!
	static NeoGPS::time_t  changeover;
	static NeoGPS::clock_t springForward, fallBack;

	if ((springForward == 0) || (changeover.year != dt.year)) {

		//  Calculate the spring changeover time (seconds)
		changeover.year = dt.year;
		changeover.month = springMonth;
		changeover.date = springDate;
		changeover.hours = springHour;
		changeover.minutes = 0;
		changeover.seconds = 0;
		changeover.set_day();
		// Step back to a Sunday, if day != SUNDAY
		changeover.date -= (changeover.day - NeoGPS::time_t::SUNDAY);
		springForward = (NeoGPS::clock_t) changeover;

		//  Calculate the fall changeover time (seconds)
		changeover.month = fallMonth;
		changeover.date = fallDate;
		changeover.hours = fallHour - 1; // to account for the "apparent" DST +1
		changeover.set_day();
		// Step back to a Sunday, if day != SUNDAY
		changeover.date -= (changeover.day - NeoGPS::time_t::SUNDAY);
		fallBack = (NeoGPS::clock_t) changeover;
	}
#endif

	//  First, offset from UTC to the local timezone
	seconds += zone_offset;

#ifdef CALCULATE_DST
	//  Then add an hour if DST is in effect
	if ((springForward <= seconds) && (seconds < fallBack))
		seconds += NeoGPS::SECONDS_PER_HOUR;
#endif

	dt = seconds; // convert seconds back to a date/time structure

} // adjustTime


#ifdef NMEAGPS_INTERRUPT_PROCESSING
static void GPSisr(uint8_t c)
{
	gps.handle(c);
}
#endif

/*---------------------------------------------------------------------------------*/
/*
  Convert the long GPS data into String values as if they were high-precision floats
*/
static String dataToString(int32_t degE7)
{
	String data;
	// Extract and print negative sign
	if (degE7 < 0) {
		degE7 = -degE7;
		data.concat('-');
	}

	// Whole degrees
	int32_t deg = degE7 / 10000000L;
	data.concat(deg);
	data.concat('.');


	// Get fractional degrees
	degE7 -= deg * 10000000L;

	// Print leading zeroes, if needed
	int32_t factor = 100000L;
	while ((degE7 < factor) && (factor > 1L)) {
		data.concat('0');
		factor /= 10L;
	}

	// Print fractional degrees
	data.concat(degE7);

	return data;
}

/*-----------------------------------IMPORTANT GPS METHODS---------------------------*/

/*
Prints various GPS data fpr debugging purposes
*/
static void printGpsData()
{
	//  This is the best place to do your time-consuming work, right after
	//     the RMC sentence was received.  If you do anything in "loop()",
	//     you could cause GPS characters to be lost, and you will not
	//     get a good lat/lon.
	//  For this example, we just print the lat/lon.  If you print too much,
	//     this routine will not get back to "loop()" in time to process
	//     the next set of GPS data.

	DEBUG_PORT.println(F("Time:   "));
	DEBUG_PORT.print(F("Valid:   "));
	DEBUG_PORT.print(fix.valid.time ? "YES" : "NO");
	DEBUG_PORT.print(F("   |   "));
	adjustTime(fix.dateTime);
	DEBUG_PORT << fix.dateTime;
	DEBUG_PORT.println();

	DEBUG_PORT.println(F("Time (UTC - Seconds:   "));
	DEBUG_PORT.print(F("Valid:   "));
	DEBUG_PORT.print(fix.valid.time ? "YES" : "NO");
	DEBUG_PORT.print(F("   |   "));
	unsigned long currTime = ((NeoGPS::clock_t) fix.dateTime) + 946684800; //Get Unix Time (946684800 is because conversion gives Y2K epoch)
	DEBUG_PORT.println(currTime);

	DEBUG_PORT.println(F("Lat/Lon:"));
	DEBUG_PORT.print(F("Valid:   "));
	DEBUG_PORT.print(fix.valid.location ? "YES" : "NO");
	DEBUG_PORT.print(F("   |   "));
	// DEBUG_PORT.print( fix.latitude(), 6 ); // floating-point display
	// DEBUG_PORT.print( fix.latitudeL() ); // integer display
	DEBUG_PORT.print(dataToString(fix.latitudeL())); // prints int like a float
	DEBUG_PORT.print(F(" , "));
	// DEBUG_PORT.print( fix.longitude(), 6 ); // floating-point display
	// DEBUG_PORT.print( fix.longitudeL() );  // integer display
	DEBUG_PORT.print(dataToString(fix.longitudeL())); // prints int like a float
	DEBUG_PORT.println();

	DEBUG_PORT.println(F("# SVs Used:"));
	DEBUG_PORT.print(F("Valid:   "));
	DEBUG_PORT.print(fix.valid.satellites ? "YES" : "NO");
	DEBUG_PORT.print(F("   ,   "));
	DEBUG_PORT.print(fix.satellites);

	DEBUG_PORT.println();
	DEBUG_PORT.println();
}

/*
Reads from the GPS
*/
static void GPSloop()
{
	long time = millis();
	while (millis() - time < GPS_LOOP_TIME) {
		while (gps.available(gpsPort)) {
			fix = gps.read();
		}
	}
}


/*----------------------------------SETUP AND LOOP FUNCTION------------------------*/
void setup() {
	
/*** Initilize Serial Ports, Pins, and Random Generator ***/
	randomSeed(analogRead(0));
	gpsPort.begin(GPS_BAUDRATE);
	DEBUG_PORT.begin(PC_BAUDRATE);
	delay(500);


#if defined(RTC_AVAILABLE)
	pinMode(GPS_ENABLE_PIN, OUTPUT);
	pinMode(GPS_VBAT_PIN, OUTPUT);
	digitalWrite(GPS_VBAT_PIN, HIGH);		// Leave the V_BAT Pin HIGH to ensure GPS is always powered (to retain RTC and position data)
											// Doing so does not cause additional power drawn (as far as I can tell). Will remain HIGH in Low Power Mode
	digitalWrite(GPS_ENABLE_PIN, LOW);

#endif

/*** Initilize LoRa Module and display debugging data	***/
	//Uncomment to set the ID of the Node
	if (!turtleTracker_LoRa_Flooding.setNodeId(NODE_ID))
		return;

	uint8_t nodeId = turtleTracker_LoRa_Flooding.getNodeId();
	RHReliableDatagramManager = new RHReliableDatagram(rf95, nodeId);

	if (turtleTracker_LoRa_Flooding.init(rf95, *RHReliableDatagramManager))
		DEBUG_PORT.println(F("RF95 ready\n"));
	else
		DEBUG_PORT.println(F("RF95 init Failed\n"));

	delay(1000);

	/*** Check how much Memory is available for use ***/
	DEBUG_PORT.print(F("Memory Available = "));
	DEBUG_PORT.println(freeMemory());
	DEBUG_PORT.println();
	// Placed before Pin definition because undefined behaviour occurs once the EN pin is defined
	/*** Configure the GPS Module (if it supports the the UBLOX Protocol) ***/
#if defined(UBLOX_PROTOCOL_AVAILABLE)
	digitalWrite(GPS_ENABLE_PIN, HIGH);
	delay(1000);
	turtleTracker_UBX.configGPS();
	digitalWrite(GPS_ENABLE_PIN, LOW);
	DEBUG_PORT.println();
	delay(500);
#endif

/*** Initialize NEOGPS and display debugging data for GPS ***/
	DEBUG_PORT.println(F("NMEAtimezone.INO: started"));
	DEBUG_PORT.println(F("Looking for GPS device on " GPS_PORT_NAME));
	DEBUG_PORT.println(F("Local time\n"));

	DEBUG_PORT.println(F("NMEAloc.INO: started"));
	DEBUG_PORT.print(F("fix object size = "));
	DEBUG_PORT.println(sizeof(gps.fix()));
	DEBUG_PORT.print(F("NMEAGPS object size = "));
	DEBUG_PORT.println(sizeof(gps));
	DEBUG_PORT.println(F("Looking for GPS device on " GPS_PORT_NAME));
	DEBUG_PORT.println();

	/*** Check how much Memory is available for use ***/
	DEBUG_PORT.print(F("Memory Available = "));
	DEBUG_PORT.println(freeMemory());
	DEBUG_PORT.println();
	delay(1000);

	DEBUG_PORT.flush();
}

void loop() {

	DEBUG_PORT.println(F("Starting..."));

	/*** Let GPS run to aquire data ***/
	DEBUG_PORT.println(F("Running GPS"));
	gpsPort.listen();
	digitalWrite(GPS_ENABLE_PIN, HIGH);

	long gpsRuntime = millis();	// Tracks time that GPS has been running for
	// Let the GPS run for a few seconds to determine if it's underwater
	while (millis() - gpsRuntime < GPS_FIX && !fix.valid.location) {
		GPSloop();
	}
	if (fix.satellites == 0)
		goto lowPowerMode;

	// All good. Continue running to aquire a fix
	while (millis() - gpsRuntime < GPS_FIX && !fix.valid.location) { 
		GPSloop(); 
	}

	DEBUG_PORT.print(F("RUNTIME:     "));
	DEBUG_PORT.println(millis() - gpsRuntime);

	digitalWrite(GPS_ENABLE_PIN, LOW);

	DEBUG_PORT.println(F("GPS Data:    "));
	printGpsData();
	delay(500);

	/*** Construct Payload and Send Data using LoRa RFM95 ***/
	String latS = dataToString(fix.latitudeL());
	String lngS = dataToString(fix.longitudeL());
	unsigned long currTime = ((NeoGPS::clock_t) fix.dateTime) + GPS_TIME_ADJUSTMENT;
	char currTimeS[sizeof(currTime) / sizeof(unsigned long)];
	ltoa(currTime, currTimeS, 10);

	/*DEBUG_PORT.println(F("Size of Lat, Lng, and Time:"));
	DEBUG_PORT.println(sizeof(latS));
	DEBUG_PORT.println(sizeof(lngS));
	DEBUG_PORT.println(sizeof(currTimeS));

	const uint8_t size_t = sizeof(latS) + sizeof(lngS) + sizeof(currTimeS) + 3;
	DEBUG_PORT.println(size_t);
	
	char payload[size_t];										// This is the payload that is tranmistted
	*/

	strcpy(data, latS.c_str());
	strcat(data, " , ");
	strcat(data, lngS.c_str());
	strcat(data, " , ");
	strcat(data, currTimeS);

	DEBUG_PORT.print(F("Data:   "));
	DEBUG_PORT.println(data);

	/*
	Add code here for sending saved data. Maybe just put a large delay (tracker can sleep during it) to ensure all broadcasts have finished, then send next payload
	
	
	
	*/

	// Broadcast Message. Save is send is unsuccessful	
	if (!turtleTracker_LoRa_Flooding.sendData(data))
		turtleTracker_EEPROM.savePayload(data);

	//// Establish connection with a Relayer
	//TurtleTracker_LoRa turtleTracker_LoRa(DEBUG_PORT);
	//bool error = false;
	//
	//uint8_t from;
	//error = turtleTracker_LoRa.initConnection(&from);
	//
	//if (!error) {
	//	DEBUG_PORT.println(F("Data NOT Sent"));
	//	goto lowPowerMode;
	//}
	//DEBUG_PORT.println(F("Sending Data"));
	//
	//// Determine how many payloads are to be sent
	//uint8_t count = turtleTracker_EEPROM.getSavedPayloadsCounter();
	//
	//// Send this value to the Relayer so it knows how long to wait for
	//strcpy(payload, count);
	//error = turtleTracker_LoRa.sendDataToRelayer(&from, payload);
	//if (!error) {
	//	DEBUG_PORT.println(F("Connection Lost. Abort"));
	//	goto lowPowerMode;
	//}
	//
	//// Send any saved data
	//for (uint8_t i = count - 1; i >= 0; i--) {
	//	turtleTracker_EEPROM.getSavedPayload(i, payload);
	//
	//	if (turtleTracker_LoRa.sendDataToRelayer(&from, payload)) {
	//		DEBUG_PORT.print(F("Saved payload Sent - #"));
	//		DEBUG_PORT.println(i);
	//	}
	//	else {
	//		DEBUG_PORT.println(F("Unable to send saved payload. Abort"));
	//		goto lowPowerMode;
	//	}
	//	// decrement saved payload counter so next payload to not be sent will override it
	//	turtleTracker_EEPROM.setSavedPayloadsCounter(turtleTracker_EEPROM.getSavedPayloadsCounter() - 1);
	//}

	delay(100);

lowPowerMode:
	rf95.sleep();			// Sleep the RFM95 Module
	bool isValid = true;

	do {
		// Power on the GPS brief enough to update available data
		DEBUG_PORT.println("Getting Time from GPS");
		gpsPort.listen();
		digitalWrite(GPS_ENABLE_PIN, HIGH);

		// update fix variable with latest time from GPS
		GPSloop();

		digitalWrite(GPS_ENABLE_PIN, LOW);

		// Print the time for debugging purposes
		DEBUG_PORT.println("GPS Data:    ");
		// Below calls Adjust time. If removed, need to put adjustTime() in its place
		printGpsData();

		isValid = false;
#if defined(RTC_AVAILABLE)
		if (fix.valid.time)
			isValid = true;
#endif

		// If the RTC data is not valid, use the default sleep time (1 hour)
		if (!isValid) 
			DEBUG_PORT.println(F("RTC Time not Valid or Available. Using Default Sleep time"));
		else
			DEBUG_PORT.println(F("RTC Time Valid"));


	} while (!turtleTracker_LowPower.enterLowPowerMode(fix.dateTime.hours, fix.dateTime.minutes, fix.dateTime.seconds, isValid));
}

#else
    #error Please set Node_Type to either Sensor_End_Node, Gateway, or Turtle_Tracker
#endif //Node_Type
