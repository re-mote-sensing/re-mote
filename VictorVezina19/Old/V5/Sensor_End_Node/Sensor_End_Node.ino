/*----------IMPORTANT INFORMATION AND CONFIG PARAMETERS---------*/

// Change the following parameters to accommodate your specific setup
#define NAME "Office"           //Name to associate with this node

#define NETWORK_ID 0x1           //LoRa network ID, has to be the same on every LoRa module in your network
#define NODE_ID 0x0              //LoRa ID of this node, must be unique to all nodes in a network
#define GATEWAY_ADDRESS 0x1000   //ID of the gateway node to send data to

#define LORA_RX 7                //Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
#define LORA_TX 6                //Pin that the LoRa RXD pin is connected to
#define GPS_RX 2                 //Pin that the GPS TXD pin is connected to
#define GPS_TX 3                 //Pin that the GPS RXD pin is connected to
#define GPS_EN 10                //Pin that the GPS EN pin is connected to

// The following paramaters have to do with the sensors you're using on this end node
// Look on our GitLab (LINK!) for more information on sensor setup and how to edit these values
#define NUMBER_SENSORS 4 //Max is 15
char* sensorTypes[NUMBER_SENSORS] = {"Dissolved_Oxygen", "Conductivity", "Turbidity", "Water_Temperature"};
uint8_t sensorPorts[NUMBER_SENSORS][2] = { {12, 11}, {9, 8}, {5, 20}, {4, 0} };


/*
Things to do:
- Split functions into driver files
- Change code structure to use event loop
- Change to NEOGPS
- Add proper error ack handling
- Change Atlas reading (add temp/salinity compensation)
- Replace all delays with sleeps
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

#include <SPI.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Time.h>
#include <TinyGPS++.h>
#include <OneWire.h>

#include <avr/sleep.h>
#include <avr/wdt.h>

/*-------------------------CONSTRUCTORS-------------------------*/

SoftwareSerial loraPort(LORA_RX, LORA_TX);

TinyGPSPlus gps;
SoftwareSerial gpsPort(GPS_RX, GPS_TX);

/*-----------------------GLOBAL VARIABLES-----------------------*/

unsigned long currTime; //The last updated unix time from the gps
unsigned long timeLastUpdated; //The millis() value that the unix time was updated
uint8_t timesRead = 0; //The amount of times the sensors have been read

float latitude;
float longitude;

float lastLat;
float lastLon;

unsigned long gpsLocTime = 60000;
unsigned long gpsTimeTime = 30000;

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
    
    pinMode(GPS_EN, OUTPUT);
    digitalWrite(GPS_EN, HIGH);
    
    gpsPort.begin(9600); //Begin the gps software serial
    
    delay(250);
    
    //For debugging where you can't get a GPS signal
    bool bypassGPS = false;
    
    if (bypassGPS) {
        latitude = 43;
        longitude = -79;
        lastLat = 0;
        lastLon = 0;
        currTime = 1561742241;
        timeLastUpdated = 0;
        gpsLocTime = 0;
        gpsTimeTime = 0;
    } else {
        gpsSetLocation(true);
        gpsSetTime(true);
    }
    
    //readSensors(); //Throw away first read, sometimes it's wrong (especially for TB)
    
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
    unsigned int i = 4;
    EEPROM.put(0, i);
    EEPROM.put(2, i - 1);
    
    delay(500);
}

/*----------------------------LOOP------------------------------*/

void loop() {
    unsigned long start = millis(); //The time this sensor read is starting
    
    digitalWrite(GPS_EN, HIGH);
    
    saveData(readSensors()); //Read the sensors and save the data to the EEPROM
    
    digitalWrite(GPS_EN, LOW);
    
    loraPort.listen();
    
    //Send the saved data from the EEPROM to the gateway
    uint8_t received = 0;
    
    //Send data until either an acknowledgement isn't received or there's no data left to send
    while (received == 0) {
        Serial.println(F("Sending"));
        received = sendSensorData();
        Serial.print(F("Received ack: "));
        Serial.println(received);
        delay(250);
    }
    
    //Delay the appropriate amount of time before the next sensor read
    if (received > 1) {
        delayOverflow(90000, start); //If sending the data wasn't successful then wait 3/4 of the full time, to offset this node from the gateway
    } else {
        delayOverflow(120000, start); //If sending tha data was successful then wait the full time
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
        } else if (strcmp(sensorTypes[i], "Water_Temperature") == 0) { //Temp
            initialiseTemp(i);
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
    
    //For some reason it seems you have to send a couple random commands at startup
    //sensor.print(F("R\r"));
    //delay(100);
    sensor.print(F("R\r"));
    
    //Wait for response and get rid of it
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;
    
    sensor.print(F("OK,1\r")); //Turn on OK responses (does nothing on older firmware)
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;
    
    sensor.print(F("C,0\r")); //Turn off continous mode
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;
    
    sensor.print(F("Plock,1\r")); //Lock to UART mode
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;
    
    /*sensor.print(F("L,0\r")); //Turn off LEDs
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;*/
    
    sensor.print(F("i\r")); //Get device info for rest of initialisation
    
    char response[30];
    
    while (sensor.available() <= 0); //Wait for it's response
    delay(100); //Delay to wait for the entire response to be ready

    int dataIndex = sensor.readBytesUntil('\r', response, 30); //Read the response
    
    //Check for D.O. sensor (different firmwares return either DO or D.O.
    if ((response[3] == 'D' && response[4] == 'O') || (response[3] == 'D' && response[4] == '.' && response[5] == 'O' && response[6] == '.')) {
        //Set proper reading outputs
        sensor.print(F("O,DO,1\r")); //For older firmware versions
        while (!(sensor.available())) ;
        delay(100);
        while (sensor.available()) sensor.read() ;
        
        sensor.print(F("O,mg,1\r"));
        while (!(sensor.available())) ;
        delay(100);
        while (sensor.available()) sensor.read() ;
        
        sensor.print(F("O,%,0\r"));
        while (!(sensor.available())) ;
        delay(100);
        while (sensor.available()) sensor.read() ;
    } else if (response[3] == 'E' && response[4] == 'C') { //EC sensor
        //Set proper probe type
        sensor.print(F("K,1.0\r"));
        while (!(sensor.available())) ;
        delay(100);
        while (sensor.available()) sensor.read() ;
        
        
        //Set proper reading outputs (Only needed for older firmware version)
        sensor.print(F("O,EC,1\r"));
        while (!(sensor.available())) ;
        delay(100);
        while (sensor.available()) sensor.read() ;
        
        sensor.print(F("O,TDS,0\r"));
        while (!(sensor.available())) ;
        delay(100);
        while (sensor.available()) sensor.read() ;
        
        sensor.print(F("O,S,0\r"));
        while (!(sensor.available())) ;
        delay(100);
        while (sensor.available()) sensor.read() ;
        
        sensor.print(F("O,SG,0\r"));
        while (!(sensor.available())) ;
        delay(100);
        while (sensor.available()) sensor.read() ;
    }
    
    sensor.print(F("Sleep\r")); //Put sensor to sleep
    
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

//Initialise a DFRobot Turbidity sensor at index i
void initialiseTemp(uint8_t i) {
    //Maybe set settings?
    
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
    unsigned long timeout = gpsTimeTime; //Time that the Arduino will try too get the time from the gps, should be parameter or define
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
        Serial.println(F("Aborting getting GPS time and date"));
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
    uint8_t arrLength = 1 + 1 + strlen(NAME);
    for (uint8_t i = 0; i < NUMBER_SENSORS; i++) {
        arrLength += 1 + strlen(sensorTypes[i]);
    }
    
    //If the length is longer than one LoRa message
    if (arrLength > 111) {
        Serial.println(F("ERROR: Name and sensor types too long for LoRa message"));
        while (true) ;
    }

    uint8_t* dataArr = malloc(sizeof(uint8_t) * arrLength); //Allocate the array for the LoRa message

    dataArr[0] = ((uint8_t) NUMBER_SENSORS) & 0b00001111; //Set first byte of message; registration type and number of sensors
    
    uint8_t curr = 1; //Current posistion in the LoRa message
    
    //Copy the length of the node name into the message
    uint8_t currLen = strlen(NAME);
    memcpy(&dataArr[curr++], &currLen, sizeof(uint8_t));
    
    //Copy the node name into the message
    for (uint8_t i = 0; i < strlen(NAME); i++) {
        memcpy(&dataArr[curr++], &NAME[i], sizeof(uint8_t));
    }
    
    //Go through each connected sensor and add it's name to the LoRa message
    for (uint8_t i = 0; i < NUMBER_SENSORS; i++) {
        //Copy the length of this sensor type into the message
        currLen = strlen(sensorTypes[i]);
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
bool gpsSetLocation(bool force) {
    float lat; //Variable for latitude
    float lon; //Variable for longitude
    
    unsigned long timeout = gpsLocTime; //Time to try the gps, should be parameter or define
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
        
    }
    
    Serial.print(F("GPS location took: "));
    Serial.println(millis() - before);
    
    if (!worked) { //If the location wasn't updated
        Serial.println(F("Aborting getting GPS location"));
        return false;
    } else { //If it was
        latitude = lat;
        longitude = lon;
        return true;
    }
}


/*-----------------Data Save Function------------------*/

//Save data to EEPROM
void saveData(uint8_t* data) {
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
    
    uint8_t size = 1 + (data[0] * 8) + (4 * (NUMBER_SENSORS + 1)); //Get the size of the data array
    
    //Loop through each byte of the current value
    for (uint8_t i = 0; i < size; i++) {
        EEPROM.update(currAddress++, data[i]); //Write current byte

        //Check for looping/double looping in the EEPROM
        if (currAddress == EEPROM.length()) {
            currAddress = 4;
            if (loop) {
                doubleLoop = true;
            } else {
                loop = true;
            }
        }
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
uint8_t* readSensors() {
    Serial.println(F("\nReading Sensors"));
    
    //Try to update time and location
    gpsSetLocation(false);
    gpsSetTime(false);
    
    uint8_t* ans; //Array that will be returned
    uint8_t curr = 0; //Current position in the array
    
    //Check if latitude or longitude has changed enough since it was last sent
    if (abs(latitude - lastLat) > 0.00005 || abs(longitude - lastLon) > 0.00005) {
        //Update last sent latitude and longitude
        lastLat = latitude;
        lastLon = longitude;
        
        //Allocate memory for answer, including latitude and longitude
        ans = malloc(sizeof(uint8_t) * (1 + (4 * (NUMBER_SENSORS + 3))));
        
        ans[curr++] = 1; //Means that this data point has location data
        
        //Copy time into array
        unsigned long time = gpsGetTime();
        memcpy(&ans[curr], &time, sizeof(uint8_t) * 4);
        curr +=4 ;
        
        //Copy location into array
        memcpy(&ans[curr], &latitude, sizeof(uint8_t) * 4);
        memcpy(&ans[curr + 4], &longitude, sizeof(uint8_t) * 4);
        curr += 8;
    } else {
        //Allocate memory for answer, not including latitude and longitude
        ans = malloc(sizeof(uint8_t) * (1 + (4 * (NUMBER_SENSORS + 1))));
        
        ans[curr++] = 0; //Means that this data point doesn't have location data
        
        //Copy time into array
        unsigned long time = gpsGetTime();
        memcpy(&ans[curr], &time, sizeof(uint8_t) * 4);
        curr +=4 ;
    }
    
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
        } else if (strcmp(sensorTypes[i], "Water_Temperature") == 0) { //DFRobot Temperature
            data = readTemp(i);
        } else { //Should probably handle this better
            data = -1;
        }
        
        //Copy this sensor's data into the data array
        memcpy(&ans[curr], &data, sizeof(uint8_t) * 4);
        curr += 4;
    }
    
    return ans;
}

//Read data from an Atlas Scientific sensor at index i
float readAS(uint8_t i) {
    //Compensate for temp(both) and salinity(DO)??
    
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
    Serial.println(buf);
    
    //Put sensor to sleep
    sensor.print(F("Sleep\r"));
    
    sensor.end();
    
    return data;
}

//Read data from a DFRobot turbidity sensor at index i
float readTB(uint8_t i) {
    digitalWrite(sensorPorts[i][0], HIGH); //Turn on the sensor
    delay(250);
    
    float data = analogRead(sensorPorts[i][1]); //Read the sensor
    
    //Do some manipulation to the data to get a more "accurate" number (just something that looks more correct)
    float ans;
    if (data < 775.0) {
        ans = 775.0 - data;
    } else {
        int factor = data / 5;
        ans = data - (5.0 * factor);
    }
    
    digitalWrite(sensorPorts[i][0], LOW); //Turn off the sensor
    
    //Debugging print statements
    Serial.print(F("Read TB sensor: "));
    Serial.println(ans);
    
    return ans;
}

//Read data from a DFRobot turbidity sensor at index i
float readTemp(uint8_t i) {
    OneWire tempSensor(sensorPorts[i][0]); //Start One Wire interface
    
    uint8_t addr[8];
    
    //Get the address of the sensor
    do {
        if (!tempSensor.search(addr)) {
            Serial.println(F("Didnt find temperature sensor"));
            return -1;
        }
    } while (!(addr[0] == 16 || addr[0] == 40));
    
    //Make sure the CRC is valid
    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println(F("CRC not valid"));
        return -1;
    }
    
    //Read temperature command
    tempSensor.reset();
    tempSensor.select(addr);
    tempSensor.write(0x44, 1);
    
    float ans;
    
    delay(750); //Temperature query time
    
    //Read the sensor's value until it's valid (conversion is done)
    do {
        //Read saved temperature value command
        tempSensor.reset();
        tempSensor.select(addr);
        tempSensor.write(0xBE);
        
        uint8_t data[2];
        
        //Get sensor's value
        for (uint8_t i = 0; i < 2; i++) {
            data[i] = tempSensor.read();
        }
        
        //Convert into deg C
        float tmp = ((data[1] << 8) | data[0]);
        ans = tmp / 16;
    } while (ans == 85); //It returns 85 if the conversion isn't yet finished
    
    //Debugging print statements
    Serial.print(F("Read Water Temp sensor: "));
    Serial.println(ans);
    
    return ans;
}


/*---------------Send Sensor Data Function----------------*/

//Sends saved data from EEPROM through LoRa
uint8_t sendSensorData() {
    //Get EEPROM address of last data point
    unsigned int validToAddress = 4;
    EEPROM.get(0, validToAddress);
    
    //Get EEPROM address of where valid data starts
    unsigned int currAddress = 3;
    EEPROM.get(2, currAddress);
    
    if (currAddress == 3) { //Default state, means no data
        return 1;
    }
    
    uint8_t* dataInfo = getDataMessageInfo(validToAddress, currAddress); //Get info for making LoRa data message
    
    uint8_t* dataArr = malloc(sizeof(uint8_t) * dataInfo[0]); //Allocate the array for the LoRa message
    
    dataArr[0] = 0b00010000 | ((uint8_t) NUMBER_SENSORS & 0b00001111); //Set the first byte of the message; the message type and the number of sensors
    dataArr[1] = dataInfo[1]; //Number of locations
    
    //Variables for keeping track of important message information
    uint8_t currLocPtr = 2;
    uint8_t currDataPtr = 2 + dataInfo[1];
    uint8_t currDataNum = 0;
    
    //Read data from EEPROM into the LoRa message
    while (currDataPtr < dataInfo[0]) {
        currDataNum++; //Reading new data point
        
        uint8_t hasLoc = EEPROM.read(currAddress++); //Has location byte
        
        if (currAddress > EEPROM.length()) { //Check EEPROM loop
            currAddress -= (EEPROM.length() - 4);
        }
        
        readEEPROM(dataArr, &currDataPtr, &currAddress, 4); //Read time
        
        if (hasLoc) {
            readEEPROM(dataArr, &currDataPtr, &currAddress, 8); //Read location
            dataArr[currLocPtr++] = currDataNum; //Put data point number into locations part of message
        }
        
        readEEPROM(dataArr, &currDataPtr, &currAddress, 4 * NUMBER_SENSORS); //Read sensor data
    }
    
    sendData(loraPort, GATEWAY_ADDRESS, dataInfo[0], dataArr); //Send data to the gateway
    
    //Free allocated memory
    free(dataInfo);
    free(dataArr);

    uint8_t ack = ackWait(loraPort, GATEWAY_ADDRESS, 5000); //Wait for acknowledgement
    
    if (ack == 0) { //If it receives a success acknowledgement
        
        if (currAddress == validToAddress) { //If all valid data has been read, reset the addresses
            unsigned int address = 4;
            EEPROM.put(0, address);
            EEPROM.put(2, address - 1);
        } else {
            EEPROM.put(2, currAddress); //Set the address of the last data point correctly
        }
        
        return 0;
        
    } else { //Should handle different failure codes correctly
        return 0xFF;
    }
}

//Gets the info of a data LoRa message
uint8_t* getDataMessageInfo(unsigned int validToAddress, unsigned int validFromAddress){
    uint8_t* ans = calloc(2, sizeof(uint8_t));
    unsigned int curr = validFromAddress;
    uint8_t size = 2; //Size of message
    uint8_t lastSize = 2; //Last valid size
    
    //Go through data to find max size that can be sent
    while (size <= 111 && curr != validToAddress) {
        lastSize = size;
        
        //Has location byte
        uint8_t hasLocation = EEPROM.read(curr++);
        
        //Time
        curr += 4;
        size += 4;
        
        if (hasLocation) {
            //Location
            curr += 8;
            size += 9;
            
            ans[1]++; //Number of locations counter
        }
        
        //Sensor data
        curr += 4 * NUMBER_SENSORS;
        size += 4 * NUMBER_SENSORS;
        
        if (curr >= EEPROM.length()) { //EEPROM loop check
            curr -= (EEPROM.length() - 4);
        }
    }
    
    if (size <= 111) { //Check what caused loop exit
        ans[0] = size;
    } else {
        ans[0] = lastSize;
    }
    
    return ans;
}

//Read numBytes from EEPROM starting at *memAddress into message starting at *messageIndex, incrementing everything accordingly and checking for EEPROM looping
void readEEPROM(uint8_t* message, uint8_t* messageIndex, unsigned int* memAddress, uint8_t numBytes) {
    for (uint8_t i = 0; i < numBytes; i++) {
        message[(*messageIndex)++] = EEPROM.read((*memAddress)++);

        if (*memAddress > EEPROM.length()) {
            *memAddress -= (EEPROM.length() - 4);
        }
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
    long delayTime; //The amount of time to delay
    
    //Get how long to delay, accounts for overflow
    if (millis() < start) {
        delayTime = time - (millis() + (4294967295-start));
    } else {
        delayTime = time - (millis() - start);
    }
    
    //Don't try to sleep for a negative amount of time
    if (delayTime < 0) {
        return;
    }
    
    unsigned int sleepNum = delayTime/8000; //Amount of times to sleep
    sleepNum += (delayTime - (8000 * sleepNum)) >= 4000; //Essentially just rounding
    
    //Sleep setup
    byte prevADCSRA = ADCSRA;
    ADCSRA = 0;
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    
    //Sleep loop
    for (unsigned int i = 0; i < sleepNum; i++) {
        // Turn of Brown Out Detection (low voltage). 
        //This is automatically re-enabled upon timer interrupt
        sleep_bod_disable();  //BOD is for Low Voltage Protection & resets the chip

        // Ensures we can wake up again by first disabling interrupts (temporarily) so
        // the wakeISR does not run before we are asleep and then prevent interrupts,
        // and then defining the ISR (Interrupt Service Routine) to run -
        // when poked awake by the timer
        noInterrupts();

        // clear various "reset" flags
        MCUSR = 0;  // allow changes, disable reset
        WDTCSR = bit (WDCE) | bit(WDE); // set interrupt mode and an interval
        WDTCSR = bit (WDIE) | bit (WDP3) /*| bit(WDP2) | bit(WDP1) */| bit(WDP0);    
        // now set to 8 seconds delay, increase the counter to elongate sleep time
        wdt_reset();

        // Allow interrupts now
        interrupts();

        // And enter sleep mode as set above
        sleep_cpu();
    }
    
    //Go back to normal
    sleep_disable();
    ADCSRA = prevADCSRA;
    
    //Because millis() isn't updated during sleeping, change the time the last GPS was updated
    timeLastUpdated -= sleepNum * 8000;
    
    return;
}

// When WatchDog timer causes Arduino to wake it comes here
ISR (WDT_vect) {
  wdt_disable(); //Turn off watchdog
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