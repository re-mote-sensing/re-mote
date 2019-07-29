/*
Library for using a LoRa mesh module (https://www.dfrobot.com/product-1670.html), used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Spencer Park, Ryan Tyrell, and Victor Vezina, last modified on July 29, 2019
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

    writeFrame(port, 0x01, 0x02, 0x00, NULL);

    uint8_t frameType = 0;
    uint8_t cmdType = 0;
    uint8_t* payload = NULL;

    int len = readFrame(port, &frameType, &cmdType, &payload);

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

    if (frameType != 0x01 || cmdType != 0x82) {
        #ifdef DEBUG
        Serial.println(F("BAD TYPE"));
        #endif
        free(payload);
        port.end();
        return;
    }

    #ifdef DEBUG
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


	// FrameType: 0x01		Configuration parameters for reading and writing modules, etc
	// Command Type: Write configuration information request (0x01)
	writeFrame(port, 0x01, 0x01, 16, payload);

	free(payload);
    
	uint8_t frameType = 0;
	uint8_t cmdType = 0;
	uint8_t * responsePayload = NULL;
    int len = readFrame(port, &frameType, &cmdType, &responsePayload);
    
    bool ans = false;
	if (len != -1 && frameType == 0x01 && cmdType == 0x81) {
		// Application data sending response
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
    return ans;
}

//Send a message and wait for an acknowledgement
uint8_t* remoteLoRa::sendReceive(uint16_t target, uint8_t dataLen, uint8_t* data, unsigned long timeout) {
    //Start the software serial for the module
    NeoSWSerial port(LORA_RX, LORA_TX);
    port.begin(9600);
    
    sendData(port, target, dataLen, data); //Send the data
    
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
    if (!port.available()) {
        return NULL;
    }

    uint8_t frameType = 0;
    uint8_t cmdType = 0;
    uint8_t* payload = NULL;
    int len = readFrame(port, &frameType, &cmdType, &payload);

    if (len == -1 || frameType != 0x05 || cmdType != 0x82) {
        #ifdef DEBUG
        Serial.println(F("Bad Message"));
        #endif
        
        free(payload);
        return NULL;
    }

    uint16_t srcAddr = (payload[0] << 8) | payload[1];
    uint8_t userPayloadLength = payload[3];
    
    #ifdef DEBUG
    uint8_t power = payload[2];
    Serial.print("srcAddr ="); printShort(srcAddr);
    Serial.print(" power ="); printByte(power);
    Serial.print(" userPayloadLength ="); printByte(userPayloadLength);
    Serial.print(" payload =");
    #endif

    uint8_t* ans = (uint8_t*) malloc(sizeof(uint8_t) * (userPayloadLength + 3));
    memcpy(ans, &srcAddr, sizeof(uint16_t));
    ans[2] = userPayloadLength;
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
void remoteLoRa::sendData(Stream& port, uint16_t target, uint8_t dataLen, uint8_t* data) {
    // We add 7 bytes to the head of data for this payload
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

    // frameType = 0x05, cmdType = 0x01 for sendData
    writeFrame(port, 0x05, 0x01, payloadLen, payload);

    free(payload);
    
    #ifdef DEBUG
    uint8_t frameType = 0;
    uint8_t cmdType = 0;
    uint8_t* responsePayload = NULL;
    int len = readFrame(port, &frameType, &cmdType, &responsePayload);

    if (len != -1 && frameType == 0x5 && cmdType == 0x81) {
        // Application data sending response
        uint16_t targetAddr = (responsePayload[0] << 8) | responsePayload[1];
        uint8_t status = responsePayload[2];
        Serial.print(F("targetAddr =")); 
        printShort(targetAddr);
        Serial.print(F(" status ="));
        printByte(status);
        Serial.println();
    }
    
    free(responsePayload);
    #endif
    
    return;
}

//Read a frame from the LoRa module
int remoteLoRa::readFrame(Stream& port, byte* rFrameType, byte* rCmdType, byte** rPayload) {
    uint8_t checksum = 0;
    bool error = false;
    
    uint8_t frameType = readByte(port, &error);
    
    uint8_t frameNum = readByte(port, &error);
    
    uint8_t cmdType = readByte(port, &error);
    uint8_t payloadLen = readByte(port, &error);
    if (error) return -1;

    #ifdef DEBUG
    Serial.print(F("Got: "));
    Serial.println(error);
    printByte(frameType);
    printByte(frameNum);
    printByte(cmdType);
    printByte(payloadLen);
    #endif

    checksum ^= frameType;
    //checksum ^= frameNum;
    checksum ^= cmdType;
    checksum ^= payloadLen;
    
    uint8_t* payload = (uint8_t *) malloc(sizeof(uint8_t) * payloadLen);
    for (int i = 0; i < payloadLen; i++) {
        payload[i] = readByte(port, &error);
        checksum ^= payload[i];
        
        #ifdef DEBUG
        printByte(payload[i]);
        #endif
    }

    *rFrameType = frameType;
    *rCmdType = cmdType;
    *rPayload = payload;

    uint8_t frameCheck = readByte(port, &error);
    if (error) return -1;
    checksum ^= frameCheck;
    
    #ifdef DEBUG
    printByte(frameCheck);
    Serial.println();
    #endif

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

    port.write(frameType);
    port.write((uint8_t) 0); // frameNum
    port.write(cmdType);
    port.write(payloadLen);

    for (int i = 0; i < payloadLen; i++) {
        checksum ^= payload[i];
        port.write(payload[i]);
        
        #ifdef DEBUG
        printByte(payload[i]);
        #endif
    }

    port.write(checksum);
    
    #ifdef DEBUG
    printByte(checksum);
    Serial.println();
    #endif
}

//Read a byte from thw LoRa module
uint8_t remoteLoRa::readByte(Stream& port, bool* error) {
    if (*error) return -1;
    unsigned long start = millis();
    while (!port.available()) {
        if ((millis() - start) > LoRa_Read_Timeout) {
            (*error) = true;
            return -1;
        }
    }
    return port.read();
}

#ifdef DEBUG
void remoteLoRa::printByte(uint8_t b) {
    Serial.print(" 0x");
    if (b <= 0xF)
        Serial.print("0");
    Serial.print(b, HEX);
}

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