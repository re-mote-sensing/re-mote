/*
Library for saving the data of a gateway, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated August 8, 2019
Released into the public domain
*/

#include "remoteGatewayData.h"
#include <remoteConfig.h>

#if Data_Type == SD_Type
#include <SdFat.h>
#endif

#ifdef DEBUG
#include <MemoryFree.h>
#endif

//Start the data saving method
remoteGatewayData::remoteGatewayData() {
}

//Initialise the data storage
void remoteGatewayData::initialise() {
    #if Data_Type == SD_Type
    initialiseSD();
    #endif
}

//Reset the data storage
void remoteGatewayData::reset(bool hard) {
    #if Data_Type == SD_Type
    resetSD(hard);
    #endif
}

//Save a registration to the storage
uint8_t remoteGatewayData::saveRegistration(uint8_t* data) {
    #if Data_Type == SD_Type
    return saveRegSD(data);
    #endif
    
    return 0x03;
}

//Save some data to the storage
uint8_t remoteGatewayData::saveData(uint8_t* data) {
    #if Data_Type == SD_Type
    return saveDataSD(data);
    #endif
    
    return 0x03;
}

//Get a LoRa data message
char* remoteGatewayData::getPost(unsigned int arg, void** toFree) {
    #if Data_Type == SD_Type
    return getPostSD(arg, toFree);
    #endif
    
    return NULL;
}

//Set the data according to a successful message sent
void remoteGatewayData::messageSuccess() {
    #if Data_Type == SD_Type
    messageSuccessSD();
    #endif
}


/*--------------------------PRIVATE--------------------------*/

#if Data_Type == SD_Type
/*--------------------------SD Card--------------------------*/
//COMBINE SIZE WITH SENDINFO?
//Replace seekEnd with custom function???

//Initialise the SD Card
void remoteGatewayData::initialiseSD() {
    //Create "ToSend.csv" if it doesn't already exist
    
    //Initialise the SD card
    SdFat sd;
    if (!sd.begin(SD_CS)) {
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
        
        SdFile file;
        if (!file.open("ToSend.csv", FILE_WRITE)) {
            #ifdef DEBUG
            Serial.println(F("Error opening ToSend.csv"));
            #endif
            return;
        }
        
        //Print the column headers
        file.print("NodeID,Data Points,Position\n");
        file.close();
    }
}

//Reset the SD card
void remoteGatewayData::resetSD(bool hard) {
    #ifdef DEBUG
    if (hard) Serial.println(F("Hard resetting SD card"));
    else Serial.println(F("Soft resetting SD card"));
    #endif
    
    //Initialise the SD card
    SdFat sd;
    if (!sd.begin(SD_CS)) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return;
    }
    
    if (hard) {
        //Remove everything on the sd card
        (*sd.vwd()).rmRfStar();
    } else {
        //Delete the data in ToSend.csv
        if (sd.exists("ToSend.csv")) {
            SdFile file;
            if (!file.open("ToSend.csv", FILE_WRITE)) {
                #ifdef DEBUG
                Serial.println(F("Error opening ToSend.csv"));
                #endif
                return;
            }
            
            file.seekSet(0);
            while (file.read() != 10) ;
            file.truncate(file.curPosition());
            file.close();
        }
    }
}

//Register a node
uint8_t remoteGatewayData::saveRegSD(uint8_t* data) {
    uint8_t ans = 0x03; //Number to return
    
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin(SD_CS)) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return ans;
    }
    SdFile file;
    
    uint8_t curr = 4; //Used for going through the registration message data
    
    uint8_t nameLen = data[curr++];
    char* name = (char*) malloc(sizeof(char) * (nameLen + 1));
    if (name == NULL) {
        #ifdef DEBUG
        Serial.println(F("Ran out of memory making node name"));
        #endif
        return ans;
    }
    
    name[nameLen] = 0;
    memcpy(name, &data[curr], sizeof(char) * nameLen);
    curr += nameLen;
    
    uint8_t numSensors = data[3] & 0x0F; //Get number of sensors in registration
    char* types[numSensors]; //Create array of char arrays for the sensor types
    
    for (uint8_t i = 0; i < numSensors; i++) { //Get the sensor types from the message
        uint8_t len = data[curr++]; //Get length of type
        types[i] = (char*) malloc(sizeof(char) * (len + 1)); //Allocate current char array
        if (types[i] == NULL) {
            #ifdef DEBUG
            Serial.println(F("Ran out of memory making type name"));
            #endif
            for (uint8_t i = 0; i < numSensors; i++) {
                free(types[i]);
            }
            free(name);
            return ans;
        }
        
        types[i][len] = 0;
        memcpy(types[i], &data[curr], len);
        curr += len;
    }
    
    //Get id of node that sent the registration
    uint16_t add;
    memcpy(&add, data, sizeof(uint8_t) * 2);
    
    //Get the file name for that node
    char* fileName = (char*) malloc(sizeof(char) * 13);
    if (fileName == NULL) {
        #ifdef DEBUG
        Serial.println(F("Ran out of memory making file name"));
        #endif
        for (uint8_t i = 0; i < numSensors; i++) {
            free(types[i]);
        }
        free(name);
        return ans;
    }
    
    if (sprintf(fileName, "node%u.csv", add) < 0) {
        #ifdef DEBUG
        Serial.println(F("Error printing to node file name"));
        #endif
        free(fileName);
        for (uint8_t i = 0; i < numSensors; i++) {
            free(types[i]);
        }
        free(name);
        return ans;
    }

    if (sd.exists(fileName)) { //Receiving registration from already registered node, check to see if good or not
        if (!file.open(fileName, FILE_READ)) {
            #ifdef DEBUG
            Serial.println(F("Error opening node file"));
            #endif
            free(fileName);
            for (uint8_t i = 0; i < numSensors; i++) {
                free(types[i]);
            }
            free(name);
            return ans;
        }
        
        bool err = false;
        
        //Check that name is correct
        for (uint8_t i = 0; !err && i < nameLen; i++) {
            err = file.read() != name[i];
        }
        
        err = err || file.read() != '\n'; //Skip new line
        skipConstTypes(&file); //Skip Time,Latitude,Longitude
        
        //Go through sensor types and make sure they match the registration message
        uint8_t currTypeNum = 0;
        for (uint8_t i = 0; !err; i++) {
            uint8_t currByte = file.read();
            if (currByte == 44) {
                currTypeNum++;
                i = -1;
            } else if (currByte == 10) {
                break;
            } else {
                err = currByte != types[currTypeNum-1][i];
            }
        }
        file.close();
        
        err = err || currTypeNum != numSensors;
        
        if (err) { //If the registration is bad
            #ifdef DEBUG
            Serial.print(F("Received bad registration data from node "));
            Serial.println(add);
            #endif
            ans = 0x02;
        } else { //If the registration is good
            ans = 0x01;
        }
    } else { //Registering new node
        if (!file.open(fileName, FILE_WRITE)) {
            #ifdef DEBUG
            Serial.println(F("Error opening node file"));
            #endif
            free(fileName);
            for (uint8_t i = 0; i < numSensors; i++) {
                free(types[i]);
            }
            free(name);
            return ans;
        }
        
        //Print the name and the set columns
        file.print(name);
        file.print(F("\nTime,Latitude,Longitude"));
        
        
        for (uint8_t i = 0; i < numSensors; i++) { //Print the sensor types into the file
            file.print(",");
            file.print(types[i]);
        }
        file.print('\n');
        file.close();
        
        ans = 0x00;
    }

    //Free allocated memory
    free(fileName);
    for (uint8_t i = 0; i < numSensors; i++) {
        free(types[i]);
    }
    free(name);
    
    return ans;
}

//Save node sensor data
uint8_t remoteGatewayData::saveDataSD(uint8_t* data) {
    uint8_t ans = 0x03;
    
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin(SD_CS)) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return ans;
    }

    //Get id of node that sent the data
    uint16_t add;
    memcpy(&add, data, sizeof(uint16_t));
    
    //Get the file name for that node
    char* fileName = (char*) malloc(sizeof(char) * 13);
    if (fileName == NULL) {
        #ifdef DEBUG
        Serial.println(F("Ran out of memory making node name"));
        #endif
        return ans;
    }
    
    if (sprintf(fileName, "node%u.csv", add) < 0) {
        #ifdef DEBUG
        Serial.println(F("Error printing to node file name"));
        #endif
        free(fileName);
        return ans;
    }
    
    //Get some useful information from the message data
    uint8_t payloadLen = data[2] + 3;
    uint8_t numSensors = data[3] & 0x0F;
    
    if (!sd.exists(fileName)) { //If the node hasn't yet registered with this gateway
        //Should send error acknowledgement
        #ifdef DEBUG
        Serial.print(F("Received sensor data before registration from node "));
        Serial.println(add);
        #endif
        ans = 0x01;
    } else {
        SdFile file;
        if (!file.open(fileName, FILE_WRITE)) {
            #ifdef DEBUG
            Serial.println(F("Error opening node file"));
            #endif
            free(fileName);
            return ans;
        }

        //Check the data to make sure it matches the node's registration
        
        file.seekSet(0); //Go to the begining of the file

        while (file.read() != 10) ; //Skip first line (name)

        //Count the number of sensors in the given file
        uint8_t sum = 1;
        while (file.available()) {
            char c = file.read();
            if (c == 10) {
                break;
            } else if (c == 44) {
                sum++;
            }
        }
        
        if (sum - 3 != numSensors) { //If it doesn't match
            //Should send error acknowledgement
            #ifdef DEBUG
            Serial.print(F("Received bad sensor data from node "));
            Serial.println(add);
            #endif
            ans = 0x02;
        } else { //If it does match
            file.seekEnd();
            uint32_t position = file.curPosition(); //Get the position before the new data in the node's file
            
            //Add the new data points to the node's file
            
            uint8_t numData = 0; //Keeps track of how many data points have been added
            uint8_t numLoc = 0; //Keeps track of how many locations have been added
            
            for (uint8_t i = 5 + data[4]; i < payloadLen; ) { //While there's still data in the message
                numData++;
                
                //Get the time for the current data points
                unsigned long time;
                memcpy(&time, &data[i], sizeof(uint8_t) * 4);
                
                i += 4; //Make sure the index for the message data is being increased
                
                //Print the time into the file
                file.print(time);
                file.print(F(","));
                
                #ifdef DEBUG
                Serial.print(F("Time: "));
                Serial.println(time);
                #endif
                
                if (numLoc < data[4] && numData == data[5 + numLoc]) { //Check if this data point has a location
                    numLoc++;
                    
                    i += filePrintFloat(&file, &data[i], 2); //Print the locations into the file
                } else {
                    file.print(F(",,")); //Print empty location if there's no location here
                }
                
                i += filePrintFloat(&file, &data[i], numSensors); //Print the sensor data into the file
                
                //Replace the last ',' with a '\n'
                file.seekCur(-1);
                file.print('\n');
            }
            
            file.close();
            
            //See if the node already has data to be sent in ToSend.csv
            if (!file.open("ToSend.csv", FILE_WRITE)) {
                #ifdef DEBUG
                Serial.println(F("Error opening ToSend.csv"));
                #endif
                free(fileName);
                return ans;
            }
            
            file.seekSet(0);
            
            bool notFound = false;
            uint16_t currId;
            
            do { //While the node id on the current line isn't the node id we're looking for
                while (file.read() != 10); //Go to end of line (skips first line)
                
                if (!file.available()) { //If we reach the end of ToSend.csv without finding the id of the node
                    notFound = true;
                    break;
                }
                
                //Get the id of the node on the current line
                currId = (uint16_t) fileReadInt(&file);
                
            } while (currId != add);
            
            if (notFound) { //If the node isn't already in ToSend.csv
                //Print the node's info into the file
                char str[23];
                sprintf(str, "%u,%.3hu,%lu\n", add, numData, position);
                file.print(str);
            } else { //If the node is already in ToSend.csv
                //Change the number of data points and locations
                
                //Read the current amount of data points ready to be sent for the current node
                uint8_t numPointsData = (uint8_t) fileReadInt(&file, ',', -3);
                
                char str[4];
                sprintf(str, "%.3hu", numPointsData + numData);
                file.print(str);
            }
            
            ans = 0x00;
        }
        file.close(); //Close the open file
    }
    free(fileName); //Free allocated memory
    
    return ans;
}

//Print num floats (6 decimals of precision) to a file from arr
uint8_t remoteGatewayData::filePrintFloat(SdFile* file, uint8_t* arr, uint8_t num) {
    uint8_t max = num * 4;
    uint8_t i = 0;
    for (; i < max; i += 4) {
        float curr;
        memcpy(&curr, &arr[i], sizeof(float));
        (*file).printField(curr, ',', 6);
        
        #ifdef DEBUG
        Serial.print(F("Float: "));
        Serial.println(curr);
        #endif
    }
    return i;
}

//Reads an unsigned int from a file and returns the uint32_t representation of it
uint32_t remoteGatewayData::fileReadInt(SdFile* file, char end, int8_t move) {
    uint32_t ans = 0;
    char c = (*file).read();
    
    while (c != end) {
        ans *= 10;
        ans += ((uint8_t) c) - 48;
        c = (*file).read();
    }
    
    (*file).seekCur(move - 1);
    
    return ans;
}

//Gets the HTTPS post request based on SD card data
char* remoteGatewayData::getPostSD(unsigned int numLoops, void** toFree) {
    #ifdef DEBUG
    Serial.println(F("Getting post"));
    //delay(250);
    #endif
    
    //Get the size required for sendInfo
    unsigned int infoSize = getSendInfoSize(numLoops);
    if (infoSize == 1) { //No data to send
        char* ans = (char*) malloc(sizeof(char));
        ans[0] = 0;
        return ans;
    } else if (infoSize == -1) { //Ran out of memory
        return NULL;
    }
    
    #ifdef DEBUG
    Serial.println(F("Mem: "));
    Serial.println(freeMemory());
    //delay(250);
    #endif
    
    //Get info on what we need to post
    uint8_t* sendInfo = (uint8_t*) malloc(sizeof(uint8_t) * (infoSize));
    if (sendInfo == NULL) { //Ran out of memory
        return NULL;
    }
    if (!getSendInfo(sendInfo, numLoops)) { //Ran out of memory
        free(sendInfo);
        return NULL;
    }
    
    #ifdef DEBUG
    Serial.println(F("Mem: "));
    Serial.println(freeMemory());
    //delay(250);
    #endif
    
    //Get the size of the data we need to post
    unsigned int dataSize = getDataSize(sendInfo);
    if (dataSize == -1) { //Ran out of memory
        free(sendInfo);
        return NULL;
    }
    
    #ifdef DEBUG
    Serial.print(F("Data size: "));
    Serial.println(dataSize);
    Serial.println(F("Mem: "));
    Serial.println(freeMemory());
    //delay(250);
    #endif
    
    //Get the HTTPS request we need to post
    char* request = (char*) malloc(sizeof(char) * dataSize);
    if (request == NULL) { //Ran out of memory
        free(sendInfo);
        return NULL;
    }
    
    #ifdef DEBUG
    Serial.println(F("Mem: "));
    Serial.println(freeMemory());
    //delay(250);
    #endif
    
    if (!buildRequest(request, sendInfo)) { //Ran out of memory
        free(request);
        free(sendInfo);
        return NULL;
    }
    
    #ifdef DEBUG
    Serial.println(F("Mem: "));
    Serial.println(freeMemory());
    //delay(250);
    #endif
    
    (*toFree) = sendInfo;
    loops = numLoops;
    return request;
}

//Get the size of sendInfo
unsigned int remoteGatewayData::getSendInfoSize(unsigned int numLoops) {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin(SD_CS)) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card (getSendInfoSize)"));
        #endif
        return -1;
    }
    
    //Open ToSend.csv
    SdFile file;
    if (!file.open("ToSend.csv", FILE_READ)) {
        #ifdef DEBUG
        Serial.println(F("Error opening ToSend.csv"));
        #endif
        return -1;
    }
    
    uint8_t nodes = 0;
    
    //Get the number of nodes
    if (!getToSendData(&file, numLoops, &nodes)) {
        file.close();
        return 1;
    }
    
    file.close();
    
    //Calculate and return the total size
    return 1 + 8 * nodes;
}

//Get some data about the ToSend.csv file
bool remoteGatewayData::getToSendData(SdFile* file, unsigned int numLoops, uint8_t* numNodes, uint8_t* lastNum) {
    (*file).seekSet(0);
    
    unsigned int currNum = 0;
    
    while (currNum <= numLoops) {
        while ((*file).read() != 10) ; //Skip header line / rest of line
        
        if (!(*file).available()) return false;
        
        while ((*file).read() != 44) ; //Skip ID
        
        currNum += (uint8_t) fileReadInt(file); //Get number of data points
    }
    
    //Go back to the begining of the line
    while ((*file).read() != 10) {
        (*file).seekCur(-2);
    }
    
    //Get the number of nodes if it's needed
    if (numNodes != NULL) {
        unsigned long position = (*file).curPosition(); //Save the current position
        
        (*numNodes) = 0;
        
        //Loop through the nodes until the end of the file
        while ((*file).available()) {
            if ((*file).read() == 10) {
                (*numNodes)++;
            }
        }
        
        (*file).seekSet(position);
    }
    
    //Get the number of data points of the first node if it's needed
    if (lastNum != NULL) {
        (*lastNum) = currNum - numLoops;
    }
    
    return true;
}

//Get the information on what data needs to be sent
//The returned array has the form(the numbers in the brackets is number of bytes): #nodes(1) id1(2) #locations1(1) #points1(1) FilePostition1(4) id2(2) ...
bool remoteGatewayData::getSendInfo(uint8_t* ans, unsigned int numLoops) {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin(SD_CS)) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card (getSendInfo)"));
        #endif
        return false;
    }
    
    //Open ToSend.csv
    SdFile file;
    if (!file.open("ToSend.csv", FILE_READ)) {
        #ifdef DEBUG
        Serial.println(F("Error opening ToSend.csv"));
        #endif
        return false;
    }
    
    uint8_t nodes;
    uint8_t numPoints;
    
    if (!getToSendData(&file, numLoops, &nodes, &numPoints)) {
        file.close();
        return false;
    }
    
    ans[0] = nodes;
    
    //Go through each node and get it's corresponding information
    for (uint8_t i = 0; i < nodes; i++) {
        //Get the current node's id
        uint16_t currId = (uint16_t) fileReadInt(&file);
        memcpy(&ans[1 + (8 * i)], &currId, sizeof(uint16_t));
        
        if (i == 0) {
             while (file.read() != ','); //Skip number of data points of first node
        } else {
            //Get the number of data points that the current node has to send
            numPoints = (uint8_t) fileReadInt(&file);
        }
        
        //Get the position where the data to send starts
        uint32_t currPos = fileReadInt(&file, '\n');
        
        //Open the current node's file
        char* fileName = (char*) malloc(sizeof(char) * 13);
        if (fileName == NULL) {
            #ifdef DEBUG
            Serial.println(F("Ran out of memory making file name"));
            #endif
            file.close();
            return false;
        }
        
        if (sprintf(fileName, "node%u.csv", currId) < 0) {
            #ifdef DEBUG
            Serial.println(F("Error printing to node file name"));
            #endif
            free(fileName);
            file.close();
            return false;
        }
        
        SdFile nodeFile;
        if (!nodeFile.open(fileName, FILE_READ)) {
            #ifdef DEBUG
            Serial.println(F("Error opening node file"));
            #endif
            free(fileName);
            file.close();
            return false;
        }
        free(fileName);
        
        //Get the number of locations that the current node has to send
        uint8_t numLocs = getNumberLocations(&nodeFile, numPoints, currPos);
        if (numLocs == -1) {
            nodeFile.close();
            file.close();
            return false;
        }
    
        nodeFile.close();
        
        //Save the data into the array
        ans[3 + (8 * i)] = numLocs;
        ans[4 + (8 * i)] = numPoints;
        memcpy(&ans[5 + (8 * i)], &currPos, sizeof(uint32_t));
    }
    
    file.close();
    return true;
}

//Get the number of locations to send for a specific node
uint8_t remoteGatewayData::getNumberLocations(SdFile* file, uint8_t numPoints, unsigned long position) {
    (*file).seekSet(position);
    
    uint8_t locations = 0;
    
    //Go through the number of data points and count the number of locations
    for (uint8_t i = 0; i < numPoints; i++) {
        while ((*file).read() != 44) ; //Skip time
        
        //If the location is non empty, increment counter
        if ((*file).read() != 44) {
            locations++;
        }
        
        while ((*file).read() != 10) ; //Skip the rest of the line
    }
    
    return locations;
}

//Gets the size of the data we need to post
unsigned int remoteGatewayData::getDataSize(uint8_t* sendInfo) {
    unsigned int totalLen = 1; //For number of nodes
    
    for (uint8_t i = 0; i < sendInfo[0]; i++) { //Go through each node that has data to send
        //Get the node's id
        int currId;
        memcpy(&currId, &sendInfo[1 + (8 * i)], sizeof(uint16_t));
        
        uint8_t* currTypesInfo = (uint8_t*) malloc(sizeof(uint8_t) * 3);
        if (currTypesInfo == NULL) {
            return -1;
        }
        
        //Get this node's info
        //First byte is total size that will be taken in final data array, second byte is just number of sensors, third byte is length of name
        if (!getTypesInfo(currTypesInfo, currId)) {
            free(currTypesInfo);
            return -1;
        }
        
        //Add the size that this node will take up
        //Essentially: 4 + total type info size + (num data points * number sensors * 4 bytes)
        totalLen += 4 + currTypesInfo[0] + (sendInfo[4 + (8 * i)] * (currTypesInfo[1] + 1) * 4);
        
        uint8_t locs = sendInfo[3 + (8 * i)]; //Number of locations
        
        if (locs > 0) {
            totalLen += 1 + currTypesInfo[2] + 9 * locs;
        }
        
        free(currTypesInfo);
    }
    
    return (2 * totalLen) + 46 + strlen(URL_Path) + strlen(URL_Host); //Each byte takes two characters, plus size for url
}

//Get sensor type information (plus other information) for a specific node
//First byte is total size that will be taken in final data array, second byte is just number of sensors, third byte is length of name
bool remoteGatewayData::getTypesInfo(uint8_t* typesInfo, uint16_t id) {
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin(SD_CS)) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return false;
    }
    
    //Open the given node's file
    char* fileName = (char*) malloc(sizeof(char) * 13);
    if (fileName == NULL) {
        #ifdef DEBUG
        Serial.println(F("Ran out of memory making file name"));
        #endif
        return false;
    }
        
    if (sprintf(fileName, "node%u.csv", id) < 0) {
        #ifdef DEBUG
        Serial.println(F("Error printing to node file name"));
        #endif
        free(fileName);
        return false;
    }

    SdFile file;
    if (!file.open(fileName, FILE_READ)) {
        #ifdef DEBUG
        Serial.println(F("Error opening node file"));
        #endif
        free(fileName);
        return false;
    }
    free(fileName);
    
    //Get length of name
    typesInfo[2] = 0;
    while (file.read() != 10) {
        typesInfo[2]++;
    }
    
    //Skip Time,Latitude,Longitude
    skipConstTypes(&file);
    
    typesInfo[0] = 1; //Total size of the sensor types (add 1 for length of last type)
    typesInfo[1] = 0; //How many sensor types there are
    
    //Go through the types
    while (file.available()) {
        char c = file.read();
        if (c == 10) {
            break;
        } else if (c == 44) {
            typesInfo[1]++;
        }
        typesInfo[0]++;
    }
    
    file.close();
    return true;
}

//Skip Time,Latitude,Longitude
void remoteGatewayData::skipConstTypes(SdFile* file) {
    for (uint8_t i = 0; i < 3;) {
        char c = (*file).read();
        if (c == 10 || c == 44) {
            i++;
        }
    }
    (*file).seekCur(-1);
}

//Build the HTTPS request we need to post
bool remoteGatewayData::buildRequest(char* request, uint8_t* sendInfo) {
    sprintf(request, "POST %s/sensor/data?data=%02X", URL_Path, sendInfo[0]); //The first part of the HTTPS request
    
    //Go through each node and copy its data into the request
    uint16_t curr = strlen(URL_Path) + 25;
    for (uint8_t i = 0; i < sendInfo[0]; i++) {
        //Get current node id
        uint16_t currId;
        memcpy(&currId, &sendInfo[1 + (8 * i)], sizeof(uint16_t));
        
        //Get current number of locations
        uint8_t currLocs = sendInfo[3 + (8 * i)];
        
        //Get current number of data points
        uint8_t currData = sendInfo[4 + (8 * i)];
        
        //Get postition in the node's file
        uint32_t currPos;
        memcpy(&currPos, &sendInfo[5 + (8 * i)], sizeof(uint32_t));
        
        //Gets the data for the current node and puts it into the request array starting at curr, increments curr accordingly
        if (!getNodeData(request, &curr, currId, currData, currLocs, currPos)) {
            return false;
        }
    }
    
    sprintf(&request[curr], " HTTPS/1.1\r\nHost: %s\r\n\r\n", URL_Host); //The last part of the HTTP request
    
    return true;
}

//Gets data froma specific node and puts it into the request char array
//MAKE NAME AND TYPES JUST NORMAL CHARS???
//CHANGE - NUMBERS IN THINGS (NAME + TYPES)
bool remoteGatewayData::getNodeData(char* request, uint16_t* curr, uint16_t id, uint8_t numData, uint8_t locations, uint32_t position) {
    #ifdef DEBUG
    Serial.print(F("Getting data for node: "));
    Serial.println(id);
    #endif
    
    //Test to make sure there is enough memory for this function to run 
    //(only way I could figure out how to ensure there wasn't an error)
    uint8_t* memTest = (uint8_t*) malloc(sizeof(uint8_t) * 21);
    if (memTest == NULL) {
        return false;
    }
    free(memTest);
    
    //Copy the current node's id into the answer array
    strPrintShort(&request[*curr], id);
    (*curr) += 4;
    
    strPrintByte(&request[*curr], locations); //Put number of locations in
    (*curr) += 2;
    
    //Initialise the microSD card
    SdFat sd;
    if (!sd.begin(SD_CS)) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return false;
    }
    
    //Open the current node's file
    char* fileName = (char*) malloc(sizeof(char) * 13);
    if (fileName == NULL) {
        #ifdef DEBUG
        Serial.println(F("Ran out of memory making file name"));
        #endif
        return false;
    }
        
    if (sprintf(fileName, "node%u.csv", id) < 0) {
        #ifdef DEBUG
        Serial.println(F("Error printing to node file name"));
        #endif
        free(fileName);
        return false;
    }

    SdFile file;
    if (!file.open(fileName, FILE_READ)) {
        #ifdef DEBUG
        Serial.println(F("Error opening node file"));
        #endif
        free(fileName);
        return false;
    }
    
    free(fileName);
    
    //Copy in some location info
    
    unsigned int locationPtr = 0;
    
    if (locations > 0) { //Means we need to put in name and adjust for future location positions
        (*curr) += 2; //For name len, filled in later
        uint8_t nameLen = 0;
        
        while (true) {
            char c = file.read();
            if (c == 10) {
                break;
            }
            nameLen++;
            //request[(*curr)++] = c;
            strPrintByte(&request[*curr], c);
            (*curr) += 2;
        }
        
        strPrintByte(&request[(*curr) - (2*(nameLen+1))], nameLen); //Put length of name in
        
        locationPtr = *curr; //For future use
        
        (*curr) += 2*locations; //Leave room for location info
    } else {
        while (file.read() != 10) ; //Skip name
    }
    
    //Skip Time,Latitude,Longitude
    skipConstTypes(&file);
    
    //Copy in sensor types
    
    //Variables needed to copy in the sensor type names
    unsigned int numPos = *curr; //Position where the total number of types needs to go
    uint8_t currTypeLen = 0; //Length of the current sensor type
    uint8_t types = 0; //Number of sensor types
    (*curr) += 2;
    
    while (true) {
        char c = file.read();
        unsigned int place = (*curr);
        
        if (c == 10) { //If it's a newline
            if (currTypeLen != 0) {
                //Put in the length of the type name that was just copied in
                strPrintByte(&request[place - (2*(currTypeLen + 1))], currTypeLen);
            }
            break;
        } else if (c == 44) { //If it's a comma
            if (currTypeLen == 0) {
                types++;
                (*curr) += 2;
                continue;
            }
            //Don't put in the current character, put len in instead
            place -= (2*(currTypeLen + 1));
            c = currTypeLen;
            currTypeLen = -1; //Reset the length of the current sensor type
            types++;
        }
        //Copy the letter into the request array
        //request[(*curr)++] = c;
        strPrintByte(&request[place], c);
        (*curr) += 2;
        currTypeLen++; //Increase the length of the current sensor type
    }
    
    strPrintByte(&request[numPos], types); //Put in the total number of types that was copied in
    
    file.seekSet(position); //Go to the position where the data points start
    
    //Put the number of data points into the answer array
    strPrintByte(&request[*curr], numData);
    (*curr) += 2;
    
    
    //Go through each data point and copy in time, location, and sensor readings
    
    for (uint8_t i = 1; i <= numData; i++) { //Loop through the number of data points to send
        //Put the current time into the answer array
        uint32_t currTime = fileReadInt(&file);
        strPrintLongP(&request[*curr], &currTime);
        (*curr) += 8;
        
        //Location check and value copy
        
        uint8_t extra = 0;
        if (file.read() != 44) { //Means this data point has a location
            //Indicate that this data point has a location
            strPrintByte(&request[locationPtr], i);
            locationPtr += 2;
            
            file.seekCur(-1);
            extra += 2; //We need to read two extra values
        } else { //Skip the other comma
            file.read();
        }
        
        //Copy in sensor values
        
        char end = ',';
        uint8_t floats = types + extra; //How many times to loop
        for (uint8_t j = 1; j <= floats; j++) {
            if (j == floats) {
                end = '\n';
            }
            
            //Read the next data point and copy it in
            float currData = fileReadFloat(&file, end);
            strPrintLongP(&request[*curr], (uint32_t*) &currData);
            (*curr) += 8;
        }
    }
    
    file.close();
    
    return true;
}

//Read a float from a file, ends on the end character
float remoteGatewayData::fileReadFloat(SdFile* file, char end, uint8_t bufferSize) {
    char str[bufferSize];
    uint8_t curr = 0;
    
    char c = (*file).read();
    while (c != end) {
        str[curr++] = c;
        c = (*file).read();
    }
    str[curr] = 0;
    
    return atof(str);
}

//If the post was successful
void remoteGatewayData::messageSuccessSD() {
    //Initialise the SD card
    SdFat sd;
    if (!sd.begin(SD_CS)) {
        #ifdef DEBUG
        Serial.println(F("Error initialising sd card"));
        #endif
        return;
    }
    
    SdFile file;
    if (!file.open("ToSend.csv", FILE_WRITE)) {
        #ifdef DEBUG
        Serial.println(F("Error openening ToSend.csv"));
        #endif
        return;
    }
    
    uint8_t numPoints;
    
    //Get some information on the node
    if (!getToSendData(&file, loops, NULL, &numPoints)){
        file.close();
        return;
    }
    
    loops = -1;
    
    uint16_t id = (uint16_t) fileReadInt(&file);
    uint8_t oldNumPoints = (uint8_t) fileReadInt(&file);
    uint32_t oldPos = (uint32_t) fileReadInt(&file, '\n');
    
    if (oldNumPoints == numPoints) { //If we can just truncate out this node too
        //Go back to before this node
        file.seekCur(-2);
        while (file.read() != 10) {
            file.seekCur(-2);
        }
    } else { //If we need to edit the data in ToSend.csv
        //Open the current node's file
        char* fileName = (char*) malloc(sizeof(char) * 13);
        if (fileName == NULL) {
            #ifdef DEBUG
            Serial.println(F("Ran out of memory making file name"));
            #endif
            return;
        }
        
        if (sprintf(fileName, "node%u.csv", id) < 0) {
            #ifdef DEBUG
            Serial.println(F("Error printing to node file name"));
            #endif
            free(fileName);
            return;
        }
        
        SdFile nodeFile;
        if (!nodeFile.open(fileName, FILE_READ)) {
            #ifdef DEBUG
            Serial.println(F("Error opening node file"));
            #endif
            free(fileName);
            return;
        }
        free(fileName);

        //Go to old file position
        nodeFile.seekSet(oldPos);

        //Go to new file position
        for (uint8_t i = 0; i < numPoints;) {
            if (nodeFile.read() == 10) {
                i++;
            }
        }
        uint32_t newPos = nodeFile.curPosition();

        nodeFile.close();

        //Go back to in front of number of data points
        for (uint8_t i = 0; i < 2; i++) {
            file.seekCur(-2);
            while (file.read() != 44) {
                file.seekCur(-2);
            }
        }

        //Print the node's new data points and position into the file
        char str[16];
        sprintf(str, "%.3hu,%lu\n", oldNumPoints - numPoints, newPos);
        file.print(str);
    }
    
    //Truncate the file to delete the data that was sent
    file.truncate(file.curPosition());
    
    file.close();
}
#endif


/*--------------------------General--------------------------*/
//Print a byte as hex into a string
void remoteGatewayData::strPrintByte(char* str, uint8_t data) {
    char c = str[2]; //Used to remove trailing null
    sprintf(str, "%02X", data);
    str[2] = c;
}

//Print a short as hex into a string (following little-endian)
void remoteGatewayData::strPrintShort(char* str, uint16_t data) {
    strPrintByte(str, (uint8_t) (data & 0xFF));
    strPrintByte(&str[2], (uint8_t) (data >> 8));
}

//Print the value of a uint32_t* as hex into a string (following little-endian)
void remoteGatewayData::strPrintLongP(char* str, uint32_t* data) {
    strPrintByte(str, (uint8_t) ((*data) & 0xFF));
    strPrintByte(&str[2], (uint8_t) (((*data) >> 8) & 0xFF));
    strPrintByte(&str[4], (uint8_t) (((*data) >> 16) & 0xFF));
    strPrintByte(&str[6], (uint8_t) (((*data) >> 24)));
}