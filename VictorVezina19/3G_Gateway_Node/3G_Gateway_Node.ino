/*----------IMPORTANT INFORMATION AND CONFIG PARAMETERS---------*/

// You have to press the power button on the 3G shield when you power the Arduino

// Change the following parameters to accommodate your specific setup
#define NETWORK_ID 0x1   //LoRa network ID, has to be the same on every LoRa module in your network
#define NODE_ID 0x1000   //LoRa ID of this node, you then need to put this into the end nodes that are sending to this gateway
#define LORA_RX 6      //Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
#define LORA_TX 7      //Pin that the LoRa RXD pin is connected to


/*
Things to do:
- Make location part of sensor data
- Make name part of registration
- Change data reuqest to not include "Time"
- Change HTTP requests to not be hardcoded
*/


/*--------------------------------------------------------------*/
/*-----------------------------CODE-----------------------------*/
/*--------------------------------------------------------------*/

/*---------------------------INCLUDES---------------------------*/

#include <SPI.h>
#include <SoftwareSerial.h>
#include <SdFat.h>

#include <MemoryFree.h>

/*----------------------DEFINITIONS-----------------------------*/

#define FONA_TX 4
#define FONA_RX 5

/*-------------------------GLOBALS------------------------------*/

unsigned long lastPost;

/*------------------------CONSTRUCTORS--------------------------*/

SoftwareSerial loraPort(LORA_RX, LORA_TX);

/*---------------------------SETUP------------------------------*/

void setup() {
    //Begin some serials
    loraPort.begin(9600);
    Serial.begin(9600);

    //All these freeMemory() calls are for testing, you can just ignore them
    Serial.print(F("\n\n1: "));
    Serial.println(freeMemory());

    //Write the LoRa configuration from the defines to the LoRa module
    writeConfig(loraPort, NETWORK_ID, NODE_ID);
    
    delay(1000);

    Serial.print(F("1.1: "));
    Serial.println(freeMemory());
    
    //while (!Serial.available()) ; //Useful for testing

    Serial.print(F("2: "));
    Serial.println(freeMemory());

    //Make sure ToSend.csv exists
    createToSend();

    Serial.print(F("2.6: "));
    Serial.println(freeMemory());
    
    //Pretend like the last post just happened
    lastPost = millis();
}

/*----------------------------LOOP------------------------------*/

void loop() {
    loraPort.listen(); //Listen on the LoRa module software serial
    Serial.println(F("Waiting for LoRa messages"));
    
    Serial.print(F("3: "));
    Serial.println(freeMemory());

    //Just keep checking the LoRa module until the right amount of time has passed
    while ((millis() >= lastPost) ? ((millis() - lastPost) < 60000) : ((millis() + (4294967295 - lastPost)) < 60000 )) { //Makes sure to check for overflow
        if (loraPort.available()) { //If the LoRa module received a message

            uint8_t* loraData = readData(loraPort); //Read the data from the LoRa module

            //Print the data to the Serial interface
            Serial.println(F("Received LoRa message"));
            printByte(loraData[0]);
            printByte(loraData[1]);
            printByte(loraData[2]);
            for (int i = 0; i <= loraData[2]; i++) {
                printByte(loraData[i+3]);
            }
            Serial.println();

            //Once we encrypt data, it will be decrypted here

            uint8_t type = loraData[3] >> 4; //Get the type of the message
            switch (type) { //Handle the message based on the type
                case 0: //Registration message
                    parseRegistration(loraData);
                    break;

                case 1: //Data message
                    saveData(loraData);
                    break;
                    
                case 2: //Location message
                    sendLocation(loraData);
                    break;

                default: //Unknown message
                    Serial.println(F("Unknown Message Type Received"));
            }

            free(loraData); //Free allocated message memory
        }
    }
    Serial.println(F("Posting data"));
    
    Serial.print(F("8: "));
    Serial.println(freeMemory());
    
    lastPost = millis(); //Get post time before we actually post, this ensures that the posting starts exactly every hour
    postData(); //Post the saved data
}

/*------------------------ALL FUNCTIONS-------------------------*/


/*------------ToSend.csv Initialisation Function------------*/
//Create "ToSend.csv" if it doesn't already exist
void createToSend() {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    //If ToSend.csv doesn't already exist, create it with the appropriate starting line
    if (!sd.exists("ToSend.csv")) {
        Serial.println(F("Creating ToSend.csv"));
        File file = sd.open("ToSend.csv", FILE_WRITE);
        file.print(F("Node ID, Data Points, Position\n"));
        file.close();
    }
}


/*---------------Node Registration Function---------------*/
//Parse registration message and save new node information
void parseRegistration(uint8_t* data) {
    uint8_t numSensors = data[3] & 0x0F; //Get number of sensors in registration
    uint8_t curr = 4; //Used for going through the registration message data
    char *types[numSensors]; //Create array of char arrays for the sensor types
    
    for (uint8_t i = 0; i < numSensors; i++) { //Get the sensor types from the message
        uint8_t len = data[curr++]; //Get length of type
        char *currType = malloc(sizeof(char) * (len + 1)); //Allocate current char array
        currType[len] = 0;
        for (uint8_t j = 0; j < len; j++) { //Copy type into char array
	        currType[j] = data[curr++];
        }
        types[i] = currType; //Point array of char arrays to current char array
    }

    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }

    //Get id of node that sent the registration
    uint16_t add;
    memcpy(&add, data, sizeof(uint8_t) * 2);
    
    //Get the file name for that node
    char* fileName = malloc(sizeof(char) * 13);
    sprintf(fileName, "node%u.csv", add);

    if (sd.exists(fileName)) { //Receiving registration from already registered node, check to see if good or not
        File file = sd.open(fileName, FILE_READ);
        file.seekSet(5);
        uint8_t currTypeNum = 0;
        bool err = false;
        for (uint8_t i = 0; i < numSensors; i++) { //Go through sensor types and make sure they match the registration message
            uint8_t currByte = file.read();
            if (currByte == 44) {
                currTypeNum++;
                i = -1;
            } else if (currByte != types[currTypeNum][i]) {
                err = true;
                break;
            }
        }
        file.close();
        
        if (err) { //If the registration is bad
            //Should send error acknowledgement back
            Serial.print(F("Received bad registration data from node "));
            Serial.println(add);
        } else { //If the registration is good
            //Send success acknoledgement back
            uint8_t* ackData = malloc(sizeof(uint8_t));
            ackData[0] = 0;
            sendData(loraPort, add, 1, ackData);
            free(ackData);
        }
    } else { //Registering new node
        File file = sd.open(fileName, FILE_WRITE);
        file.print(F("Time,")); //The first column is always time
        for (uint8_t i = 0; i < numSensors; i++) { //Print the sensor types into the file
            file.print(types[i]);
            if ((i+1) != numSensors) {
                file.print(F(","));
            }
        }
        file.print(F("\n"));
        file.close();
        
        //Send success acknowledgement back
        uint8_t* ackData = malloc(sizeof(uint8_t));
        ackData[0] = 0;
        sendData(loraPort, add, 1, ackData);
        free(ackData);
    }

    //Free allocated memory
    for (uint8_t i = 0; i < numSensors; i++) {
        free(types[i]);
    }
    free(fileName);
}


/*------------------Data Saving Functions------------------*/
//Save the received data into the sd card
void saveData(uint8_t* data) {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }

    //Get id of node that sent the registration
    uint16_t add;
    memcpy(&add, data, sizeof(uint8_t) * 2);
    
    //Get the file name for that node
    char* fileName = malloc(sizeof(char) * 13);
    sprintf(fileName, "node%u.csv", add);
    
    //Get some useful information from the message data
    uint8_t payloadLen = data[2];
    uint8_t numSensors = data[3] & 0x0F;
    
    if (!sd.exists(fileName)) { //If the node hasn't yet registered with this gateway
        //Should send error acknowledgement
        Serial.print(F("Received sensor data before registration from node "));
        Serial.println(add);
    } else {
        File file = sd.open(fileName, FILE_WRITE);
        //Check the data to make sure it matches the node's registration
        if (!checkSensorData(file, numSensors)) { //If it doesn't match
            //Should send error acknowledgement
            Serial.print(F("Received bad sensor data from node "));
            Serial.println(add);
        } else { //If it does match
            uint32_t position = file.curPosition(); //Get the position before the new data in the node's file
            
            //Add the new data points to the node's file
            uint8_t numData = 0;
            for (uint8_t i = 4; i < payloadLen + 3; ) { //While there's still data in the message
                numData++; //Keeps track of how many data points are being added
                
                //Get the time for the current data points
                unsigned long time;
                memcpy(&time, &data[i], sizeof(uint8_t) * 4);
                
                i += 4; //Make sure the index for the message data in being augmented
                
                //Print the time into the file
                file.print(time);
                file.print(F(","));
                
                for (uint8_t j = 0; j < numSensors; j++) { //Iterate through the value for each sensor
                    //Get the value of the current sensor
                    float currFloat;
                    memcpy(&currFloat, &data[i], sizeof(uint8_t) * 4);
                    
                    i += 4;
                    
                    //Print the current sensor value into the file
                    file.print(currFloat);
                    
                    if (j+1 != numSensors) { //Print a , if it's not the last sensor
                        file.print(F(","));
                    }
                }
                file.print("\n"); //Finish the line
            }

            //See if the node already has data to be sent in ToSend.csv
            File toSendFile = sd.open("ToSend.csv", FILE_WRITE);
            toSendFile.seekSet(31);
            
            bool notFound = false;
            uint16_t currId;
            do { //While the node id on the current line isn't the node id we're looking for
                if (!toSendFile.available()) { //If we reach the end of ToSend.csv without finding the id of the node
                    notFound = true;
                    break;
                }
                //Get the id of the node on the current line
                char currStr[12];
                int len = toSendFile.fgets(currStr, 12, ",");
                currStr[len-2] = 0;
                currId = atoi(currStr);

                while (toSendFile.read() != 10); //Go to end of line
            } while (currId != add);

            if (notFound) { //If the node isn't already in ToSend.csv
                //Print the node's id into the ToSend file
                toSendFile.print(add);
                toSendFile.print(F(","));
                
                //Print the number of data points into theToSend  file
                char numDataStr[4];
                sprintf(numDataStr, "%.3hu", numData);
                toSendFile.print(numDataStr);
                toSendFile.print(F(","));
                
                //Print the cursor position of the node's file into the ToSend file
                toSendFile.print(position);
                toSendFile.print("\n");
            } else { //If the node is already in ToSend.csv
                for (uint8_t i = 0; i < 2; i++) { //Back up until it's before the number of data points
                    toSendFile.seekCur(-1);
                    while (toSendFile.peek() != 44) { // ','
                        toSendFile.seekCur(-1);
                    }
                }
                
                toSendFile.read(); //Get rid of the ,
                
                //Position will now be about to read number of data points
                
                //Read the current amount of data points ready to be sent for the current node
                char currStr[12];
                int len = toSendFile.fgets(currStr, 12, ",");
                currStr[len-1] = 0; //Overwrite ','
                uint8_t numPoints = atoi(currStr);
                
                //Go back until it's before the number of data points
                toSendFile.seekCur(-2);
                while (toSendFile.peek() != 44) { // ','
                    toSendFile.seekCur(-1);
                }
                toSendFile.read();
                
                //Position will now be about to write number of data points
                
                //Write the new number of data points
                char numPointsStr[4];
                sprintf(numPointsStr, "%.3hu", numPoints + numData);
                toSendFile.print(numPointsStr);
            }

            toSendFile.close(); //Close ToSend.csv
            
            //Send a success acknowledgement
            uint8_t* ackData = malloc(sizeof(uint8_t));
            ackData[0] = 0;
            sendData(loraPort, add, 1, ackData);
            free(ackData);
        }
        file.close(); //Close the node's file
    }
    free(fileName); //Free allocated memory
}

//Check that the number of sensors given is the same as what's in the node file
bool checkSensorData(File file, uint8_t numSensors) {
    uint32_t position = file.curPosition(); //Get current position so that the file isn't changed by this function
    
    file.seekSet(0); //Go to the begining of the file
    
    //Count the number of sensors in the given file
    //Technically counts Time (which isn't included in numSensors), but misses the last one, so the count comes out right
    uint8_t sum = 0;
    while (file.peek() != 10) { //While it's not a newline character
        if (file.read() == 44) { //If it's a comma
            sum++;
        }
    }
    
    if (sum != numSensors) { //If the number of sensors in the file isn't the same as the given number
        return false;
    } else { //If it is
        file.seek(position);
        return true;
    }
}


/*----------------Location Handling Functions----------------*/

//Didn't bother commenting this cause it needs heavy changing
//The location data should maybe just be sent with every data point, and the name with the registration

//Sends the location of an end node to the server
void sendLocation(uint8_t* data) {
    uint16_t address;
    memcpy(&address, data, sizeof(uint8_t) * 2);
    
    //Send first acknowledgement to show that it's being worked on
    uint8_t* ackData = malloc(sizeof(uint8_t));
    ackData[0] = 1;
    sendData(loraPort, address, 1, ackData);
    free(ackData);
    
    uint32_t time;
    float lat;
    float lon;
    
    memcpy(&time, &data[4], sizeof(uint8_t) * 4);
    memcpy(&lat, &data[8], sizeof(uint8_t) * 4);
    memcpy(&lon, &data[12], sizeof(uint8_t) * 4);
    
    char* latStr = malloc(sizeof(char) * 11);
    dtostrf(lat, 8, 6, latStr);
    char* lonStr = malloc(sizeof(char) * 12);
    dtostrf(lon, 8, 6, lonStr);
    
    uint8_t nameLen = data[16];
    char* name = malloc(sizeof(char) * (nameLen + 1));

    Serial.print(F("11: "));
    Serial.println(freeMemory());
    
    for (uint8_t i = 0; i < nameLen; i++) {
        name[i] = data[17 + i];
    }
    name[nameLen] = 0;
    
    char* request = malloc(sizeof(char) * (127 + nameLen));

    Serial.print(F("12: "));
    Serial.println(freeMemory());
    
    sprintf(request, "POST /ollie/sensor/info?id=%d&name=%s&coords=%s,%s&time=%lu HTTPS/1.1\r\nHost: www.cas.mcmaster.ca\r\n\r\n", address, name, latStr, lonStr, time);

    free(latStr);
    free(lonStr);
    free(name);
    
    Serial.print(F("Time: "));
    Serial.println(time);
    
    Serial.print(F("13: "));
    Serial.println(freeMemory());
    
    bool error = fonaPost(request);

    Serial.print(F("14: "));
    Serial.println(freeMemory());
    
    free(request);
    
    loraPort.listen();
    
    ackData = malloc(sizeof(uint8_t));

    Serial.print(F("15: "));
    Serial.println(freeMemory());
    
    if (!error) {
        ackData[0] = 0;
    } else {
        ackData[0] = 0xFF;
    }

    Serial.print(F("16: "));
    Serial.println(freeMemory());
    
    sendData(loraPort, address, 1, ackData);
    
    free(ackData);

    Serial.print(F("17: "));
    Serial.println(freeMemory());
}


/*--------------Saved Data Posting Functions--------------*/
//Post collected data to webserver
void postData() {
    Serial.print(F("10: "));
    Serial.println(freeMemory());

    //Build HTTP request
    char* reqArr = buildRequest();
    
    //Makes sure the request isn't empty
    if (reqArr == NULL) {
        return;
    }
    
    Serial.print(F("12: "));
    Serial.println(freeMemory());

    //Post the request through the FONA
    bool error = fonaPost(reqArr);
    
    //Free the allocaated memory
    free(reqArr);
    
    if (!error) { //If it posted correctly, truncate ToSend.csv (all the data that needed to be sent was sent)
        truncateToSend();
    }
}

//Build HTTP request to send through fona
char* buildRequest() {
    Serial.print(F("11.1: "));
    Serial.println(freeMemory());
    
    uint8_t* data = getAllData(); //Get the data to send in the request
    
    //If there's no data to send, return NULL
    if (data == NULL) {
        return NULL;
    }
    
    Serial.print(F("11.2: "));
    Serial.println(freeMemory());
    
    //Get the size of the data
    unsigned int dataSize;
    memcpy(&dataSize, data, sizeof(uint8_t) * 2);
    dataSize -= 2; //Don't need to account for the bytes taken by dataSize
    
    Serial.print(F("Data size: ")); //For debugging
    Serial.println(dataSize);
    
    //Stuff here needs to be changed; the URL shouldn't be hardcoded
    
    uint8_t size = 70; //Total size of the hardcoded request
    
    char* request = malloc(sizeof(char) * (size + (2 * dataSize) + 1)); //Allocate the char array for the request
    
    if (request == NULL) { //If the arduino doesn't have enough RAM for the request
        Serial.println(F("Ran out of memory while building request"));
        return NULL;
    }
    
    Serial.print(F("11.3: "));
    Serial.println(freeMemory());
    
    strcpy(request, "POST /ollie/sensor/data?data="); //The first part of the HTTP request
    
    Serial.print(F("11.4: "));
    Serial.println(freeMemory());
    
    for (unsigned int i = 0; i < dataSize; i++) { //Go through each byte of data, convert it into hex, and add that to the request
        char* curr = byteToHexStr(data[i + 2]);
        request[(2 * i) + 29] = curr[0];
        request[(2 * i) + 29 + 1] = curr[1];
        free(curr);
    }
    
    Serial.print(F("11.5: "));
    Serial.println(freeMemory());
    
    strcpy(&request[29 + (2 * dataSize)], " HTTPS/1.1\r\nHost: www.cas.mcmaster.ca\r\n\r\n\0"); //The last part of the HTTP request
    
    free(data); //Free the allocated memory for the data
    
    Serial.print(F("11.6: "));
    Serial.println(freeMemory());
    
    return request;
}

//Converts a uint8_t (byte) into a char* representing the hexadecimal form of the byte
char* byteToHexStr(uint8_t data) {
    char* ans = malloc(sizeof(char) * 2); //Allocate the char array
    
    sprintf(ans, "%02X", data); //Print the byte of data as hex into the char array, always aking up two characters
    return ans;
}

//Get data for all nodes for HTTP request
uint8_t* getAllData() {
    Serial.print(F("11.11: "));
    Serial.println(freeMemory());
    
    //Get some information on what needs to be sent (used to get the size of the data to be sent)
    //The returned array has the form(the numbers in the brackets is number of bytes): #nodes(1) id1(2) #points1(1) pos1(4) id2(2) ...
    uint8_t* sendInfo = getSendInfo();
    
    //If there's no data to be sent, return NULL
    if (sendInfo[0] == 0) {
        free(sendInfo);
        return NULL;
    }
    
    Serial.print(F("11.12: "));
    Serial.println(freeMemory());
    
    //Keep track of the total size of the final data array
    unsigned int totalLen = 3; //2 for data size (just used in converting to string), 1 for # of nodes
    
    for (uint8_t i = 0; i < sendInfo[0]; i++) { //Go through each node that has data to send
        //Get the node's id
        int currId;
        memcpy(&currId, &sendInfo[1 + (7 * i)], sizeof(uint8_t) * 2);
        
        totalLen += 2; //For the node's id
        
        //Gets the type info for the current node
        uint8_t* currTypesInfo = malloc(sizeof(uint8_t) * 2); //Allocate the array to put the info into
        getTypesInfo(currTypesInfo, currId); //First byte is total size that will be taken in final data array, second byte is just number of sensors
        
        totalLen += currTypesInfo[0]; //Size of sensor types information
        totalLen += 1; //For # data points
        
        //Account for size of data points
        //Essentially doing: totalLen += #DataPoints * #Types * 4 bytes
        totalLen += sendInfo[3 + (7 * i)] * currTypesInfo[1] * 4;
        
        free(currTypesInfo); //Free allocated memory
    }
    
    Serial.print(F("11.13: "));
    Serial.println(freeMemory());
    
    Serial.println(totalLen);
    
    //Allocate the final data array
    uint8_t* ans = malloc(sizeof(uint8_t) * totalLen);
    
    ans[2] = sendInfo[0]; //Number of nodes
    
    Serial.print(F("11.14: "));
    Serial.println(freeMemory());
    
    //Go through each node and get the data for the data points it needs to send
    unsigned int curr = 3;
    for (uint8_t i = 0; i < sendInfo[0]; i++) {
        //Get current node id
        int currId;
        memcpy(&currId, &sendInfo[1 + (7 * i)], sizeof(uint8_t) * 2);
        
        //Get postition in the node's file
        unsigned long currPos;
        memcpy(&currPos, &sendInfo[4 + (7 * i)], sizeof(uint8_t) * 4);
        
        //Gets the data for the current node and puts it into the ans array starting at curr, increments curr accordingly
        getNodeData(ans, &curr, currId, currPos);
    }
    
    free(sendInfo); //Free allocated memory
    
    memcpy(ans, &curr, sizeof(uint8_t) * 2); //Copy in data size onto the data array
    
    Serial.print(F("11.15: "));
    Serial.println(freeMemory());
    
    return ans;
}

//Get the information on what data needs to be sent
//The returned array has the form(the numbers in the brackets is number of bytes): #nodes(1) id1(2) #points1(1) pos1(4) id2(2) ...
uint8_t* getSendInfo() {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    //Open ToSend.csv and skip the first line
    File file = sd.open("ToSend.csv", FILE_READ);
    file.seekSet(31);
    
    //Get the number of nodes that have data to send
    uint8_t numNodes = 0;
    while (file.available()) { //Go through the entire file
        while (file.read() != 10); //Go to end of line
        numNodes++;
    }
    
    file.seekSet(31); //Go back to right after the first line
    
    uint8_t* ans = malloc(sizeof(uint8_t) * (1 + (7 * numNodes))); //Allocate the array to return
    
    ans[0] = numNodes; //The first byte is the number of nodes
    
    for (uint8_t i = 0; i < numNodes; i++) { //Go through each node, and get it's corresponding information
        char currStr[12];
        
        //Get the current node's id
        int len = file.fgets(currStr, 12, ",");
        currStr[len-1] = 0; //Overwrite ','
        uint16_t currId = atoi(currStr);
        
        //Get the number of data points that the current node has to send
        len = file.fgets(currStr, 12, ",");
        currStr[len-1] = 0; //Overwrite ','
        uint8_t numPoints = atoi(currStr);
        
        //Get the postition in the current node's file
        len = file.fgets(currStr, 12, "\n");
        currStr[len-1] = 0; //Overwrite '\n'
        uint32_t currPos = atol(currStr);
        
        //Put the gathered info into the array to return
        memcpy(&ans[1 + (7 * i)], &currId, sizeof(uint8_t) * 2);
        ans[3 + (7 * i)] = numPoints;
        memcpy(&ans[4 + (7 * i)], &currPos, sizeof(uint8_t) * 4);
    }
    
    file.close();
    
    return ans;
}

//Get some information of the sensor types of a specific node
//First byte is total size that will be taken in final data array, second byte is just number of sensors
void getTypesInfo(uint8_t* ans, int id) {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    //Open the given node's file
    char fileName[13];
    sprintf(fileName, "node%u.csv", id);
    File file = sd.open(fileName, FILE_READ);
    
    char currStr[12];
    
    //Keep track of the total size of the sensor types
    uint8_t typesSize = 1; //Add 1 to account for number of types
    
    //Keep track of how many sensor types there are
    uint8_t numTypes = 1; //Won't count the last type, so add 1
    
    while (file.peek() != 10) { //While it's not the end of the line
        if (file.read() != 44) { //If it's a comma
            typesSize++;
        } else {
            numTypes++;
        }
    }
    file.close();
    
    typesSize += numTypes; //For length of each sensor type
    
    //Put the data into the passed in array
    ans[0] = typesSize;
    ans[1] = numTypes;
    
    return;
}

//Gets the data to be sent from a specific node
//Form of data: Id(2) NumTypes(1) Type1Len(1) Type1Char1(1) Type1Char2(1) ... Type1CharN(1) Type2Len(1) Type2Char1(1) ... Type2CharN(1) NumData(1) Data1Type1(4) Data1Type2(4) ... Data1TypeN(4) Data2Type1(4) ... DataNTypeN(4)
void getNodeData(uint8_t* ans, unsigned int* ansCurr, int id, unsigned long pos) {
    //Currently this copies in Time as a type, but that's just a waste of space

    Serial.print(F("Getting data for node: "));
    Serial.println(id);
    
    //Copy the current node's id into the answer array
    memcpy(&ans[*ansCurr], &id, sizeof(uint8_t) * 2);
    (*ansCurr) += 2;
    
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    //Open the current node's file
    char fileName[13];
    sprintf(fileName, "node%u.csv", id);
    File file = sd.open(fileName, FILE_READ);
    file.seekSet(0); //Should skip time
    
    Serial.print(F("11.14201: "));
    Serial.println(freeMemory());
    
    //Variables needed to copy in the sensor type names
    unsigned int numPos = *ansCurr; //Posistion where the total number of types needs to go
    uint8_t currTypeLen = 0; //Length of the current sensor type
    uint8_t currNum = 1; //Number of sensor types (won't count the last one, so add one)
    (*ansCurr) += 2;
    
    while (file.peek() != 10) { //While it's not the end of the line
        if (file.peek() == 44) { //If it's a comma
            ans[((*ansCurr)++) - (currTypeLen + 1)] = currTypeLen; //Put in the length of the type name that was just copied in
            currNum++;
            currTypeLen = 0;
            file.read(); //Skip the comma
        } else { //Not a comma (some letter)
            ans[(*ansCurr)++] = file.read(); //Copy the letter into the answer array
            currTypeLen++; //Increase the length of the current sensor type
        }
    }
    
    ans[(*ansCurr) - (currTypeLen + 1)] = currTypeLen; //Put in the length of the type name that was just copied in
    ans[numPos] = currNum; //Put in the total number of types that was copied in
    
    Serial.print(F("11.14202: "));
    Serial.println(freeMemory());
    
    file.seekSet(pos); //Go to the position where the data points start
    numPos = *ansCurr; //Postition for the number of data points
    currNum = 0; //Number of data points
    (*ansCurr)++;
    
    //Go through each data point
    while (file.available()) { //While it's not the end of the file
        char dataStr[13];
        
        //Get the time for the curr data point
        uint8_t len = file.fgets(dataStr, 13, ",");
        dataStr[len-1] = 0; //Overwrite ','
        unsigned long currTime = strtoul(dataStr, NULL, 10);
        
        Serial.print(F("TIME: "));
        Serial.println(currTime);
        
        //Put the current time into the answer array
        memcpy(&ans[*ansCurr], &currTime, sizeof(uint8_t) * 4);
        (*ansCurr) += 4;
        
        float currData; //Value of current sensor value
        uint8_t currDataStr = 0; //Index for dataStr
        
        //Go through the value of each sensor for the current data point
        while (file.peek() != 10) { //While it's not the end of the line (end of current data point)
            if (file.peek() == 44) { //If it's a comma
                dataStr[currDataStr] = 0; //Add a 0 to the end of the current data string
                currDataStr = 0; //Reset dataStr index
                
                //Convert dataStr into current sensor value and copy it into the answer array
                currData = atof(dataStr);
                memcpy(&ans[*ansCurr], &currData, sizeof(uint8_t) * 4);
                
                (*ansCurr) += 4;
                file.read(); //Skip comma
            } else { //If it's not a comma
                dataStr[currDataStr++] = file.read(); //Add the current character(number) to the current data string
            }
        }
        dataStr[currDataStr] = 0; //Add a 0 to the end of the current data string
        
        //Convert dataStr into current sensor value and copy it into the answer array
        currData = atof(dataStr);
        memcpy(&ans[*ansCurr], &currData, sizeof(uint8_t) * 4);
        
        (*ansCurr) += 4;
        file.read();
        currNum++;
    }
    
    file.close();
    
    ans[numPos] = currNum; //Put the number of data points into the answer array
    
    Serial.print(F("11.14203: "));
    Serial.println(freeMemory());
    
    return;
}

//Truncate ToSend.csv to account for sent data
void truncateToSend() {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    File file = sd.open("ToSend.csv", FILE_WRITE);
    
    file.truncate(31);
    
    file.close();
    
    return;
}


/*------------------------FONA Functions------------------------*/
//Post a given HTTP request through the FONA
bool fonaPost(char* reqArr) {
    //Initialise the FONA software serial
    SoftwareSerial fonaSS(FONA_TX, FONA_RX);
    fonaSS.begin(4800);
    
    delay(250); //Delay to ensure the software serial is initialised
    
    bool err = false; //Boolean to keep track of if an error occurs
    
    //The way the commands are sent are all the same, so I'll only comment one
    
    //First send an AT command to make sure the FONA is working
    for (uint8_t i = 0; !err && i < 20; i++) { //Try 20 times
        if (!sendCheckReply(fonaSS, F("AT"), "OK", 1000)) { //Send "AT" to the FONA and wait 1000 milliseconds for the "OK" response
            if (i == 19) { //If it fails on the 20th time, then set the error flag
                err = true;
            }
        } else { //If it works
            break;
        }
    }
    
    //Send HTTP start command
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!sendCheckReply(fonaSS, F("AT+CHTTPSSTART"), "OK", 10000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }
    
    //Send HTTP open session command
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!sendCheckReply(fonaSS, F("AT+CHTTPSOPSE=\"www.cas.mcmaster.ca\",80,1"), "OK", 5000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }
    
    //Build HTTP send command with size of given request
    char* httpssendArr = malloc(sizeof(char) * 20);
    sprintf(httpssendArr, "AT+CHTTPSSEND=%d", strlen(reqArr)+20);
    
    //Send HTTP send command
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!sendCheckReply(fonaSS, httpssendArr, ">", 5000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }
    
    free(httpssendArr); //Free allocated memory
    
    //Send HTTP request
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!sendCheckReply(fonaSS, reqArr, "OK", 15000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }
    
    sendCheckReply(fonaSS, F("AT+CHTTPSCLSE"), "OK", 1000); //Send close HTTP session command
    sendCheckReply(fonaSS, F("AT+CHTTPSSTOP"), "OK", 1000); //Send stop HTTP command
    fonaSS.end();
    
    return err; //Return if there was an error or not
}

//Send an AT command to the FONA and wait for a specific reply
//Command is as a __FlashStringHelper
bool sendCheckReply(Stream& port, __FlashStringHelper* command, char* reply, int timeout) {
    Serial.print(F("Sending to fona: "));
    Serial.println(command);
    
    port.println(command); //Send the command to the FONA
    
    unsigned long start = millis(); //The time before we wait in the while loop
    
    char response[20]; //Char array for the response from the FONA
    uint8_t curr = 0; //Index for the response char array
    
    //While timeout time hasn't passed yet
    while ((millis() >= start) ? ((millis() - start) < timeout) : ((millis() + (4294967295 - start)) < timeout)) { //Checks for overflow
        while (port.available() && curr < 19) { //If the FONA has a character available and the response char array isn't full
            delay(250);
            char c = port.read(); //Read the available character
            
            if (c == '+') { //If it's a '+' then the current response if finished
                break;
            }
            
            if (c != '\n' && c != '\r') { //If the current character isn't a new line or carriage return add it to the response char array
                response[curr++] = c;
            }
        }
        
        if (curr != 0) { //If the response is non-empty
            response[curr] = 0; //Add a terminating 0

            Serial.println(response);

            if (strcmp(response, reply) == 0) { //If it's the response we're waiting for
                return true;
            } else { //If it isn't
                //Reset the response char array and index variable
                response[0] = 0;
                curr = 0;
            }
        }
    }
    
    return false; //If it timed out and didn't receive the correct response from the FONA
}

//Send an AT command to the FONA and wait for a specific reply
//Command is as a char*
bool sendCheckReply(Stream& port, char* command, char* reply, int timeout) {
    //Refer to the function above for comments, as it's the exact same code
    
    Serial.print(F("Sending to fona: "));
    Serial.println(command);
    
    port.println(command);
    
    unsigned long start = millis();
    
    char response[20];
    uint8_t curr = 0;
    
    while ((millis() >= start) ? ((millis() - start) < timeout) : ((millis() + (4294967295 - start)) < timeout)) {
        while (port.available() && curr < 19) {
            delay(250);
            char c = port.read();
            if (c == '+') {
                break;
            }
            if (c != '\n' && c != '\r') {
                response[curr++] = c;
            }
        }
        
        if (curr != 0) {
            response[curr] = 0;

            Serial.println(response);

            if (strcmp(response, reply) == 0) {
                return true;
            } else {
                response[0] = 0;
                curr = 0;
                continue;
            }
        }
    }
    
    return false;
}

/*
//Function to test the FONA, not needed
void TestFona() {
    SoftwareSerial fonaSS(FONA_TX, FONA_RX);
    
    fonaSS.begin(4800);
    delay(250);
    
    while (true) {
        while (Serial.available()) {
            char c = Serial.read();
            fonaSS.write(c);
            Serial.write(c);
        }
        while (fonaSS.available()) {
            Serial.write(fonaSS.read());
        }
    }
}*/


/*----------------------LORA FUNCTIONS----------------------*/
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
  Serial.print(F(" 0x"));
  if (b <= 0xF)
	Serial.print(F("0"));
  Serial.print(b, HEX);
}

void printShort(uint16_t s) {
  Serial.print(F(" 0x"));
  if (s <= 0xFFF)
	Serial.print(F("0"));
  if (s <= 0xFF)
	Serial.print(F("0"));
  if (s <= 0xF)
	Serial.print(F("0"));
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

  printByte(frameType);
  printByte((uint8_t) 0); // frameNum
  printByte(cmdType);
  printByte(payloadLen);

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
  printByte(checksum);

  Serial.println();
}

uint8_t* readData(Stream& port) {
  uint8_t frameType = 0;
  uint8_t cmdType = 0;
  uint8_t* payload = NULL;
  int len = readFrame(port, &frameType, &cmdType, &payload);

  if (frameType != 0x05 || cmdType != 0x82) {
	Serial.println(F("BAD TYPE"));
	free(payload);
	return;
  }

  uint16_t srcAddr = (payload[0] << 8) | payload[1];
  //uint8_t power = payload[2];
  uint8_t userPayloadLength = payload[3];
  //Serial.print(" srcAddr="); printShort(srcAddr);
  //Serial.print(" power="); printByte(power);
  //Serial.print(" userPayloadLength="); printByte(userPayloadLength);

  uint8_t *ans = malloc(sizeof(uint8_t) * (userPayloadLength + 3));
  memcpy(ans, &srcAddr, sizeof(uint8_t) * 2);
  ans[2] = userPayloadLength;
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
	Serial.print(F(" targetAddr=")); printShort(targetAddr);
	Serial.print(F(" status=")); printByte(status);
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
	Serial.println(F("ERROR"));
	free(payload);
	return;
  }

  Serial.print(F("FrameType="));
  printByte(frameType);
  Serial.print(F(" CmdType="));
  printByte(cmdType);

  if (frameType != 0x01 || cmdType != 0x82) {
	Serial.println(F("BAD TYPE"));
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
  free(payload);

  Serial.print(F(" flag=")); printShort(flag);
  Serial.print(F(" channel=")); printByte(channel);
  Serial.print(F(" txPower=")); printByte(txPower);
  Serial.print(F(" uiMode=")); printByte(uiMode);
  Serial.print(F(" eqType=")); printByte(eqType);
  Serial.print(F(" netId=")); printShort(netId);
  Serial.print(F(" nodeId=")); printShort(nodeId);
  Serial.print(F(" serPortParam=")); printByte(serPortParam);
  Serial.print(F(" airRate=")); printShort(airRate);
  Serial.println();
  return nodeId;
}