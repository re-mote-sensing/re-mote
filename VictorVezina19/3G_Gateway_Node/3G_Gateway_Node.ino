/*----------IMPORTANT INFORMATION AND CONFIG PARAMETERS---------*/

// You have to press the power button on the 3G shield when you power the Arduino

// Change the following parameters to accommodate your specific setup
#define NETWORK_ID 0x0   //LoRa network ID, has to be the same on every LoRa module in your network
#define NODE_ID 0x1234   //LoRa ID of this node, you then need to put this into the end nodes that are sending to this gateway
#define LORA_RX 6      //Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
#define LORA_TX 7      //Pin that the LoRa RXD pin is connected to


/*--------------------------------------------------------------*/
/*-----------------------------CODE-----------------------------*/
/*--------------------------------------------------------------*/

/*---------------------------INCLUDES---------------------------*/

#include <SPI.h>
#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"
#include <SdFat.h>

#include <MemoryFree.h>

/*----------------------DEFINITIONS-----------------------------*/

#define FONA_TX 4
#define FONA_RX 5
#define FONA_RST 9

/*-------------------------GLOBALS------------------------------*/

unsigned long lastPost;
bool dataGood = false;

/*------------------------CONSTRUCTORS--------------------------*/

SoftwareSerial loraPort(LORA_RX, LORA_TX);

/*---------------------------SETUP------------------------------*/

void setup() {
    loraPort.begin(9600);

    Serial.begin(9600);

    Serial.print(F("\n\n1: "));
    Serial.println(freeMemory()); //All these freeMemory() calls are for testing

    writeConfig(loraPort, NETWORK_ID, NODE_ID);
    
    delay(1000);

    Serial.print(F("1.1: "));
    Serial.println(freeMemory());
    
    while (!Serial.available()) ;

    Serial.print(F("2: "));
    Serial.println(freeMemory());

    createToSend();

    Serial.print(F("2.6: "));
    Serial.println(freeMemory());
    
    lastPost = millis();
}

/*----------------------------LOOP------------------------------*/

void loop() {
    loraPort.listen();
    Serial.println(F("Waiting for LoRa messages"));
    
    Serial.print(F("3: "));
    Serial.println(freeMemory());

    while ((millis() >= lastPost) ? ((millis() - lastPost) < 60000) : ((millis() + (4294967295 - lastPost)) < 60000 )) { //Makes sure to check for overflow
        if (loraPort.available()) {

            uint8_t* loraData = readData(loraPort);

            Serial.println(F("Received LoRa message"));
            printByte(loraData[0]);
            printByte(loraData[1]);
            printByte(loraData[2]);
            for (int i = 0; i <= loraData[2]; i++) {
                printByte(loraData[i+3]);
            }
            Serial.println();

            //Once we encrypt data, it will be decrypted here

            uint8_t type = loraData[3] >> 4;
            switch (type) {
                case 0:
                    parseRegistration(loraData);
                    break;

                case 1:
                    saveData(loraData);
                    break;

                default:
                    Serial.println(F("Unknown Message Type Received"));
            }

            free(loraData);
        }
    }
    Serial.println(F("Posting data"));
    
    Serial.print(F("8: "));
    Serial.println(freeMemory());
    
    dataGood = true;
    lastPost = millis();
    postData();
}

/*-------------------------FUNCTIONS----------------------------*/

//Create "ToSend.csv" if it doesn't already exist
void createToSend() {
    SdFat sd;
    
    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    if (!sd.exists("ToSend.csv")) {
        Serial.println(F("Creating ToSend.csv"));
        File file = sd.open("ToSend.csv", FILE_WRITE);
        file.print(F("Node ID, Data Points, Position\n"));
        file.close();
    }
}

//Parse registration message and save new node information
void parseRegistration(uint8_t* data) {
    uint8_t numSensors = data[3] & 0x0F;
    uint8_t curr = 4;
    char *types[numSensors];
    for (uint8_t i = 0; i < numSensors; i++) {
        uint8_t len = data[curr++];
        char *currType = malloc(sizeof(char) * (len + 1));
        currType[len] = 0;
        for (uint8_t j = 0; j < len; j++) {
	        currType[j] = data[curr++];
        }
        types[i] = currType;
    }

    SdFat sd;

    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }

    uint16_t add = data[0] << 8 | data[1];
    char* fileName = malloc(sizeof(char) * 13);
    sprintf(fileName, "node%u.csv", add);

    if (sd.exists(fileName)) { //Receiving registration from already registered node, check to see if good or not
        File file = sd.open(fileName, FILE_READ);
        file.seekSet(5);
        uint8_t currTypeNum = 0;
        bool err = false;
        for (uint8_t i = 0; i < numSensors; i++) {
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
        if (err) {
            Serial.print(F("Received bad registration data from node "));
            Serial.println(add);
        } else {
            uint8_t* ackData = malloc(sizeof(uint8_t));
            ackData[0] = 0;
            sendData(loraPort, add, 1, ackData);
            free(ackData);
        }
    } else { //Receiving new data (registering new node)
        File file = sd.open(fileName, FILE_WRITE);
        file.print(F("Time,"));
        for (uint8_t i = 0; i < numSensors; i++) {
            file.print(types[i]);
            if ((i+1) != numSensors) {
                file.print(F(","));
            }
        }
        file.print(F("\n"));
        file.close();
        uint8_t* ackData = malloc(sizeof(uint8_t));
        ackData[0] = 0;
        sendData(loraPort, add, 1, ackData);
        free(ackData);
    }

    for (uint8_t i = 0; i < numSensors; i++) {
        free(types[i]);
    }
    free(fileName);
}

//Save the received data into the sd card
void saveData(uint8_t* data) {
    SdFat sd;

    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }

    uint16_t add = data[0] << 8 | data[1];
    char* fileName = malloc(sizeof(char) * 13);
    sprintf(fileName, "node%u.csv", add);
    
    uint8_t payloadLen = data[2];
    uint8_t numSensors = data[3] & 0x0F;
    
    if (!sd.exists(fileName)) {
        Serial.print(F("Received sensor data before registration from node "));
        Serial.println(add);
    } else {
        File file = sd.open(fileName, FILE_WRITE);
        if (!checkSensorData(file, numSensors)) {
            Serial.print(F("Received bad sensor data from node "));
            Serial.println(add);
        } else {
            dataGood = true;
            
            File toSendFile = sd.open("ToSend.csv", FILE_WRITE);
            toSendFile.seekSet(31);
            uint32_t position = file.curPosition();

            bool notFound = false;
            uint16_t currId;
            do {
                if (!toSendFile.available()) {
                    notFound = true;
                    break;
                }
                char currStr[12];
                int len = toSendFile.fgets(currStr, 12, ",");
                currStr[len-2] = 0;
                currId = atoi(currStr);

                while (toSendFile.read() != 10); //Go to end of line
            } while (currId != add);
            
            uint8_t numData = 0;
            for (uint8_t i = 4; i < payloadLen + 3; ) {
                numData++;
                unsigned long time;
                memcpy(&time, &data[i], sizeof(uint8_t) * 4);
                i += 4;
                file.print(time);
                file.print(F(","));
                for (uint8_t j = 0; j < numSensors; j++) {
                    float currFloat;
                    memcpy(&currFloat, &data[i], sizeof(uint8_t) * 4);
                    i += 4;
                    file.print(currFloat);
                    if (j+1 != numSensors) {
                        file.print(F(","));
                    }
                }
                file.print("\n");
            }

            if (notFound) {
                toSendFile.print(add);
                toSendFile.print(F(","));
                
                char numDataStr[4];
                sprintf(numDataStr, "%.3hu", numData);
                toSendFile.print(numDataStr);
                
                toSendFile.print(F(","));
                toSendFile.print(position);
                toSendFile.print("\n");
            } else {
                for (uint8_t i = 0; i < 2; i++) {
                    toSendFile.seekCur(-1);
                    while (toSendFile.peek() != 44) { // ','
                        toSendFile.seekCur(-1);
                    }
                }
                
                toSendFile.read();
                //Position will now be about to read number of data points
                
                char currStr[12];
                int len = toSendFile.fgets(currStr, 12, ",");
                currStr[len-1] = 0; //Overwrite ','
                uint8_t numPoints = atoi(currStr);
                
                toSendFile.seekCur(-2);
                while (toSendFile.peek() != 44) { // ','
                    toSendFile.seekCur(-1);
                }
                toSendFile.read();
                //Position will now be about to write number of data points
                
                char numPointsStr[4];
                sprintf(numPointsStr, "%.3hu", numData + 1);
                toSendFile.print(numPointsStr);
            }

            toSendFile.close();
            
            uint8_t* ackData = malloc(sizeof(uint8_t));
            ackData[0] = 0;
            sendData(loraPort, add, 1, ackData);
            free(ackData);
        }
        file.close();
    }
    free(fileName);
}

//Check that the number of sensors given is the same as what's in the node file
bool checkSensorData(File file, uint8_t numSensors) {
    uint32_t position = file.curPosition();
    file.seekSet(0);
    uint8_t sum = 0;
    while (file.peek() != 10) {
        if (file.read() == 44) {
            sum++;
        }
    }
    if (sum != numSensors) {
        return false;
    } else {
        file.seek(position);
        return true;
    }
}

//Post collected data to webserver
void postData() {
    if (!dataGood) {
        return;
    }

    Serial.print(F("10: "));
    Serial.println(freeMemory());

    //Build HTTP request
    char* reqArr = buildRequest();
    
    Serial.print(F("12: "));
    Serial.println(freeMemory());

    /*
    Serial.print(F("-------------------------------------\n\n\n"));
    Serial.print(reqArr);
    Serial.println(F("\n\n\n-------------------------------------"));
    */

    fonaPost(reqArr);
}

//Post a request through the fona
void fonaPost(char* reqArr) {
    //Initialising the FONA:
    SoftwareSerial fonaSS(FONA_TX, FONA_RX);
    Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);
    
    delay(500);
    
    fonaSS.begin(4800);
    if (!fona.begin(fonaSS)) {
        Serial.println(F("Error initialising FONA"));
        while (true);
    }

    delay(500);

    fona.setGPRSNetworkSettings(F("pda.bell.ca"));
    fona.setHTTPSRedirect(true);

    delay(2000);
    
    Serial.print(F("12.5: "));
    Serial.println(freeMemory());
    
    bool err = false;
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!fona.sendCheckReply(F("AT+CHTTPSSTART"), F("OK"), 30000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }
    
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!fona.sendCheckReply(F("AT+CHTTPSOPSE=\"www.cas.mcmaster.ca\",80,1"), F("OK"), 10000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }
    
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!fona.sendCheckReply(F("AT+CHTTPSSEND=120"), F(">"), 10000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }
    
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!fona.sendCheckReply(reqArr, F("OK"), 10000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }
    
    for (uint8_t i = 0; !err && i < 3; i++) {
        if (!fona.sendCheckReply(F("AT+CHTTPSCLSE"), F("OK"), 10000)) {
            if (i == 2) {
                err = true;
            }
        } else {
            break;
        }
    }

    free(reqArr);
    
    if (!err) {
        truncateToSend();
        dataGood = false;
    }

    Serial.print(F("13: "));
    Serial.println(freeMemory());

    fonaSS.end();
}

//Build HTTP request to send through fona
char* buildRequest() {
    Serial.print(F("11.1: "));
    Serial.println(freeMemory());
    
    uint8_t* data = getAllData();
    
    Serial.print(F("11.2: "));
    Serial.println(freeMemory());
    
    unsigned int dataSize;
    memcpy(&dataSize, data, sizeof(uint8_t) * 2);
    dataSize -= 2; //Don't need to account for dataSize
    
    Serial.print(F("Data size: "));
    Serial.println(dataSize);
    
    char* request = malloc(sizeof(char) * (64 + (2 * dataSize) + 1));
    
    if (request == NULL) {
        Serial.println(F("Ran out of memory while building request"));
    }
    
    Serial.print(F("11.3: "));
    Serial.println(freeMemory());
    
    strcpy(request, "POST /ollie/sensor/data HTTPS/1.1\r\nHost: www.cas.mcmaster.ca\r\n\r\n");
    
    Serial.print(F("11.4: "));
    Serial.println(freeMemory());
    
    for (uint8_t i = 0; i < dataSize; i++) {
        char* curr = byteToHexStr(data[i + 2]);
        request[(2 * i) + 64] = curr[0];
        request[(2 * i) + 65] = curr[1];
        free(curr);
    }
    
    Serial.print(F("11.5: "));
    Serial.println(freeMemory());
    
    request[64 + (2 * dataSize)] = 0;
    
    free(data);
    
    Serial.print(F("11.6: "));
    Serial.println(freeMemory());
    
    return request;
}

//Converts a uint8_t (byte) into a char* representing the hexadecimal form of the byte
char* byteToHexStr(uint8_t data) {
    char* ans = malloc(sizeof(char) * 2);;
    sprintf(ans, "%02X", data);
    return ans;
}

//Get data for all nodes for HTTP request
uint8_t* getAllData() {
    Serial.print(F("11.11: "));
    Serial.println(freeMemory());
    
    uint8_t* sendInfo = getSendInfo();
    //#nodes(1) id1(2) #points1(1) pos1(4) id2(2) ...
    
    Serial.print(F("11.12: "));
    Serial.println(freeMemory());
    
    unsigned int totalLen = 3; //2 for data size (just used in converting to string), 1 for # of nodes
    
    for (uint8_t i = 0; i < sendInfo[0]; i++) {
        int currId;
        memcpy(&currId, &sendInfo[1 + (7 * i)], sizeof(uint8_t) * 2);
        totalLen += 2; //For id
        
        uint8_t* currTypesInfo = malloc(sizeof(uint8_t) * 2);
        getTypesInfo(currTypesInfo, currId); //Size of types (including byte for #, lens, and actual types) and # types
        
        totalLen += currTypesInfo[0]; //Size of types information
        totalLen += 1; //For # data points
        
        //totalLen += #DataPoints * #Types * 4 bytes
        totalLen += sendInfo[3 + (7 * i)] * currTypesInfo[1] * 4;
        
        free(currTypesInfo);
    }
    
    Serial.print(F("11.13: "));
    Serial.println(freeMemory());
    
    Serial.println(totalLen);
    
    uint8_t* ans = malloc(sizeof(uint8_t) * totalLen);
    
    ans[2] = sendInfo[0];
    
    Serial.print(F("11.14: "));
    Serial.println(freeMemory());
    
    unsigned int curr = 3;
    for (uint8_t i = 0; i < sendInfo[0]; i++) {
        int currId;
        memcpy(&currId, &sendInfo[1 + (7 * i)], sizeof(uint8_t) * 2);
        
        unsigned long currPos;
        memcpy(&currPos, &sendInfo[4 + (7 * i)], sizeof(uint8_t) * 4);
        
        getNodeData(ans, &curr, currId, currPos); //Puts node data into ans starting at curr
    }
    
    free(sendInfo);
    
    memcpy(ans, &curr, sizeof(uint8_t) * 2); //Data size
    
    Serial.print(F("11.15: "));
    Serial.println(freeMemory());
    
    return ans;
}

//Get the information on what data needs to be sent
uint8_t* getSendInfo() {
    SdFat sd;

    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    File file = sd.open("ToSend.csv", FILE_READ);
    file.seekSet(31);
    
    uint8_t numNodes = 0;
    while (file.available()) {
        while (file.read() != 10); //Go to end of line
        numNodes++;
    }
    
    file.seekSet(31);
    
    uint8_t* ans = malloc(sizeof(uint8_t) * (1 + (7 * numNodes)));
    ans[0] = numNodes;
    
    for (uint8_t i = 0; i < numNodes; i++) {
        char currStr[12];
        
        int len = file.fgets(currStr, 12, ",");
        currStr[len-1] = 0; //Overwrite ','
        uint16_t currId = atoi(currStr);
        
        len = file.fgets(currStr, 12, ",");
        currStr[len-1] = 0; //Overwrite ','
        uint8_t numPoints = atoi(currStr);
        
        len = file.fgets(currStr, 12, "\n");
        currStr[len-1] = 0; //Overwrite '\n'
        uint32_t currPos = atol(currStr);
        
        memcpy(&ans[1 + (7 * i)], &currId, sizeof(uint8_t) * 2);
        ans[3 + (7 * i)] = numPoints;
        memcpy(&ans[4 + (7 * i)], &currPos, sizeof(uint8_t) * 4);
    }
    
    file.close();
    
    return ans;
}

//Get some information of the sensor types of a specific node
void getTypesInfo(uint8_t* ans, int id) {
    SdFat sd;

    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    char fileName[13];
    sprintf(fileName, "node%u.csv", id);
    File file = sd.open(fileName, FILE_READ);
    
    char currStr[12];
    
    uint8_t typesSize = 1; //Won't count the last type, so add 1
    uint8_t numTypes = 1; //Add 1 to account for number of types
    while (file.peek() != 10) { // "\n"
        if (file.read() != 44) { // ","
            typesSize++;
        } else {
            numTypes++;
        }
    }
    file.close();
    
    typesSize += numTypes; //Account for len of each type
    
    ans[0] = typesSize;
    ans[1] = numTypes;
    
    return;
}

//Gets the data to be sent from a specific node
//Form of data: id numTypes len1 type1 len2 type2 ... numData data1Type1 data1Type2 ... data2Type1 data2Type2 ...
void getNodeData(uint8_t* ans, unsigned int* ansCurr, int id, unsigned long pos) {
    //Currently this copies in Time as a type, but that's just a waste of space

    Serial.print(F("Getting data for node: "));
    Serial.println(id);
    
    memcpy(&ans[*ansCurr], &id, sizeof(uint8_t) * 2);
    (*ansCurr) += 2;
    
    SdFat sd;

    if (!sd.begin()) {
        Serial.println(F("Error initialising sd card"));
        return;
    }
    
    char fileName[13];
    sprintf(fileName, "node%u.csv", id);
    File file = sd.open(fileName, FILE_READ);
    file.seekSet(0); //Should skip time
    
    Serial.print(F("11.14201: "));
    Serial.println(freeMemory());
    
    unsigned int numPos = *ansCurr;
    uint8_t currTypeLen = 0;
    uint8_t currNum = 1;
    (*ansCurr) += 2;
    
    while (file.peek() != 10) { // '\n'
        if (file.peek() == 44) { // ','
            ans[((*ansCurr)++) - (currTypeLen + 1)] = currTypeLen;
            currNum++;
            currTypeLen = 0;
            file.read();
        } else {
            ans[(*ansCurr)++] = file.read();
            currTypeLen++;
        }
    }
    
    ans[(*ansCurr) - (currTypeLen + 1)] = currTypeLen;
    ans[numPos] = currNum;
    
    Serial.print(F("11.14202: "));
    Serial.println(freeMemory());
    
    file.seekSet(pos);
    numPos = *ansCurr;
    currNum = 0;
    (*ansCurr)++;
    
    while (file.available()) {
        char dataStr[13];
        uint8_t len = file.fgets(dataStr, 13, ",");
        dataStr[len-1] = 0; //Overwrite ','
        unsigned long currTime = strtoul(dataStr, NULL, 10);
        
        Serial.print(F("TIME: "));
        Serial.println(currTime);
        
        memcpy(&ans[*ansCurr], &currTime, sizeof(uint8_t) * 4);
        (*ansCurr) += 4;
        
        float currData;
        uint8_t currDataStr = 0;
        while (file.peek() != 10) { // '\n'
            if (file.peek() == 44) { // ','
                dataStr[currDataStr] = 0;
                currDataStr = 0;
                currData = atof(dataStr);
                memcpy(&ans[*ansCurr], &currData, sizeof(uint8_t) * 4);
                (*ansCurr) += 4;
                file.read();
            } else {
                dataStr[currDataStr++] = file.read();
            }
        }
        dataStr[currDataStr] = 0;
        currData = atof(dataStr);
        memcpy(&ans[*ansCurr], &currData, sizeof(uint8_t) * 4);
        (*ansCurr) += 4;
        file.read();
        currNum++;
    }
    
    file.close();
    
    ans[numPos] = currNum;
    
    Serial.print(F("11.14203: "));
    Serial.println(freeMemory());
    
    return;
}

//Truncate ToSend.csv to account for sent data
void truncateToSend() {
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


/*-------------------LOWER LEVEL LORA FUNCTIONS-------------------*/

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