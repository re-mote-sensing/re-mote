/*----------IMPORTANT INFORMATION AND CONFIG PARAMETERS---------*/

// You have to press the power button on the 3G shield when you power the Arduino

// Change the following parameters to accommodate your specific setup
#define NETWORK_ID 0x1   //LoRa network ID, has to be the same on every LoRa module in your network
#define NODE_ID 0x1000   //LoRa ID of this node, you then need to put this into the end nodes that are sending to this gateway
#define NAME "Gateway"   //Name to associate with this node
#define LORA_RX 6        //Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
#define LORA_TX 7        //Pin that the LoRa RXD pin is connected to

#define URL_HOST "www.cas.mcmaster.ca"
#define URL_PATH "/ollie"
#define PORT_NUMBER 80

#define NUMBER_SENSORS 2
char* sensorTypes[NUMBER_SENSORS] = {"Air_Temperature", "Humidity"};
uint8_t sensorPorts[NUMBER_SENSORS][2] = { {2, 0}, {2, 0} };


/*
Things to do:
- Add temperature/humidity sensor
- Actually implement other acks
- Don't just sit there waiting, sleep and check every second for lora
- Check for SD card being full
- Split functions into driver files
- Change code structure to use event loop
- Robustify
*/


/*--------------------------------------------------------------*/
/*-----------------------------CODE-----------------------------*/
/*--------------------------------------------------------------*/

/*---------------------------INCLUDES---------------------------*/

#include <remoteConfig.h>
#include <remoteLoRa.h>
#include <NeoSWSerial.h>
#include <SdFat.h>

#ifdef DEBUG
#include <MemoryFree.h>
#endif

/*----------------------DEFINITIONS-----------------------------*/

#define FONA_TX 4
#define FONA_RX 5
#define FONA_EN 8

/*------------------------CONSTRUCTORS--------------------------*/

remoteLoRa LoRa(LORA_RX, LORA_TX);

/*---------------------------SETUP------------------------------*/

void setup() {
    //Turn on the 3G chip on the fona
    pinMode(FONA_EN, OUTPUT);
    digitalWrite(FONA_EN, HIGH);
    delay(180);
    digitalWrite(FONA_EN, LOW);
    
    Serial.begin(9600);
    
    //initialiseSensors
    
    LoRa.writeConfig(NETWORK_ID, NODE_ID);
    
    //#ifdef DEBUG
    while (!Serial.available()) ; //Useful for testing
    //#endif
    
    //LoRa.readData(); //Backed-up data seems to break the gateway sometimes
    
    //Make sure ToSend.csv exists
    createToSend();
}

/*----------------------------LOOP------------------------------*/

void loop() {
    //readSensors();
    
    unsigned long lastPost = millis(); //Get post time before we actually post, this ensures that the posting starts exactly every hour
    postData(); //Post the saved data
    
    #ifdef DEBUG
    Serial.println(F("Waiting for LoRa messages"));
    Serial.println(freeMemory());
    #endif

    //Just keep checking the LoRa module until the right amount of time has passed
    while (((millis() >= lastPost) ? (millis() - lastPost) : (millis() + (4294967295 - lastPost))) < 60000) { //Makes sure to check for overflow
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
                    saveData(loraData);
                    break;

                default: //Unknown message
                    #ifdef DEBUG
                    Serial.println(F("Unknown Message Type Received"));
                    #else
                    ;
                    #endif
            }

            free(loraData); //Free allocated message memory
        }
    }
}

/*------------------------ALL FUNCTIONS-------------------------*/

/*------------ToSend.csv Initialisation Function------------*/
//Create "ToSend.csv" if it doesn't already exist
void createToSend() {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return;
    }
    
    //If ToSend.csv doesn't already exist, create it with the appropriate starting line
    if (!sd.exists("ToSend.csv")) {
        #ifdef DEBUG
        Serial.println(F("Creating ToSend.csv"));
        #endif
        
        File file = sd.open("ToSend.csv", FILE_WRITE);
        file.print(F("Node ID, Data Points, Locations, Position\n"));
        file.close();
    }
}


/*---------------Node Registration Function---------------*/
//Parse registration message and save new node information
void parseRegistration(uint8_t* data) {
    uint8_t numSensors = data[3] & 0x0F; //Get number of sensors in registration
    uint8_t curr = 4; //Used for going through the registration message data
    char *types[numSensors]; //Create array of char arrays for the sensor types
    
    uint8_t nameLen = data[curr++];
    char* name = (char*) malloc(sizeof(char) * (nameLen + 1));
    
    for (uint8_t i = 0; i < nameLen; i++) {
        name[i] = data[curr++];
    }
    name[nameLen] = 0;
    
    for (uint8_t i = 0; i < numSensors; i++) { //Get the sensor types from the message
        uint8_t len = data[curr++]; //Get length of type
        char *currType = (char*) malloc(sizeof(char) * (len + 1)); //Allocate current char array
        currType[len] = 0;
        for (uint8_t j = 0; j < len; j++) { //Copy type into char array
	        currType[j] = data[curr++];
        }
        types[i] = currType; //Point array of char arrays to current char array
    }

    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return;
    }

    //Get id of node that sent the registration
    uint16_t add;
    memcpy(&add, data, sizeof(uint8_t) * 2);
    
    //Get the file name for that node
    char* fileName = (char*) malloc(sizeof(char) * 13);
    sprintf(fileName, "node%u.csv", add);

    if (sd.exists(fileName)) { //Receiving registration from already registered node, check to see if good or not
        File file = sd.open(fileName, FILE_READ);
        
        bool err = false;
        
        //Check that name is correct
        for (uint8_t i = 0; i < nameLen; i++) {
            if (!(file.read() == name[i])) {
                err = true;
                break;
            }
        }
        
        file.seekCur(25); //Skip new line and default types (time, lat, lon)
        
        //Go through sensor types and make sure they match the registration message
        uint8_t currTypeNum = 0;
        for (uint8_t i = 0; err == false; i++) {
            uint8_t currByte = file.read();
            if (currByte == 44) {
                currTypeNum++;
                i = -1;
            } else if (currByte == 10) {
                break;
            } else if (currByte != types[currTypeNum][i]) {
                err = true;
                break;
            }
        }
        file.close();
        
        if (err) { //If the registration is bad
            //Should send error acknowledgement back
            #ifdef DEBUG
            Serial.print(F("Received bad registration data from node "));
            Serial.println(add);
            #endif
        } else { //If the registration is good
            //Send success acknoledgement back
            uint8_t* ackData = (uint8_t*) malloc(sizeof(uint8_t));
            ackData[0] = 0;
            LoRa.sendData(add, 1, ackData);
            free(ackData);
        }
    } else { //Registering new node
        File file = sd.open(fileName, FILE_WRITE);
        
        file.print(name); //Print the node name
        file.print(F("\n"));
        
        file.print(F("Time,Latitude,Longitude,")); //The first column is always time
        
        for (uint8_t i = 0; i < numSensors; i++) { //Print the sensor types into the file
            file.print(types[i]);
            if ((i+1) != numSensors) {
                file.print(F(","));
            }
        }
        file.print(F("\n"));
        file.close();
        
        //Send success acknowledgement back
        uint8_t* ackData = (uint8_t*) malloc(sizeof(uint8_t));
        ackData[0] = 0;
        LoRa.sendData(add, 1, ackData);
        free(ackData);
    }

    //Free allocated memory
    for (uint8_t i = 0; i < numSensors; i++) {
        free(types[i]);
    }
    free(name);
    free(fileName);
}


/*------------------Data Saving Functions------------------*/
//Save the received data into the sd card
void saveData(uint8_t* data) {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return;
    }

    //Get id of node that sent the data
    uint16_t add;
    memcpy(&add, &data[0], sizeof(uint8_t) * 2);
    
    //Get the file name for that node
    char* fileName = (char*) malloc(sizeof(char) * 13);
    sprintf(fileName, "node%u.csv", add);
    
    //Get some useful information from the message data
    uint8_t payloadLen = data[2];
    uint8_t numSensors = data[3] & 0x0F;
    
    if (!sd.exists(fileName)) { //If the node hasn't yet registered with this gateway
        //Should send error acknowledgement
        #ifdef DEBUG
        Serial.print(F("Received sensor data before registration from node "));
        Serial.println(add);
        #endif
    } else {
        File file = sd.open(fileName, FILE_WRITE);
        //Check the data to make sure it matches the node's registration
        if (!checkSensorData(file, numSensors)) { //If it doesn't match
            //Should send error acknowledgement
            #ifdef DEBUG
            Serial.print(F("Received bad sensor data from node "));
            Serial.println(add);
            #endif
        } else { //If it does match
            uint32_t position = file.curPosition(); //Get the position before the new data in the node's file
            
            //Add the new data points to the node's file
            
            uint8_t numData = 0; //Keeps track of how many data points and locations have been added
            uint8_t numLoc = 0; //Keeps track of how many locations have been added
            
            for (uint8_t i = 5 + data[4]; i < payloadLen + 3; ) { //While there's still data in the message
                numData++;
                
                //Get the time for the current data points
                unsigned long time;
                memcpy(&time, &data[i], sizeof(uint8_t) * 4);
                
                i += 4; //Make sure the index for the message data is being augmented
                
                //Print the time into the file
                file.print(time);
                file.print(F(","));
                
                if (numData == data[5 + numLoc] && numLoc < data[4]) { //Check if this data point has location
                    numLoc++;
                    
                    for (uint8_t j = 0; j < 2; j++) { //Print lat and lon to file
                        float currFloat;
                        memcpy(&currFloat, &data[i], sizeof(uint8_t) * 4);

                        i += 4;

                        file.printField(currFloat, ',', 6);
                        //file.print(F(","));
                    }
                } else {
                    file.print(F(",,")); //Print empty location if there's no location here
                }
                
                for (uint8_t j = 0; j < numSensors; j++) { //Iterate through the value for each sensor
                    //Get the value of the current sensor
                    float currFloat;
                    memcpy(&currFloat, &data[i], sizeof(uint8_t) * 4);
                    
                    i += 4;
                    
                    //Print the current sensor value into the file
                    //file.print(currFloat);
                    file.printField(currFloat, 0, 6);
                    
                    if (j+1 != numSensors) { //Print a , if it's not the last sensor
                        file.print(F(","));
                        //file.printField(currFloat, ',', 6);
                    } else {
                        //file.printField(currFloat, 0, 6);
                    }
                }
                file.print("\n"); //Finish the line
            }

            //See if the node already has data to be sent in ToSend.csv
            File toSendFile = sd.open("ToSend.csv", FILE_WRITE);
            toSendFile.seekSet(0);
            
            bool notFound = false;
            uint16_t currId;
            
            do { //While the node id on the current line isn't the node id we're looking for
                while (toSendFile.read() != 10); //Go to end of line (skips first line)
                
                if (!toSendFile.available()) { //If we reach the end of ToSend.csv without finding the id of the node
                    notFound = true;
                    break;
                }
                
                //Get the id of the node on the current line
                char currStr[12];
                int len = toSendFile.fgets(currStr, 12, ",");
                currStr[len-1] = 0;
                currId = atoi(currStr);
            } while (currId != add);

            if (notFound) { //If the node isn't already in ToSend.csv
                //Print the node's id into the ToSend file
                toSendFile.print(add);
                toSendFile.print(F(","));
                
                //Print the number of data points into theToSend file
                char numDataStr[4];
                sprintf(numDataStr, "%.3hu", numData);
                toSendFile.print(numDataStr);
                toSendFile.print(F(","));
                
                //Print the number of locations into the file
                sprintf(numDataStr, "%.3hu", numLoc);
                toSendFile.print(numDataStr);
                toSendFile.print(F(","));
                
                //Print the cursor position of the node's file into the ToSend file
                toSendFile.print(position);
                toSendFile.print("\n");
            } else { //If the node is already in ToSend.csv
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
                toSendFile.read(); //Get rid of ','
                
                //Write the new number of data points
                char numPointsStr[4];
                sprintf(numPointsStr, "%.3hu", numPoints + numData);
                toSendFile.print(numPointsStr);
                
                toSendFile.read(); //Get rid of ','
                
                //Read the current amount of locations ready to be sent for the current node
                len = toSendFile.fgets(currStr, 12, ",");
                currStr[len-1] = 0; //Overwrite ','
                numPoints = atoi(currStr);
                
                //Go back until it's before the number of locations
                toSendFile.seekCur(-2);
                while (toSendFile.peek() != 44) { // ','
                    toSendFile.seekCur(-1);
                }
                toSendFile.read();
                
                //Write the new number of locations
                sprintf(numPointsStr, "%.3hu", numPoints + numLoc);
                toSendFile.print(numPointsStr);
            }

            toSendFile.close(); //Close ToSend.csv
            
            //Send a success acknowledgement
            uint8_t* ackData = (uint8_t*) malloc(sizeof(uint8_t));
            ackData[0] = 0;
            LoRa.sendData(add, 1, ackData);
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
    
    while (file.read() != 10) ; //Skip first line (name)
    
    //Count the number of sensors in the given file
    uint8_t sum = 1;
    while (file.peek() != 10) { //While it's not a newline character
        if (file.read() == 44) { //If it's a comma
            sum++;
        }
    }
    
    if (sum - 3 != numSensors) { //If the number of sensors in the file isn't the same as the given number
        return false;
    } else { //If it is
        file.seek(position);
        return true;
    }
}


/*--------------Saved Data Posting Functions--------------*/
//Post collected data to webserver
void postData() {
    #ifdef DEBUG
    Serial.println(F("Posting data"));
    #endif
    
    //Loop until there's no data to send
    while (true) {
        uint8_t numNodes = countNodesToSend(); //Number of nodes with data to send
        
        //Loop over numNodes, essentially if it runs out of memory, try again with less nodes
        for (; numNodes > 0; numNodes--) {
            //Get info on what needs to be sent
            uint8_t* sendInfo = (uint8_t*) malloc(sizeof(uint8_t) * (8 * numNodes));
            if (sendInfo == NULL) {
                continue;
            }
            getSendInfo(sendInfo, numNodes);

            //Get data to be sent
            unsigned int dataSize = getDataSize(sendInfo, numNodes);
            uint8_t* data = (uint8_t*) malloc(sizeof(uint8_t) * dataSize);
            if (data == NULL) {
                free(sendInfo);
                continue;
            }
            getData(data, sendInfo, numNodes);

            //Build HTTPS request
            char* request = (char*) malloc(sizeof(char) * (strlen(URL_PATH) + 2 + 2 * dataSize + strlen(URL_HOST) + 46));
            if (request == NULL) {
                free(sendInfo);
                free(data);
                continue;
            }
            buildRequest(request, data, numNodes, dataSize);

            //Post the data, see if there's an error
            bool error = fonaPost(request);

            //Free allocated memory
            free(request);
            free(data);
            free(sendInfo);

            //If it posted correctly, truncate ToSend.csv to the correct length (remove numNodes nodes from the end)
            if (!error) {
                truncateToSend(numNodes);
                break;
            } else {
                continue;
            }
        }
        
        //No data to send or can't send any
        if (numNodes == 0) {
            return;
        }
    }
}

//Counts the number of nodes that have data to send to the web server
uint8_t countNodesToSend() {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return 0;
    }
    
    //Open ToSend.csv and skip the first line
    File file = sd.open("ToSend.csv", FILE_READ);
    while (file.available() && file.read() != 10);
    
    //Get the number of nodes that have data to send
    uint8_t numNodes = 0;
    while (file.available()) { //Go through the entire file
        while (file.read() != 10); //Go to end of line
        numNodes++;
    }
    
    return numNodes;
}

//Get the information on what data needs to be sent
//The returned array has the form(the numbers in the brackets is number of bytes): id1(2) #locations1(1) #points1(1) FilePostition1(4) id2(2) ...
void getSendInfo(uint8_t* ans, uint8_t numNodes) {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return;
    }
    
    //Open ToSend.csv and skip the first line
    File file = sd.open("ToSend.csv", FILE_READ);
    file.seekEnd();
    
    for (uint8_t i = 0; i < numNodes; i++) { //Go through each node, and get it's corresponding information
        //Go to in front of the next node
        file.seekCur(-2);
        while (file.available() && file.peek() != 10) {
            file.seekCur(-1);
        }
        file.seekCur(1);
        
        char currStr[12]; //Buffer
        
        //Get the current node's id
        int len = file.fgets(currStr, 12, ",");
        currStr[len-1] = 0; //Overwrite ','
        uint16_t currId = atoi(currStr);
        
        //Get the number of data points that the current node has to send
        len = file.fgets(currStr, 12, ",");
        currStr[len-1] = 0; //Overwrite ','
        uint8_t numPoints = atoi(currStr);
        
        //Get the number of data points that the current node has to send
        len = file.fgets(currStr, 12, ",");
        currStr[len-1] = 0; //Overwrite ','
        uint8_t numLocs = atoi(currStr);
        
        //Get the postition in the current node's file
        len = file.fgets(currStr, 12, "\n");
        currStr[len-1] = 0; //Overwrite '\n'
        uint32_t currPos = atol(currStr);
        
        //Put the gathered info into the array to return
        memcpy(&ans[8 * i], &currId, sizeof(uint8_t) * 2);
        ans[2 + (8 * i)] = numLocs;
        ans[3 + (8 * i)] = numPoints;
        memcpy(&ans[4 + (8 * i)], &currPos, sizeof(uint8_t) * 4);
        
        //Go to in front of the current node
        while (file.available() && file.peek() != 10) {
            file.seekCur(-1);
        }
        file.seekCur(1);
    }
    
    file.close();
}

//Calculates the array size needed for the data to be sent to the server
unsigned int getDataSize(uint8_t* sendInfo, uint8_t numNodes) {
    //Calculate needed size
    unsigned int totalLen = 0;
    
    for (uint8_t i = 0; i < numNodes; i++) { //Go through each node that has data to send
        //Get the node's id
        int currId;
        memcpy(&currId, &sendInfo[8 * i], sizeof(uint8_t) * 2);
        
        totalLen += 2; //For the node's id
        
        //Gets the type info for the current node
        uint8_t* currTypesInfo = (uint8_t*) malloc(sizeof(uint8_t) * 3); //Allocate the array to put the info into
        getTypesInfo(currTypesInfo, currId); //First byte is total size that will be taken in final data array, second byte is just number of sensors, third byte is length of name
        
        
        //Account for size of locations
        totalLen += 1; //For # locations
        
        if (sendInfo[2 + (8 * i)] > 0) {
            totalLen += 1 + currTypesInfo[2]; //For name length + name
            totalLen += 9 * sendInfo[2 + (8 * i)]; //For locations
        }
        
        totalLen += currTypesInfo[0]; //Size of sensor types information
        
        //Account for size of data points
        totalLen += 1; //For # data point
        
        //Essentially doing: totalLen += #DataPoints * (#Types + 1(for Time)) * 4 bytes
        totalLen += sendInfo[3 + (8 * i)] * (currTypesInfo[1] + 1) * 4;
        
        free(currTypesInfo); //Free allocated memory
    }
    
    return totalLen;
}

//Get some information of the sensor types of a specific node
//First byte is total size that will be taken in final data array, second byte is just number of sensors, third byte is length of name
void getTypesInfo(uint8_t* ans, int id) {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return;
    }
    
    //Open the given node's file
    char fileName[13];
    sprintf(fileName, "node%u.csv", id);
    File file = sd.open(fileName, FILE_READ);
    
    //Get length of name
    ans[2] = 0;
    while (file.available() && file.read() != 10) {
        ans[2]++;
    }
    
    for (uint8_t i = 0; i < 3; i++) { //Skip time, latitude, and longitude
        while (file.available() && file.read() != ',') ;
    }
    
    ans[0] = 2; //Total size of the sensor types (add 2; 1 for number of types and 1 for length of last type)
    ans[1] = 1; //How many sensor types there are (doesn't count the last one, so add 1)
    
    while (file.available() && file.peek() != 10) { //While it's not the end of the line
        if (file.read() == 44) { //If it's a comma
            ans[1]++; //New sensor type
            ans[0]++; //For length of new sensor type
        } else {
            ans[0]++; //For char of sensor type
        }
    }
    
    file.close();
}

//Get the data to send to the server
void getData(uint8_t* ans, uint8_t* sendInfo, uint8_t numNodes) {
    //Go through each node and get the data for the data points it needs to send
    unsigned int curr = 0;
    for (uint8_t i = 0; i < numNodes; i++) {
        //Get current node id
        int currId;
        memcpy(&currId, &sendInfo[8 * i], sizeof(uint8_t) * 2);
        
        //Get current number of locations
        uint8_t currLocs = sendInfo[2 + (8 * i)];
        
        //Get postition in the node's file
        unsigned long currPos;
        memcpy(&currPos, &sendInfo[4 + (8 * i)], sizeof(uint8_t) * 4);
        
        //Gets the data for the current node and puts it into the ans array starting at curr, increments curr accordingly
        getNodeData(ans, &curr, currId, currLocs, currPos);
    }
}

//Gets the data to be sent from a specific node
//Form of data: Id(2) NumTypes(1) Type1Len(1) Type1Char1(1) Type1Char2(1) ... Type1CharN(1) Type2Len(1) Type2Char1(1) ... Type2CharN(1) NumData(1) Data1Type1(4) Data1Type2(4) ... Data1TypeN(4) Data2Type1(4) ... DataNTypeN(4)
void getNodeData(uint8_t* ans, unsigned int* ansCurr, int id, uint8_t numLocs, unsigned long pos) {
    #ifdef DEBUG
    Serial.print(F("Getting data for node: "));
    Serial.println(id);
    #endif
    
    //Copy the current node's id into the answer array
    memcpy(&ans[*ansCurr], &id, sizeof(uint8_t) * 2);
    (*ansCurr) += 2;
    
    ans[(*ansCurr)++] = numLocs;
    
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return;
    }
    
    //Open the current node's file
    char fileName[13];
    sprintf(fileName, "node%u.csv", id);
    File file = sd.open(fileName, FILE_READ);
    file.seekSet(0);
    
    
    //Copy in some location info
    
    unsigned int locationPtr = 0;
    
    if (numLocs > 0) { //Means we need to put in name and adjust for future location positions
        (*ansCurr)++; //For name len, filled in later
        uint8_t nameLen = 0;
        
        while (file.available() && file.peek() != 10) {
            nameLen++;
            ans[(*ansCurr)++] = file.read();
        }
        file.read(); //Skip '\n'
        
        ans[(*ansCurr) - (nameLen + 1)] = nameLen; //Put length of name in
        
        locationPtr = *ansCurr; //For future use
        
        (*ansCurr) += numLocs; //Leave room for location info
    } else {
        while (file.available() && file.read() != 10) ; //Skip name
    }
    
    for (uint8_t i = 0; i < 3; i++) { //Skip time, latitude, and longitude
        while (file.available() && file.read() != ',') ;
    }
    
    
    //Copy in sensor types
    
    //Variables needed to copy in the sensor type names
    unsigned int numPos = *ansCurr; //Posistion where the total number of types needs to go
    uint8_t currTypeLen = 0; //Length of the current sensor type
    uint8_t currNum = 1; //Number of sensor types (won't count the last one, so add one)
    (*ansCurr) += 2;
    
    while (file.available() && file.peek() != 10) { //While it's not the end of the line
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
    
    file.seekSet(pos); //Go to the position where the data points start
    numPos = *ansCurr; //Postition for the number of data points
    currNum = 0; //Number of data points
    (*ansCurr)++;
    
    
    //Go through each data point and copy in time, location, and sensor readings
    
    while (file.available()) { //While it's not the end of the file
        currNum++;
        char dataStr[20];
        
        //Get the time for the curr data point
        uint8_t len = file.fgets(dataStr, 13, ",");
        dataStr[len-1] = 0; //Overwrite ','
        unsigned long currTime = strtoul(dataStr, NULL, 10);
        
        //Put the current time into the answer array
        memcpy(&ans[*ansCurr], &currTime, sizeof(uint8_t) * 4);
        (*ansCurr) += 4;
        
        
        //Location check and value copy
        
        if (file.peek() != 44) { //Means this data point has a location
            ans[locationPtr++] = currNum; //Indicate that this data point has a location
            
            //Read both latitude and longitude and copy it in
            for (uint8_t i = 0; i < 2; i++) {
                len = file.fgets(dataStr, 13, ",");
                dataStr[len-1] = 0; //Overwrite ','
                float currLoc = atof(dataStr);
                
                memcpy(&ans[*ansCurr], &currLoc, sizeof(uint8_t) * 4);
                (*ansCurr) += 4;
            }
        } else { //Skip the two commas
            file.read();
            file.read();
        }
            
        
        //Copy in sensor values
        
        float currData; //Value of current sensor value
        uint8_t currDataStr = 0; //Index for dataStr
        
        //Go through the value of each sensor for the current data point
        while (file.available() && file.peek() != 10) { //While it's not the end of the line (end of current data point)
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
        file.read(); //Skip '\n'
    }
    
    file.close();
    
    ans[numPos] = currNum; //Put the number of data points into the answer array
    
    return;
}

//Build HTTP request to send through fona
void buildRequest(char* request, uint8_t* data, uint8_t numNodes, unsigned int dataSize) {
    #ifdef DEBUG
    Serial.print(F("Data size: "));
    Serial.println(dataSize);
    #endif
    
    sprintf(request, "POST %s/sensor/data?data=%02X", URL_PATH, numNodes);//The first part of the HTTP request
    
    for (unsigned int i = 0; i < dataSize; i++) { //Go through each byte of data, convert it into hex, and add that to the request
        sprintf(&request[strlen(URL_PATH) + 25 + (2 * i)], "%02X", data[i]); //Print the byte of data as hex into the char array, always taking up two characters
    }
    
    sprintf(&request[strlen(URL_PATH) + 25 + (2 * dataSize)], " HTTPS/1.1\r\nHost: %s\r\n\r\n", URL_HOST);//The last part of the HTTP request
}

//Truncate ToSend.csv to account for sent data
void truncateToSend(uint8_t numNodes) {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return;
    }
    
    File file = sd.open("ToSend.csv", FILE_WRITE);
    
    //Go through numNodes nodes
    for (uint8_t i = 0; i < numNodes; i++) {
        file.seekCur(-2);
        while (file.available() && file.peek() != 10) {
            file.seekCur(-1);
        }
        file.seekCur(1);
    }
    
    //Truncate the file to delete the data that was sentS
    file.truncate(file.curPosition());
    
    file.close();
}


/*------------------------FONA Functions------------------------*/
//Post a given HTTP request through the FONA
bool fonaPost(char* reqArr) {
    //Initialise the FONA software serial
    NeoSWSerial fonaSS(FONA_TX, FONA_RX);
    fonaSS.begin(9600);
    
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
    
    sendCheckReply(fonaSS, F("AT+CHTTPSCLSE"), "OK", 2000); //Send close HTTP session command
    sendCheckReply(fonaSS, F("AT+CHTTPSSTOP"), "OK", 2000); //Send stop HTTP command
    
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
    
    //Build HTTP open session command with the defined url and port
    char* httpsArr = (char*) malloc(sizeof(char) * (20 + strlen(URL_HOST) + 5));
    if (httpsArr == NULL) {
        err = true;
    }
    sprintf(httpsArr, "AT+CHTTPSOPSE=\"%s\",%d,1", URL_HOST, PORT_NUMBER);
    
    //Send HTTP open session command
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!sendCheckReply(fonaSS, httpsArr, "OK", 5000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }
    
    free(httpsArr);
    
    //Build HTTP send command with size of given request
    httpsArr = (char*) malloc(sizeof(char) * 20);
    if (httpsArr == NULL) {
        err = true;
    }
    sprintf(httpsArr, "AT+CHTTPSSEND=%d", strlen(reqArr)+20);
    
    //Send HTTP send command
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!sendCheckReply(fonaSS, httpsArr, ">", 5000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }
    
    free(httpsArr);
    
    //Send HTTP request
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!sendCheckReply(fonaSS, reqArr, "OK", 5000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }
    
    sendCheckReply(fonaSS, F("AT+CHTTPSCLSE"), "OK", 2000); //Send close HTTP session command
    sendCheckReply(fonaSS, F("AT+CHTTPSSTOP"), "OK", 2000); //Send stop HTTP command
    fonaSS.end();
    
    return err; //Return if there was an error or not
}

//Send an AT command to the FONA and wait for a specific reply
//Command is as a __FlashStringHelper
bool sendCheckReply(Stream& port, const __FlashStringHelper* command, char* reply, unsigned long timeout) {
    #ifdef DEBUG
    Serial.print(F("Sending to fona: "));
    Serial.println(command);
    #endif
    
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

            #ifdef DEBUG
            Serial.println(response);
            #endif

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
bool sendCheckReply(Stream& port, char* command, char* reply, unsigned long timeout) {
    //Refer to the function above for comments, as it's the exact same code
    
    #ifdef DEBUG
    Serial.print(F("Sending to fona: "));
    Serial.println(command);
    #endif
    
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

            #ifdef DEBUG
            Serial.println(response);
            #endif

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

#ifdef DEBUG
void printByte(uint8_t b) {
  Serial.print(F(" 0x"));
  if (b <= 0xF)
	Serial.print(F("0"));
  Serial.print(b, HEX);
}
#endif