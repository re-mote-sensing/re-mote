/*
Library for using a LoRa mesh module (https://www.dfrobot.com/product-1670.html), used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Spencer Park, Ryan Tyrell, and Victor Vezina, last modified on August 13, 2019
Released into the public domain
*/

#include "remoteLoRa.h"
#include <remoteConfig.h>
#include <NeoSWSerial.h>

//Start the LoRa
remoteLoRa::remoteLoRa() {
}

//Read the current LoRa module's configuration
void remoteLoRa::readConfig() {
    //Start LoRa software serial
    NeoSWSerial port(LORA_RX, LORA_TX);
    port.begin(9600);

    //Send the read configuration command
    writeFrame(port, 0x01, 0x02, 0x00, NULL);

    uint8_t frameType = 0;
    uint8_t cmdType = 0;
    uint8_t* payload = NULL;

    //Read the response
    int len = readFrame(port, &frameType, &cmdType, &payload);

    //If the read timesout
    if (len < 0) {
        #ifdef DEBUG
        Serial.println(F("ERROR"));
        #endif
        free(payload);
        port.end();
        return;
    }

    #ifdef DEBUG
    Serial.print(F("FrameType="));
    printByte(frameType);
    Serial.print(F(" CmdType="));
    printByte(cmdType);
    #endif

    //If it's not the correct response type
    if (frameType != 0x01 || cmdType != 0x82) {
        #ifdef DEBUG
        Serial.println(F("BAD TYPE"));
        #endif
        free(payload);
        port.end();
        return;
    }

    #ifdef DEBUG
    //Print the response
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
    #endif
    
    port.end();
}

//Write config parameters to the LoRa module
bool remoteLoRa::writeConfig(uint16_t netID, uint16_t nodeID) {
    //Start LoRa software serial
    NeoSWSerial port(LORA_RX, LORA_TX);
    port.begin(9600);
    
    //Allocate memory for the payload
	uint8_t* payload = (uint8_t*)malloc(sizeof(uint8_t) * 16);

    //This is just the next few lines put into one; it's slightly more flash efficient
    uint32_t payload1 = 0x0001A5A5;
    memcpy(payload, &payload1, sizeof(uint32_t));
    
	// Configuration flag - 2-byte short
	//payload[0] = (uint8_t)((0xA5A5 >> 8) & 0xFF);
	//payload[1] = (uint8_t)(0xA5A5 & 0xFF);

	// Channel Number
	//payload[2] = (uint8_t)1;

	// RF transmit power (tx_power)
	//payload[3] = (uint8_t)0;

	// User interface mode
	payload[4] = (uint8_t)0;

	// Equpment type
	payload[5] = (uint8_t)1;

	// Network ID - 2-byte short
	payload[6] = (uint8_t)(netID >> 8);
	payload[7] = (uint8_t)(netID);

	// Node ID - 2-byte short
	payload[8] = (uint8_t)((nodeID >> 8) & 0xFF);
	payload[9] = (uint8_t)(nodeID & 0xFF);

	// Reserved - 2-byte short
	payload[10] = (uint8_t)(0);
    payload[11] = (uint8_t)(0);
    
	// Reserved
	payload[12] = (uint8_t)1;

	// Serial port Parameter
	payload[13] = (uint8_t)0x40;

	// Air Rate - 2-byte short
	payload[14] = (uint8_t)(0x09);
	payload[15] = (uint8_t)(0x09);


	//Write the write configuration command with the given parameters
	writeFrame(port, 0x01, 0x01, 16, payload);

	free(payload);
    
    //Read the response from the LoRa
	uint8_t frameType = 0;
	uint8_t cmdType = 0;
	uint8_t * responsePayload = NULL;
    int len = readFrame(port, &frameType, &cmdType, &responsePayload);
    
    bool ans = false;
	if (len != -1 && frameType == 0x01 && cmdType == 0x81) { //If the response is good
		uint8_t status = responsePayload[0];
        if (status == 0) {
            ans = true;
        }
        #ifdef DEBUG
		Serial.print("status ="); printByte(status);
		Serial.println();
        #endif
	}
    
	free(responsePayload);
    port.end();
    return ans; //Return if it worked or not
}

//Send a message and wait for an acknowledgement
uint8_t* remoteLoRa::sendReceive(uint16_t target, uint8_t dataLen, uint8_t* data, unsigned long timeout) {
    //Start the software serial for the module
    NeoSWSerial port(LORA_RX, LORA_TX);
    port.begin(9600);
    
    //Send the data
    if (!sendData(port, target, dataLen, data)) {
        port.end();
        return NULL;
    }
    
    unsigned long before = millis(); //Time before waiting for the acknowledgement
    
    uint8_t* ans = NULL; //The message received
    
    //Loop until an acknowledgement is received or timeout time has passed
    while ((millis() - before) < timeout) {
        uint8_t* data = readData(port); //Read from the LoRa module
        if (data != NULL) { //If a LoRa message was received
            uint16_t dataAdd; //The address the message was received from
            memcpy(&dataAdd, &data[0], sizeof(uint8_t) * 2); //Get the address from the LoRa message
            
            if (dataAdd == target) { //Check that the received message is from address
                ans = data; //The answer to return is the received message
                break;
            }
        }
    }
    
    //End the port
    port.end();
    
    return ans;
}

//Read some data from the LoRa module
uint8_t* remoteLoRa::readData() {
    //Start the software serial for the module
    NeoSWSerial port(LORA_RX, LORA_TX);
    port.begin(9600);
    
    //Read the data
    uint8_t* ans = readData(port);
    
    //End the port
    port.end();
    
    return ans;
}

//Send data through the LoRa module
//Max length is 111 bytes
void remoteLoRa::sendData(uint16_t target, uint8_t dataLen, uint8_t* data) {
    //Start the software serial for the module
    NeoSWSerial port(LORA_RX, LORA_TX);
    port.begin(9600);
    
    //Send the data
    sendData(port, target, dataLen, data);
    
    //End the port
    port.end();
}


/*---------------------PRIVATE---------------------*/

//Read data from a given port
uint8_t* remoteLoRa::readData(Stream& port) {
    delay(100); //Let data come in
    
    if (!port.available()) { //If there's no data
        return NULL;
    }

    //Read data from the LoRa
    uint8_t frameType = 0;
    uint8_t cmdType = 0;
    uint8_t* payload = NULL;
    int len = readFrame(port, &frameType, &cmdType, &payload);

    //If it's not a valid message
    if (len == -1 || frameType != 0x05 || cmdType != 0x82) {
        #ifdef DEBUG
        Serial.println(F("Bad Message read"));
        #endif
        
        free(payload);
        return NULL;
    }

    //Get some info from the payload
    uint16_t srcAddr = (payload[0] << 8) | payload[1];
    uint8_t userPayloadLength = payload[3];
    
    #ifdef DEBUG
    //Print the info from the message
    uint8_t power = payload[2];
    Serial.print("srcAddr ="); printShort(srcAddr);
    Serial.print(" power ="); printByte(power);
    Serial.print(" userPayloadLength ="); printByte(userPayloadLength);
    Serial.print(" payload =");
    #endif

    //Allocate memory for the array to return
    uint8_t* ans = (uint8_t*) malloc(sizeof(uint8_t) * (userPayloadLength + 3));
    
    //Add some info from the message to the array
    memcpy(ans, &srcAddr, sizeof(uint16_t));
    ans[2] = userPayloadLength;
    
    //Fill the rest of the array with the message
    for (uint8_t i = 0; i < userPayloadLength; i++) {
        ans[i+3] = payload[4+i];
        
        #ifdef DEBUG
        printByte(payload[4 + i]);
        #endif
    }
    
    #ifdef DEBUG
    Serial.println();
    #endif
    
    free(payload);
    return ans;
}

//Send data through a given port
bool remoteLoRa::sendData(Stream& port, uint16_t target, uint8_t dataLen, uint8_t* data) {
    // We add 6 bytes to the head of data for this payload
    uint8_t payloadLen = 6 + dataLen;
    uint8_t* payload = (uint8_t *) malloc(sizeof(uint8_t) * payloadLen);

    // target address as big endian short
    payload[0] = (uint8_t) (target >> 8);
    payload[1] = (uint8_t) (target);

    // ACK request == 1 -> require acknowledgement of recv
    // Doesn't seem to work
    payload[2] = (uint8_t) 0;

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

    // Data from index 6 to the end should be the data
    memcpy(&payload[6], data, dataLen);

    //Send the data through the LoRa
    writeFrame(port, 0x05, 0x01, payloadLen, payload);

    free(payload);
    
    //Read the response from the LoRa
    uint8_t frameType = 0;
    uint8_t cmdType = 0;
    uint8_t* responsePayload = NULL;
    int len = readFrame(port, &frameType, &cmdType, &responsePayload);
    
    bool ans = true;

    if (len != -1 && frameType == 0x5 && cmdType == 0x81) { //If it's a valid response
        #ifdef DEBUG
        // Application data sending response
        uint16_t targetAddr = (responsePayload[0] << 8) | responsePayload[1];
        uint8_t status = responsePayload[2];
        Serial.print(F("targetAddr =")); 
        printShort(targetAddr);
        Serial.print(F(" status ="));
        printByte(status);
        Serial.println();
        #endif
    } else {
        //If it's not a valid response, print what happened
        #ifdef DEBUG
        Serial.print(F("Len: "));
        Serial.println(len);
        if (len != -1) {
            uint16_t targetAddr = (responsePayload[0] << 8) | responsePayload[1];
            uint8_t status = responsePayload[2];
            Serial.println(F("Bad Message send"));
            Serial.print(F("targetAddr =")); 
            printShort(targetAddr);
            Serial.print(F(" status ="));
            printByte(status);
            Serial.println();
            for (uint8_t i = 0; i < responsePayload[3]; i++) {
                printByte(responsePayload[4+i]);
            }
            Serial.println();
            Serial.println();
        }
        #endif
        ans = false;
    }
    
    free(responsePayload);
    
    return ans;
}

//Read a frame from the LoRa module
int remoteLoRa::readFrame(Stream& port, byte* rFrameType, byte* rCmdType, byte** rPayload) {
    uint8_t checksum = 0;
    bool error = false;
    
    //Read the basic information of the message
    uint8_t frameType = readByte(port, &error);
    uint8_t frameNum = readByte(port, &error);
    uint8_t cmdType = readByte(port, &error);
    uint8_t payloadLen = readByte(port, &error);
    
    if (error) return -1; //If one of the reads timed out

    #ifdef DEBUG
    Serial.print(F("Got:"));
    //Serial.println(error);
    printByte(frameType);
    printByte(frameNum);
    printByte(cmdType);
    printByte(payloadLen);
    #endif

    //Make sure to keep checksum updated
    checksum ^= frameType;
    //checksum ^= frameNum;
    checksum ^= cmdType;
    checksum ^= payloadLen;
    
    //Read the payload from the LoRa
    uint8_t* payload = (uint8_t *) malloc(sizeof(uint8_t) * payloadLen);
    for (int i = 0; i < payloadLen; i++) {
        payload[i] = readByte(port, &error);
        checksum ^= payload[i];
        
        #ifdef DEBUG
        printByte(payload[i]);
        #endif
    }

    //Set the passed in parameters
    *rFrameType = frameType;
    *rCmdType = cmdType;
    *rPayload = payload;

    //Read the final byte from the LoRa
    uint8_t frameCheck = readByte(port, &error);
    
    if (error) return -1; //If the read timedout
    
    checksum ^= frameCheck;
    
    #ifdef DEBUG
    printByte(frameCheck);
    Serial.println();
    #endif

    //Check that the checksum is correct
    if (checksum != 0) {
        #ifdef DEBUG
        Serial.println(F("Checksum Problem!"));
        #endif
        
        return -1;
    }
    return payloadLen;
}

//Write a frame to the LoRa module
void remoteLoRa::writeFrame(Stream& port, uint8_t frameType, uint8_t cmdType, uint8_t payloadLen, uint8_t* payload) {
    uint8_t checksum = 0;

    //Calculate the checksum
    checksum ^= frameType;
    checksum ^= 0; // frameNum which is unused and always 0
    checksum ^= cmdType;
    checksum ^= payloadLen;

    #ifdef DEBUG
    printByte(frameType);
    printByte((uint8_t) 0); // frameNum
    printByte(cmdType);
    printByte(payloadLen);
    #endif

    //Print the basic message informations
    port.write(frameType);
    port.write((uint8_t) 0); // frameNum
    port.write(cmdType);
    port.write(payloadLen);

    //Print the payload to the LoRa
    for (int i = 0; i < payloadLen; i++) {
        checksum ^= payload[i];
        port.write(payload[i]);
        
        #ifdef DEBUG
        printByte(payload[i]);
        #endif
    }

    //Print the checksum at the end of the message
    port.write(checksum);
    
    #ifdef DEBUG
    printByte(checksum);
    Serial.println();
    #endif
}

//Read a byte from thw LoRa module
uint8_t remoteLoRa::readByte(Stream& port, bool* error) {
    if (*error) return -1; //If an error was passed in, just exit
    
    unsigned long start = millis(); //The time that we are starting to read
    
    //Wait until a byte is available
    while (!port.available()) {
        //If we timeout of reading a byte
        if ((millis() - start) > LoRa_Read_Timeout) {
            (*error) = true;
            return -1;
        }
    }
    return port.read();
}

#ifdef DEBUG
//Print a byte as hex to the Serial monitor
void remoteLoRa::printByte(uint8_t b) {
    Serial.print(" 0x");
    if (b <= 0xF)
        Serial.print("0");
    Serial.print(b, HEX);
}

//Print a short as hex to the Serial monitor
void remoteLoRa::printShort(uint16_t s) {
    Serial.print(" 0x");
    if (s <= 0xFFF)
        Serial.print("0");
    if (s <= 0xFF)
        Serial.print("0");
    if (s <= 0xF)
        Serial.print("0");
    Serial.print(s, HEX);
}
#endif