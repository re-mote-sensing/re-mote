/*----------IMPORTANT INFORMATION AND CONFIG PARAMETERS---------*/

// Change the following parameters to accommodate your specific setup
#define NAME "Akwesasne"           //Name to associate with this node

#define NETWORK_ID 0x1           //LoRa network ID, has to be the same on every LoRa module in your network
#define NODE_ID 0x4              //LoRa ID of this node, must be unique to all nodes in a network
#define GATEWAY_ADDRESS 0x1000   //ID of the gateway node to send data to

#define LORA_RX 7                //Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
#define LORA_TX 6                //Pin that the LoRa RXD pin is connected to
#define GPS_RX 2                 //Pin that the GPS TXD pin is connected to
#define GPS_TX 3                 //Pin that the GPS RXD pin is connected to

// The following paramaters have to do with the sensors you're using on this end node
// Look on our GitLab (LINK!) for more information on sensor setup and how to edit these values
#define NUMBER_SENSORS 3 //Max is 15
char* sensorTypes[NUMBER_SENSORS] = {"Dissolved_Oxygen", "Conductivity", "Turbidity"};
uint8_t sensorPorts[NUMBER_SENSORS][2] = { {12, 11}, {9, 8}, {5, 20} };


/*
Things to do:
- Make location part of sensor data
- Make name part of registration
- Split functions into driver files
- Change initialiseAtlas
- Change code structure to use event loop
- Change to NEOGPS
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

#include <SPI.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Time.h>
#include <TinyGPS++.h>

/*-------------------------CONSTRUCTORS-------------------------*/

SoftwareSerial loraPort(LORA_RX, LORA_TX);

TinyGPSPlus gps;
SoftwareSerial gpsPort(GPS_RX, GPS_TX);

/*-----------------------GLOBAL VARIABLES-----------------------*/

unsigned long currTime; //The last updated unix time from the gps
unsigned long timeLastUpdated; //The millis() value that the unix time was updated
uint8_t timesRead = 0; //The amount of times the sensors have been read

/*---------------------------SETUP------------------------------*/

void setup() {
    Serial.begin(9600); //Begin usb serial for debug print statements
    
    initialiseSensors(); //Initialise the connected sensors
    
    //Begin the LoRa module software serial and write the config based on the defines at the top of this file
    loraPort.begin(9600);
    delay(750);
    writeConfig(loraPort, NETWORK_ID, NODE_ID);
    delay(750);
    
    while (!Serial.available()) ; //Useful for testing
    
    registerNode(); //Register this node with the gateway
    
    gpsPort.begin(9600); //Begin the gps software serial
    
    delay(250);
    
    sendLocation(true); //Send this node's location to the gateway. Force is set to true, so it will continue trying until it's successfull

    //Check if EEPROM has ever been used, if not initialise it correctly
    unsigned int initialCheck = 0;
    EEPROM.get(0, initialCheck);
    if (initialCheck >= EEPROM.length()) {
        unsigned int address = 4;
        EEPROM.put(0, address);
        EEPROM.put(2, address - 1);
        Serial.print(F("Initialised EEPROM, was: "));
        Serial.println(initialCheck);
    }

    //Used for resetting the saved data
    //unsigned int i = 4;
    //EEPROM.put(0, i);
    //EEPROM.put(2, i - 1);
    
    delay(500);
}

/*----------------------------LOOP------------------------------*/

void loop() {
    //Tries to send this node's location every ten sensor reads
    if (timesRead++ >= 10) {
        if (sendLocation(false)) {
            timesRead = 0;
        }
    }
    
    unsigned long start = millis(); //The time this sensor read is starting
    
    saveData(readSensors()); //Read the sensors and save the data to the EEPROM
    
    loraPort.listen();
    
    //Send the saved data from the EEPROM to the gateway
    uint8_t received = 0;
    
    //Send data until either an acknowledgement isn't received or there's no data left to send
    while (received == 0) {
        Serial.print(F("Sending again"));
        received = sendSensorData();
    }
    
    //Delay the appropriate amount of time before the next sensor read
    if (received > 1) {
        Serial.println(F("Didn't recieve ack"));
        delayOverflow(45000, start); //If sending the data wasn't successful then wait 3/4 of the full time, to offset this node from the gateway
    } else {
        Serial.print(F("Received ack: "));
        Serial.println(received);
        delayOverflow(60000, start); //If sending tha data was successful then wait the full time
    }
}

/*-----------------------ALL FUNCTIONS--------------------------*/


/*---------------Sensor Initialisation Functions----------------*/
//Initialise the sensors
void initialiseSensors() {
    //Go through each sensor and call it's respective initialisation function
    for (uint8_t i = 0; i < NUMBER_SENSORS; i++) {
        if (strcmp(sensorTypes[i], "Dissolved_Oxygen") == 0) { //DO
            initialiseAtlas(i);
        } else if (strcmp(sensorTypes[i], "Conductivity") == 0) { //EC
            initialiseAtlas(i);
        } else if (strcmp(sensorTypes[i], "Turbidity") == 0) { //TB
            initialiseTB(i);
        }
    }
    return;
}

//Initialise an Atlas Scientific sensor at index i
void initialiseAtlas(uint8_t i) {
    //Add turning off continous??
    //Also setting up the EC sensor to only read conductivity??
    
    //Initialise the software serial for this sensor
    SoftwareSerial sensor(sensorPorts[i][0], sensorPorts[i][1]);
    sensor.begin(9600);
    
    //For some reason it seems you have to send a couple random commands before sending the sleep command
    sensor.print(F("R\r"));
    delay(100);
    sensor.print(F("R\r"));
    delay(1000);
    sensor.print(F("Sleep\r"));
    
    sensor.end();
    return;
}

//Initialise a DFRobot Turbidity sensor at index i
void initialiseTB(uint8_t i) {
    pinMode(sensorPorts[i][0], OUTPUT); //Set the digital pin to output
    digitalWrite(sensorPorts[i][0], LOW); //Turn the sensor off
    
    pinMode(sensorPorts[i][1], INPUT); //Set the analog pin to input
    
    return;
}


/*-------------------GPS Time Functions--------------------*/

//Gets current Unix time based on last time update and current system time
uint32_t gpsGetTime() {
    //Essentially adds the difference in the millis() function from the last time the unix time was saved to the last saved unix time
    return currTime + ((millis() >= timeLastUpdated) ? (millis() - timeLastUpdated) : (millis() + (4294967295 - timeLastUpdated)))/1000; //Accounts for overflow
}

//Sets current Unix time from GPS
bool gpsSetTime(bool force) {
    unsigned long timeout = 30000; //Time that the Arduino will try too get the time from the gps, should be parameter or define
    unsigned long before = millis(); //millis() time before trying the gps
    bool worked = false; //If the time was updated
    
    bool timeBool = false; //If the time from the gps was updated
    bool dateBool = false; //If the date from the gps was updated
    
    //Will try the gps for timeout time, or forever if force is set to true
    while (force || ((millis() >= before) ? ((millis() - before) < timeout) : ((millis() + (4294967295 - before)) < timeout))) { //Accounts for overflow
        gpsFeedInfo(); //Feed info to the library from the gps module
        
        //Check if the time or date was updated
        bool dateUpdated = gps.date.isUpdated();
        bool timeUpdated = gps.time.isUpdated();
        timeBool = timeBool || timeUpdated;
        dateBool = dateBool || dateUpdated;
        
        //If either was updated, update the saved unix time
        if (gps.time.isValid() && gps.date.isValid() && (timeUpdated || dateUpdated)) {
            worked = true; //Time is being updated
            uint32_t time = gps.time.value(); //Get the current gps time
            uint32_t date = gps.date.value(); //Get the current gps date
            
            //Used external library to convert from time and date notation to unix time
            setTime(time/1000000, (time/10000)%100, (time/100)%100, date/10000, (date/100)%100, (date%100) + 2000); //Set time in library
            currTime = now(); //Get unix time from the set time
            timeLastUpdated = millis(); //Set the last updated time
            
            //debugging print statements
            Serial.print(F("Updating time to: "));
            Serial.println(currTime);
        
            //If both date and time were updated then break out of the loop
            if (timeBool && dateBool) {
                break;
            }
        } else { //If neither was updated, print debugging statement
            Serial.println(F("Failed to get GPS time and date"));
        }
    }
    
    if (!worked) { //If the unix time wasn't updated
        Serial.println(F("Aborting GPS time and date"));
        return false;
    }
    
    return true;
}

//Feed data from the gps module into the software gps data formatter
bool gpsFeedInfo() {
    gpsPort.listen();
    delay(250);
	while (gpsPort.available()) { //Checks if data is available from the gps module
		if (gps.encode(gpsPort.read())) { //Repeatedly feed the library characters from the gps module
			return true; //If the encoding is done
        }
	}
	return false;
}


/*---------------Node Registration Function----------------*/

//Registers this end node with the gateway
void registerNode() {
    //Get the length of the registration message
    uint8_t arrLength = 1 + NUMBER_SENSORS;
    for (uint8_t i = 0; i < NUMBER_SENSORS; i++) {
        arrLength += strlen(sensorTypes[i]);
    }
    
    //If the length is longer than one LoRa message
    if (arrLength > 111) {
        Serial.println(F("ERROR: Sensor types too long for LoRa message"));
        while (true) ;
    }

    uint8_t* dataArr = malloc(sizeof(uint8_t) * arrLength); //Allocate the array for the LoRa message

    dataArr[0] = ((uint8_t) NUMBER_SENSORS) & 0b00001111; //Set first byte of message; registration type and number of sensors
    
    //Go through each connected sensor and add it's name to the LoRa message
    uint8_t curr = 1; //Current posistion in the LoRa message
    for (uint8_t i = 0; i < NUMBER_SENSORS; i++) {
        //Copy the length of this sensor type into the message
        uint8_t currLen = strlen(sensorTypes[i]);
        memcpy(&dataArr[curr++], &currLen, sizeof(uint8_t));
        
        //Go through the sensor name and copy it into the LoRa message
        for (uint8_t j = 0; j < strlen(sensorTypes[i]); j++) {
            memcpy(&dataArr[curr++], &sensorTypes[i][j], sizeof(uint8_t));
        }
    }
    
    loraPort.listen();
    
    bool received = false; //If an acknowledgement was received
    
    //Loop until an acknowledgement is received
    while (!received) {
        Serial.println(F("Sending registration")); //Debugging print statement
        
        sendData(loraPort, GATEWAY_ADDRESS, arrLength, dataArr); //Send the registration message
        
        uint8_t ack = ackWait(loraPort, GATEWAY_ADDRESS, 7500); //Wait for an acknowledgement
        
        if (ack == 0) { //If the registration was a success
            received = true;
            Serial.println(F("Received ack"));
        } else { //Should deal woth different failure codes
            Serial.println(F("Didn't receive ack"));
        }
    }
    
    free(dataArr); //Free allocated memory
}


/*---------------GPS Location Functions----------------*/

//Registers this end node with the gateway
bool sendLocation(bool force) {
    float lat; //Variable for latitude
    float lon; //Variable for longitude
    
    unsigned long timeout = 60000; //Time to try the gps, should be parameter or define
    unsigned long before = millis(); //Time before trying the gps
    bool worked = false; //If the location was updated
    
    //Try to update the location for timeout time, or forever if force is true
    while (force || ((millis() >= before) ? ((millis() - before) < timeout) : ((millis() + (4294967295 - before)) < timeout))) { //Accounts for overflow
        gpsFeedInfo(); //Feed info to the library from the gps module
        
        if (gps.location.isUpdated()) { //If the location was updated
            lat = gps.location.lat(); //Get new latitude
            lon = gps.location.lng(); //Get new longitude
            
            if (lat != 0 && lon != 0) { //If the latitude and longitude are actual values
                Serial.println(F("Got Location"));
                worked = true;
                break;
            }
        }
        
        Serial.println(F("Failed to get GPS location"));
        Serial.println(gps.time.value());
        Serial.println(gps.date.value());
        Serial.println(gps.location.lat());
        Serial.println(gps.location.lat());
        Serial.println();
        
    }
    
    if (!worked) { //If the location wasn't updated
        Serial.println(F("Error reading from GPS, aborting location send"));
        return false;
    }
    
    //Update the current time based from the gps
    gpsSetTime(force);
    uint32_t time = gpsGetTime();
    
    uint8_t nameLen = strlen(NAME); //Length of this node's name
    
    //I stopped adding comments here cause most of the following will probably change
    
    uint8_t arrLength = 1 + (3 * 4) + 1 + nameLen;
    
    if (arrLength > 111) {
        Serial.println(F("ERROR: Name too long to be combined with co-ordinates for LoRa message"));
        while (true) ;
    }
    
    Serial.println(F("Inside location send:"));
    Serial.print(F("Time: "));
    Serial.println(time);
    Serial.print(F("Lat: "));
    Serial.println(lat);
    Serial.print(F("Lon: "));
    Serial.println(lon);

    uint8_t* dataArr = malloc(sizeof(uint8_t) * arrLength);

    dataArr[0] = 0b00100000 | (((uint8_t) NUMBER_SENSORS) & 0b00001111);
    
    memcpy(&dataArr[1], &time, sizeof(uint8_t) * 4);
    memcpy(&dataArr[5], &lat, sizeof(uint8_t) * 4);
    memcpy(&dataArr[9], &lon, sizeof(uint8_t) * 4);
    
    dataArr[13] = nameLen;
    
    char* name = NAME;
    for (uint8_t i = 0; i < nameLen; i++) {
        dataArr[i + 14] = name[i];
    }
    
    loraPort.listen();
    bool received = false;
    uint8_t tries = 0;
    while (!received && (force || (tries++) < 3)) {
        Serial.println(F("Sending Location"));
        sendData(loraPort, GATEWAY_ADDRESS, arrLength, dataArr);
        
        uint8_t ack = ackWait(loraPort, GATEWAY_ADDRESS, 5000);
        
        if (ack == 1) {
            Serial.println(F("Received initial ack"));
        
            ack = ackWait(loraPort, GATEWAY_ADDRESS, 150000);
        
            if (ack == 0) {
                received = true;
                Serial.println(F("Received final ack"));
            } else {
                Serial.println(F("Didn't receive final ack"));
            }
        } else {
            Serial.println(F("Didn't receive initial ack"));
        }
    }
    
    free(dataArr);
    
    return true;
}


/*-----------------Data Save Function------------------*/

//Save data to EEPROM
void saveData(uint32_t* data) {
    Serial.println(F("Saving data"));
    
    unsigned int currAddress = 4;
    EEPROM.get(0, currAddress); //Get the current EEPROM address of the last saved data point
    
    unsigned int validToAddress = 3;
    EEPROM.get(2, validToAddress); //Get the current EEPROM address where valid data starts
    
    bool loop = currAddress <= validToAddress; //If the data saving has looped the EEPROM memory
    bool doubleLoop = false; //If the data has looped the EEPROM memory twice
    
    if (validToAddress == 3) { //This is needed for the weird case of the default state
        validToAddress++;
    }
    
    //Checks for exceeding EEPROM length
    for (uint8_t i = 0; i < (NUMBER_SENSORS + 1); i++) {
        //Convert from a uint32_t to an array of uint8_t, to save ach byte individually
        uint8_t* currValue = malloc(sizeof(uint8_t) * 4);
        memcpy(currValue, &data[i], sizeof(uint8_t) * 4);
        
        for (uint8_t j = 0; j < 4; j++) { //Loop through each byte of the current value
            EEPROM.update(currAddress++, currValue[j]); //Write the byte
            
            //Chack for looping/double looping in the EEPROM
            if (currAddress == EEPROM.length()) {
                currAddress = 4;
                if (loop) {
                    doubleLoop = true;
                } else {
                    loop = true;
                }
            }
        }
        
        free(currValue); //Free allocated memory
    }
    
    EEPROM.put(0, currAddress); //Change address of last saved data point
    
    //If the address where valid data starts has changed, put that change into the EEPROM
    if (doubleLoop || (loop && (currAddress > validToAddress))) {
        EEPROM.put(2, validToAddress + (4 * (NUMBER_SENSORS + 1)));
    } else {
        EEPROM.put(2, validToAddress);
    }
    
    free(data); //Free memory allocated with passed in data
}


/*-----------------Sensor Read Function------------------*/

//Read data from all sensors based on their types
uint32_t* readSensors() {
    Serial.println(F("\nReading Sensors"));
    
    uint32_t* ans = malloc(sizeof(uint32_t) * (NUMBER_SENSORS + 1)); //Allocate array for sensor data
    
    //Try to update time and put it into the data array
    gpsSetTime(false);
    ans[0] = gpsGetTime();
    
    //Go through each sensor and get the data, then add it to the data array
    //All sensors need to return a float, or else the go server won't interpret it correctly
    for (uint8_t i = 0; i < NUMBER_SENSORS; i++) {
        float data; //Data from this sensor
        
        if (strcmp(sensorTypes[i], "Dissolved_Oxygen") == 0) { //Atlas Scientific Dissolved Oxygen
            data = readAS(i);
        } else if (strcmp(sensorTypes[i], "Conductivity") == 0) { //Atlas Scientific Electrical Conductivity
            data = readAS(i);
        } else if (strcmp(sensorTypes[i], "Turbidity") == 0) { //DFRobot Turbidity
            data = readTB(i);
        } else { //Should probably handle this better
            data = -1;
        }
        
        memcpy(&ans[i+1], &data, sizeof(uint32_t)); //Copy this sensor's data into the data array
    }
    
    return ans;
}

//Read data from an Atlas Scientific sensor at index i
float readAS(uint8_t i) {
    //Initialise the sensor's software serial
    SoftwareSerial sensor(sensorPorts[i][0], sensorPorts[i][1]);
    sensor.begin(9600);
    
    //Wake the sensor up
    sensor.print(F("R\r"));
    
    float data = 0.0; //Data for this sensor
    
    char buf[40]; //Char buffer for the sensor's response
    
    uint8_t tries = 0; //How many times the sensor has been read
    while (!data && (tries++ < 1)) { //If a 0 is read the first time, will try again just to make sure (have had trouble with EC sensor returning 0 first try for no reason)
        //Make the sensor read until it returns a number
        do {
            delay(1000); //Delay to not flood sensor, not sure if it needs to be this high
            sensor.print(F("R\r")); //Send the read command to the sensor
            while (sensor.available() <= 0); //Wait for it's response
            delay(100); //Delay to wait for the entire response to be ready

            int dataIndex = sensor.readBytesUntil('\r', buf, 16); //Read the response
            buf[dataIndex] = 0; //Add terminating character
        } while ((buf[0]<48) || (buf[0]>57)); //While the buffer isn't a number

        data = atof(buf); //Convert the character array response to a float
    }
    
    //Debugging print statements
    Serial.print(F("Read AS sensor: "));
    Serial.println(data);
    
    //Put sensor to sleep
    sensor.print(F("Sleep\r"));
    
    sensor.end();
    
    return data;
}

//Read data from a DFRobot turbidity sensor at index i
float readTB(uint8_t i) {
    digitalWrite(8, HIGH); //Turn on the sensor
    delay(100);
    
    float data = analogRead(sensorPorts[i][0]); //Read the sensor
    
    //Do some manipulation to the data to get a more "accurate" number (just something that looks more correct)
    float ans;
    if (data < 775.0) {
        ans = 775.0 - data;
    } else {
        int factor = data / 5;
        ans = data - (5.0 * factor);
    }
    
    digitalWrite(8, LOW); //Turn off the sensor
    
    //Debugging print statements
    Serial.print(F("Read TB sensor: "));
    Serial.println(ans);
    
    return ans;
}


/*---------------Send Sensor Data Function----------------*/

//Sends saved data from EEPROM through LoRa
uint8_t sendSensorData() {
    //Get EEPROM address of last data point
    unsigned int currAddress = 4;
    EEPROM.get(0, currAddress);
    
    //Get EEPROM address of where valid data starts
    unsigned int validToAddress = 3;
    EEPROM.get(2, validToAddress);
    
    if (validToAddress == 3) { //Default state, means no data
        return 1;
    }
    
    unsigned int numBytes; //How many bytes of data there is to send
    if (currAddress > validToAddress) {
        numBytes = currAddress - validToAddress;
    } else {
        numBytes = (currAddress - 4) + (EEPROM.length() - validToAddress); //If data was looped around EEPROM
    }
    
    if (numBytes == 0) { //If there's no data to send
        return 1;
    }
    
    if (numBytes > 110) { //If there's too much data to send in one LoRa message
        numBytes = 110;
    }
    
    numBytes -= numBytes % (4 * (NUMBER_SENSORS + 1)); //Makes sure the number of bytes to send is a multiple of (4 * (NUMBER_SENSORS + 1))
    
    uint8_t* dataArr = malloc(sizeof(uint8_t) * (numBytes + 1)); //Allocate the array for the LoRa message
    
    dataArr[0] = 0b00010000 | ((uint8_t) NUMBER_SENSORS & 0b00001111); //Set the first byte of the message; the message type and the number of sensors
    
    //Read data byte by byte into the LoRa message
    for (uint8_t i = 0; i < numBytes; i++) {
        dataArr[1 + i] = EEPROM.read(validToAddress++);
        
        if (validToAddress == EEPROM.length()) { //Have to loop around EEPROM
            validToAddress = 4;
        }
    }
    
    Serial.println(F("Sending:")); //Debugging print statement
    
    sendData(loraPort, GATEWAY_ADDRESS, numBytes + 1, dataArr); //Send data to the gateway
    
    free(dataArr); //Free allocated memory

    uint8_t ack = ackWait(loraPort, GATEWAY_ADDRESS, 5000); //Wait for acknowledgement
    
    if (ack == 0) { //If it receives a success acknowledgement
        
        if (validToAddress == currAddress) { //If all valid data has been read, reset the addresses
            unsigned int address = 4;
            EEPROM.put(0, address);
            EEPROM.put(2, address - 1);
        } else {
            EEPROM.put(2, validToAddress); //Set the address of the last data point correctly
        }
        
        return 0;
        
    } else { //Should handle different failure codes correctly
        return 0xFF;
    }
}


/*-------------------Helper Functions-------------------*/

//Wait timeout for an acknoledgement from address, returns first byte of acknoledgement
uint8_t ackWait(Stream& port, uint16_t address, unsigned long timeout) {
    unsigned long before = millis(); //Time before waiting for the acknowledgement
    
    uint8_t ans = 0xFF; //The acknowledgement code received, oxFF will be returned if nothing is received
    
    //Loop until an ackonwledgement is received or timeout time has passed
    while ((millis() >= before) ? ((millis() - before) < timeout) : ((millis() + (4294967295 - before)) < timeout)) { //Accounts for overflow
        if (port.available()) { //If a LoRa message was received
            uint8_t* data = readData(port); //Get the LoRa message data
            uint16_t dataAdd; //The address the message was received from
            uint8_t dataAns; //The payload of the LoRa message
            
            memcpy(&dataAdd, &data[1], sizeof(uint8_t) * 2); //Get the address from the LoRa message
            memcpy(&dataAns, &data[3], sizeof(uint8_t)); //Get the payload from the LoRa message
            
            free(data); //Free the memory for the LoRa message
            
            if (dataAdd == address) { //Check that the received message is from address
                ans = dataAns; //The answer to return is the received message payload
                break;
            }
        }
    }
    
    return ans;
}

//Delay for an amount of time but accounts for start time and overflow
void delayOverflow(unsigned long time, unsigned long start) {
    //This should actually sleep the Arduino
    
    long delayTime; //The amount of time to delay
    
    //Get how long to delay, accounts for overflow
    if (millis() < start) {
        delayTime = time - (millis() + (4294967295-start));
    } else {
        delayTime = time - (millis() - start);
    }
    
    //Delay for delayTime as long as it's positive
    if (delayTime > 0) {
        delay(delayTime);
    }
}


/*-------------------------LORA FUNCTIONS-------------------------*/
//These were mostly written by Spencer Park


//Write config parameters to the LoRa module
//Function written by Ryan Tyrrell
void writeConfig(Stream& port, uint16_t netID, uint16_t nodeID) {

	uint8_t payloadLen = 16;
	uint8_t* payload = (uint8_t*)malloc(sizeof(uint8_t) * payloadLen);

	// Configuration flag - 2-byte short
	payload[0] = (uint8_t)((0xA5A5 >> 8) & 0xFF);
	payload[1] = (uint8_t)(0xA5A5 & 0xFF);

	// Channel Number
	payload[2] = (uint8_t)1;

	// RF transmit power (tx_power)
	payload[3] = (uint8_t)0;

	// User interface mode
	payload[4] = (uint8_t)0;

	// Equpment type
	payload[5] = (uint8_t)1;

	// Network ID - 2-byte short
	payload[6] = (uint8_t)((netID >> 8) & 0xFF);
	payload[7] = (uint8_t)(netID & 0xFF);

	// Node ID - 2-byte short
	payload[8] = (uint8_t)((nodeID >> 8) & 0xFF);
	payload[9] = (uint8_t)(nodeID & 0xFF);

	// Reserved - 2-byte short
	payload[10] = (uint8_t)((0x0000 >> 8) & 0xFF);
	payload[11] = (uint8_t)(0x0000 & 0xFF);

	// Reserved
	payload[12] = (uint8_t)1;

	// Serial port Parameter
	payload[13] = (uint8_t)0x40;

	// Air Rate - 2-byte short
	payload[14] = (uint8_t)((0x0909 >> 8) & 0xFF);
	payload[15] = (uint8_t)(0x0909 & 0xFF);


	// FrameType: 0x01		Configuration parameters for reading and writing modules, etc
	// Command Type: Write configuration information request (0x01)
	writeFrame(port, 0x01, 0x01, payloadLen, payload);

	free(payload);

	uint8_t frameType = 0;
	uint8_t cmdType = 0;
	uint8_t * responsePayload = NULL;
	int len = readFrame(port, &frameType, &cmdType, &responsePayload);

	if (frameType == 0x01 && cmdType == 0x81) {
		// Application data sending response
		uint8_t status = responsePayload[0];
		//Serial.print("status="); printByte(status);
		//Serial.println();
	}
	free(responsePayload);
}

void printByte(uint8_t b) {
  Serial.print(" 0x");
  if (b <= 0xF)
    Serial.print("0");
  Serial.print(b, HEX);
}

void printShort(uint16_t s) {
  Serial.print(" 0x");
  if (s <= 0xFFF)
    Serial.print("0");
  if (s <= 0xFF)
    Serial.print("0");
  if (s <= 0xF)
    Serial.print("0");
  Serial.print(s, HEX);
}


uint8_t readByte(Stream& port) {
  while (!port.available());
  return port.read();
}

int readFrame(Stream& port, byte* rFrameType, byte* rCmdType, byte** rPayload) {
  uint8_t checksum = 0;
  
  uint8_t frameType = readByte(port);
  uint8_t frameNum = readByte(port);
  uint8_t cmdType = readByte(port);
  uint8_t payloadLen = readByte(port);

  checksum ^= frameType;
  checksum ^= frameNum;
  checksum ^= cmdType;
  checksum ^= payloadLen;

  uint8_t* payload = (uint8_t *) malloc(sizeof(uint8_t) * payloadLen);
  for (int i = 0; i < payloadLen; i++) {
    payload[i] = readByte(port);
    checksum ^= payload[i];
  }

  *rFrameType = frameType;
  *rCmdType = cmdType;
  *rPayload = payload;

  uint8_t frameCheck = readByte(port);
  checksum ^= frameCheck;

  if (checksum != 0)
    return -1;
  return payloadLen;
}

void writeFrame(Stream& port, uint8_t frameType, uint8_t cmdType, uint8_t payloadLen, uint8_t* payload) {
  uint8_t checksum = 0;

  checksum ^= frameType;
  checksum ^= 0; // frameNum which is unused and always 0
  checksum ^= cmdType;
  checksum ^= payloadLen;

  //printByte(frameType);
  //printByte((uint8_t) 0); // frameNum
  //printByte(cmdType);
  //printByte(payloadLen);

  port.write(frameType);
  port.write((uint8_t) 0); // frameNum
  port.write(cmdType);
  port.write(payloadLen);

  for (int i = 0; i < payloadLen; i++) {
    checksum ^= payload[i];
    port.write(payload[i]);
    printByte(payload[i]);
  }

  port.write(checksum);
  //printByte(checksum);

  Serial.println();
}

uint8_t* readData(Stream& port) {
  uint8_t frameType = 0;
  uint8_t cmdType = 0;
  uint8_t* payload = NULL;
  int len = readFrame(port, &frameType, &cmdType, &payload);

  if (frameType != 0x05 || cmdType != 0x82) {
    Serial.println("BAD TYPE");
    free(payload);
    return;
  }

  uint16_t srcAddr = (payload[0] << 8) | payload[1];
  uint8_t power = payload[2];
  uint8_t userPayloadLength = payload[3];
  //Serial.print(" srcAddr="); printShort(srcAddr);
  //Serial.print(" power="); printByte(power);
  //Serial.print(" userPayloadLength="); printByte(userPayloadLength);

  uint8_t *ans = malloc(sizeof(uint8_t) * (userPayloadLength + 3));
  
  ans[0] = userPayloadLength;
  memcpy(&ans[1], &srcAddr, sizeof(uint8_t) * 2);
  for (int i = 0; i < userPayloadLength; i++) {
    ans[i+3] = payload[4+i];
    //printByte(payload[4 + i]);
  }
  //Serial.println();
  
  free(payload);
  
  return ans;
}

// Max len is 111 bytes
void sendData(Stream& port, uint16_t target, uint8_t dataLen, uint8_t* data) {
  // We add 7 bytes to the head of data for this payload
  uint8_t payloadLen = 6 + dataLen;
  uint8_t* payload = (uint8_t *) malloc(sizeof(uint8_t) * payloadLen);

  // target address as big endian short
  payload[0] = (uint8_t) ((target >> 8) & 0xFF);
  payload[1] = (uint8_t) (target & 0xFF);

  // ACK request == 1 -> require acknowledgement of recv
  payload[2] = (uint8_t) 0;//1;

  // Send radius: which defaults to max of 7 hops, we can use that
  payload[3] = (uint8_t) 7;

  // Discovery routing params == 1 -> automatic routing
  payload[4] = (uint8_t) 1;

  // Source routing domain: unused when automatic routing enabled
  //    - number of relays is 0
  //    - relay list is therefor non-existent
  //payload[5] = (uint8_t) 0;

  // Data length
  payload[5] = dataLen;

  // Data from index 7 to the end should be the data
  memcpy(payload + (sizeof(uint8_t) * 6), data, dataLen);

  // frameType = 0x05, cmdType = 0x01 for sendData
  writeFrame(port, 0x05, 0x01, payloadLen, payload);

  free(payload);

  uint8_t frameType = 0;
  uint8_t cmdType = 0;
  uint8_t* responsePayload = NULL;
  int len = readFrame(port, &frameType, &cmdType, &responsePayload);

  if (frameType == 0x5 && cmdType == 0x81) {
    // Application data sending response
    uint16_t targetAddr = (responsePayload[0] << 8) | responsePayload[1];
    uint8_t status = responsePayload[2];
    Serial.print(" targetAddr="); printShort(targetAddr);
    Serial.print(" status="); printByte(status);
    Serial.println();
  }
  free(responsePayload);
}

uint16_t readConfig(Stream& port) {
  writeFrame(port, 0x01, 0x02, 0x00, NULL);

  uint8_t frameType = 0;
  uint8_t cmdType = 0;
  uint8_t* payload = NULL;
  
  int len = readFrame(port, &frameType, &cmdType, &payload);
  
  if (len < 0) {
    Serial.println("ERROR");
    free(payload);
    return;
  }

  Serial.print("FrameType=");
  printByte(frameType);
  Serial.print(" CmdType=");
  printByte(cmdType);

  if (frameType != 0x01 || cmdType != 0x82) {
    Serial.println("BAD TYPE");
    free(payload);
    return;
  }

  uint16_t flag = (payload[0] << 8) | payload[1];
  uint8_t channel = payload[2];
  uint8_t txPower = payload[3];
  uint8_t uiMode = payload[4];
  uint8_t eqType = payload[5];
  uint16_t netId = (payload[6] << 8) | payload[7];
  uint16_t nodeId = (payload[8] << 8) | payload[9];
  // 10, 11, 12 reserved
  uint8_t serPortParam = payload[13];
  uint16_t airRate = (payload[14] << 8) | payload[15];

  Serial.print(" flag="); printShort(flag);
  Serial.print(" channel="); printByte(channel);
  Serial.print(" txPower="); printByte(txPower);
  Serial.print(" uiMode="); printByte(uiMode);
  Serial.print(" eqType="); printByte(eqType);
  Serial.print(" netId="); printShort(netId);
  Serial.print(" nodeId="); printShort(nodeId);
  Serial.print(" serPortParam="); printByte(serPortParam);
  Serial.print(" airRate="); printShort(airRate);
  Serial.println();
  return nodeId;
}