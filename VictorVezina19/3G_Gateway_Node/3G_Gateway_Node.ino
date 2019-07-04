/*----------IMPORTANT INFORMATION AND CONFIG PARAMETERS---------*/

// You have to press the power button on the 3G shield when you power the Arduino

// Change the following parameters to accommodate your specific setup
#define NETWORK_ID 0x1   //LoRa network ID, has to be the same on every LoRa module in your network
#define NODE_ID 0x1000   //LoRa ID of this node, you then need to put this into the end nodes that are sending to this gateway
#define LORA_RX 6      //Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
#define LORA_TX 7      //Pin that the LoRa RXD pin is connected to


/*
Things to do:
- Hardcoded URL takes global memory, which is no bueno!!!!!!

- Add location to main HTTP request
- Change HTTP request to not include "Time"
- Change HTTP requests to not be hardcoded
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

#include <SPI.h>
#include <SoftwareSerial.h>
#include <SdFat.h>

//#include <MemoryFree.h>

/*----------------------DEFINITIONS-----------------------------*/

#define FONA_TX 4
#define FONA_RX 5

/*-------------------------GLOBALS------------------------------*/

unsigned long lastPost; //The millis() value that the last data post happened at

/*------------------------CONSTRUCTORS--------------------------*/

SoftwareSerial loraPort(LORA_RX, LORA_TX);

/*---------------------------SETUP------------------------------*/

void setup() {
    //Begin some serials
    loraPort.begin(9600);
    Serial.begin(9600);

    //Write the LoRa configuration from the defines to the LoRa module
    writeConfig(loraPort, NETWORK_ID, NODE_ID);
    
    delay(1000);
    
    while (!Serial.available()) ; //Useful for testing
    
    while (loraPort.available()) loraPort.read(); //Delete data in loraPort, seems to mess gateway up once in a while

    //Make sure ToSend.csv exists
    createToSend();
    
    //Pretend like the last post just happened
    lastPost = millis();
}

/*----------------------------LOOP------------------------------*/

void loop() {
    loraPort.listen(); //Listen on the LoRa module software serial
    Serial.println(F("Waiting for LoRa messages"));

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

                default: //Unknown message
                    Serial.println(F("Unknown Message Type Received"));
            }

            free(loraData); //Free allocated message memory
        }
    }
    Serial.println(F("Posting data"));
    
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
    char* name = malloc(sizeof(char) * (nameLen + 1));
    
    for (uint8_t i = 0; i < nameLen; i++) {
        name[i] = data[curr++];
    }
    name[nameLen] = 0;
    
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
        uint8_t* ackData = malloc(sizeof(uint8_t));
        ackData[0] = 0;
        sendData(loraPort, add, 1, ackData);
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
        Serial.println(F("Error initialising sd card"));
        return;
    }

    //Get id of node that sent the data
    uint16_t add;
    memcpy(&add, &data[0], sizeof(uint8_t) * 2);
    
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
                /*
                for (uint8_t i = 0; i < 2; i++) { //Back up until it's before the number of data points
                    toSendFile.seekCur(-1);
                    while (toSendFile.peek() != 44) { // ','
                        toSendFile.seekCur(-1);
                    }
                }
                
                toSendFile.read(); //Get rid of the ,*/
                
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
    //Should handle running out of memory better
    //Could loop, if encounter error free current and continue(means numNodes--)
    //If no error break, and truncate a number of nodes
    //Would have to go through ToSend.csv backwards
    
    while (true) {
        uint8_t numNodes = countNodesToSend();
        if (numNodes == 0) {
            return;
        }

        uint8_t* sendInfo = malloc(sizeof(uint8_t) * (8 * numNodes));
        if (sendInfo == NULL) {
            return;
        }
        getSendInfo(sendInfo, numNodes);

        unsigned int dataSize = getDataSize(sendInfo, numNodes);
        uint8_t* data = malloc(sizeof(uint8_t) * dataSize);
        if (data == NULL) {
            return;
        }
        getData(data, sendInfo, numNodes);

        char* request = malloc(sizeof(char) * (2 + 2 * dataSize + urlLen() + 1));
        if (request == NULL) {
            return;
        }
        buildRequest(request, data, numNodes, dataSize);

        bool error = fonaPost(request);

        free(request);
        free(data);
        free(sendInfo);

        //If it posted correctly, truncate ToSend.csv to the correct length (remove numNodes nodes from the end)
        if (!error) {
            truncateToSend(numNodes);
        } else {
            return;
        }
    }
}

int urlLen() {
    return 70;
}

//Counts the number of nodes that have data to send to the web server
uint8_t countNodesToSend() {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
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
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    //Open ToSend.csv and skip the first line
    File file = sd.open("ToSend.csv", FILE_READ);
    while (file.available() && file.read() != 10);
    
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
        uint8_t* currTypesInfo = malloc(sizeof(uint8_t) * 3); //Allocate the array to put the info into
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
        
        //Essentially doing: totalLen += #DataPoints * #Types * 4 bytes
        totalLen += sendInfo[3 + (8 * i)] * currTypesInfo[1] * 4;
        
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
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    //Open the given node's file
    char fileName[13];
    sprintf(fileName, "node%u.csv", id);
    File file = sd.open(fileName, FILE_READ);
    
    //Get length of name
    ans[2] = 0;
    while (file.read() != 10) {
        ans[2]++;
    }
    
    for (uint8_t i = 0; i < 3; i++) { //Skip time, latitude, and longitude
        while (file.read() != ',') ;
    }
    
    ans[0] = 2; //Total size of the sensor types
    ans[1] = 2; //How many sensor types there are
    
    while (file.peek() != 10) { //While it's not the end of the line
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
    //Currently this copies in Time as a type, but that's just a waste of space
    Serial.print(F("Getting data for node: "));
    Serial.println(id);
    
    //Copy the current node's id into the answer array
    memcpy(&ans[*ansCurr], &id, sizeof(uint8_t) * 2);
    (*ansCurr) += 2;
    
    ans[(*ansCurr)++] = numLocs;
    
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
    
    
    //Copy in some location info
    
    unsigned int locationPtr = 0;
    
    if (numLocs > 0) { //Means we need to put in name and adjust for future location positions
        (*ansCurr)++; //For name len, filled in later
        uint8_t nameLen = 0;
        
        while (file.peek() != 10) {
            nameLen++;
            ans[(*ansCurr)++] = file.read();
        }
        file.read(); //Skip '\n'
        
        ans[(*ansCurr) - (nameLen + 1)] = nameLen; //Put length of name in
        
        locationPtr = *ansCurr; //For future use
        
        (*ansCurr) += numLocs; //Leave room for location info
    } else {
        while (file.read() != 10) ; //Skip name
    }
    
    for (uint8_t i = 0; i < 3; i++) { //Skip time, latitude, and longitude
        while (file.read() != ',') ;
    }
    
    
    //Copy in sensor types
    
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
        file.read(); //Skip '\n'
    }
    
    file.close();
    
    ans[numPos] = currNum; //Put the number of data points into the answer array
    
    return;
}

//Build HTTP request to send through fona
void buildRequest(char* request, uint8_t* data, uint8_t numNodes, unsigned int dataSize) {
    Serial.print(F("Data size: ")); //For debugging
    Serial.println(dataSize);
    
    //Stuff here needs to be changed; the URL shouldn't be hardcoded
    
    strcpy(request, "POST /ollie/sensor/data?data="); //The first part of the HTTP request
    
    char curr[2]; //Char buffer
    sprintf(curr, "%02X", numNodes); //Print the number of nodes as hex into the char array, always taking up two characters
    request[29] = curr[0];
    request[29 + 1] = curr[1];
    
    for (unsigned int i = 0; i < dataSize; i++) { //Go through each byte of data, convert it into hex, and add that to the request
        sprintf(curr, "%02X", data[i]); //Print the byte of data as hex into the char array, always taking up two characters
        request[(2 * i) + 29 + 2] = curr[0];
        request[(2 * i) + 29 + 2 + 1] = curr[1];
    }
    
    strcpy(&request[29 + 2 + (2 * dataSize)], " HTTPS/1.1\r\nHost: www.cas.mcmaster.ca\r\n\r\n\0"); //The last part of the HTTP request
}

//Truncate ToSend.csv to account for sent data
void truncateToSend(uint8_t numNodes) {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    File file = sd.open("ToSend.csv", FILE_WRITE);
    
    file.truncate(42);
    
    file.close();
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
bool sendCheckReply(Stream& port, const __FlashStringHelper* command, char* reply, unsigned long timeout) {
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
bool sendCheckReply(Stream& port, char* command, char* reply, unsigned long timeout) {
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