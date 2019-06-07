/*----------IMPORTANT INFORMATION AND CONFIG PARAMETERS---------*/

// Change the following parameters to accommodate your specific setup
#define NETWORK_ID 0x0             //LoRa network ID, has to be the same on every LoRa module in your network
#define NODE_ID 0x0                //LoRa ID of this node, must be unique to all nodes in a network
#define GATEWAY_ADDRESS 0x1234   //ID of the gateway node to send data to
#define LORA_RX 3                //Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
#define LORA_TX 2                //Pin that the LoRa RXD pin is connected to

// The following paramaters have to do with the sensors you're using on this end node
// Look on our GitLab (LINK!) for more information on sensor setup and how to edit these values
#define NUMBER_SENSORS 3 //Max is 15
char* sensorTypes[NUMBER_SENSORS] = {"Dissolved_Oxygen", "Conductivity", "Turbidity"};
uint8_t sensorPorts[NUMBER_SENSORS][2] = { {7, 6}, {5, 4}, {8, 14} };


/*--------------------------------------------------------------*/
/*-----------------------------CODE-----------------------------*/
/*--------------------------------------------------------------*/

/*---------------------------INCLUDES---------------------------*/

#include <SPI.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

/*----------------------LORA CONSTRUCTOR------------------------*/

SoftwareSerial loraPort(LORA_RX, LORA_TX);

/*---------------------------SETUP------------------------------*/

void setup() {
    Serial.begin(9600);
    
    initialiseSensors();
    
    loraPort.begin(9600);
    delay(750);
    writeConfig(loraPort, NETWORK_ID, NODE_ID);
    delay(750);
    
    registerNode();

    //Check if EEPROM has ever been used
    unsigned int initialCheck = 0;
    EEPROM.get(0, initialCheck);
    if (initialCheck >= EEPROM.length()) {
        int test = 2;
        EEPROM.put(0, test);
        Serial.println(F("Set EEPROM[0] to 2"));
    }

    //int i = 2;
    //EEPROM.put(0, i);
    
    delay(500);
}

/*----------------------------LOOP------------------------------*/

void loop() {
    unsigned long start = millis();
    
    saveData(readSensors());
    
    loraPort.listen();
    
    bool received;
    int memAddress = 2;
    do {
        received = sendSensorData();
        if (!received) {
            break;
        } else {
            Serial.println(F("Recieved ack"));

            EEPROM.get(0, memAddress);
            delay(250); //Small delay to not flood the gateway
        }
    } while (memAddress > 2); //Keep sending data if there's more to send
    
    if (!received) {
        Serial.println(F("Didn't recieve ack"));
        delayOverflow(45000, start);
    } else {
        delayOverflow(60000, start);
    }
}

/*-----------------------ALL FUNCTIONS--------------------------*/


/*---------------Sensor Initialisation Functions----------------*/
//Initialise the sensors
void initialiseSensors() {
    for (uint8_t i = 0; i < NUMBER_SENSORS; i++) {\
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
    SoftwareSerial sensor(sensorPorts[i][0], sensorPorts[i][1]);
    sensor.begin(9600);
    
    //Just sending sleep didn't work
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
    pinMode(sensorPorts[i][0], OUTPUT);
    digitalWrite(sensorPorts[i][0], LOW);
    
    return;
}


/*---------------Node Registration Function----------------*/

//Registers this end node with the gateway
void registerNode() {
    uint8_t arrLength = 1 + NUMBER_SENSORS;
    for (uint8_t i = 0; i < NUMBER_SENSORS; i++) {
        arrLength += strlen(sensorTypes[i]);
    }
    
    if (arrLength > 111) {
        Serial.println(F("ERROR: Sensor types too long for LoRa message"));
        while (true) ;
    }

    uint8_t* dataArr = malloc(sizeof(uint8_t) * arrLength);

    dataArr[0] = ((uint8_t) NUMBER_SENSORS) & 0b00001111;
    uint8_t curr = 1;
    for (uint8_t i = 0; i < NUMBER_SENSORS; i++) {
        uint8_t currLen = strlen(sensorTypes[i]);
        memcpy(&dataArr[curr++], &currLen, sizeof(uint8_t));
        for (uint8_t j = 0; j < strlen(sensorTypes[i]); j++) {
            memcpy(&dataArr[curr++], &sensorTypes[i][j], sizeof(uint8_t));
        }
    }
    
    loraPort.listen();
    
    bool received = false;
    while (!received) {
        Serial.println(F("Sending registration"));
        sendData(loraPort, GATEWAY_ADDRESS, arrLength, dataArr);
        
        uint8_t ack = ackWait(loraPort, GATEWAY_ADDRESS, 7500);
        
        if (ack == 0) {
            received = true;
            Serial.println(F("Received ack"));
        } else {
            Serial.println(F("Didn't receive ack"));
        }
    }
    
    free(dataArr);
}


/*-----------------Data Save Function------------------*/

//Save data to EEPROM
void saveData(uint32_t* data) {
    int address = 2;
    EEPROM.get(0, address);
    
    //Checks for exceeding EEPROM length
    //Don't really like the way I did it, but it should never happen anyway (hopefully)
    if (!((address + ((NUMBER_SENSORS + 1) * 4) - 1) >= EEPROM.length())) {
        
        for (uint8_t i = 0; i < (NUMBER_SENSORS + 1); i++) {
            EEPROM.put(address, data[i]);
            address += 4;
        }
        
        if (!(address >= EEPROM.length())) {
            EEPROM.put(0, address);
        }
    }
    free(data);
}


/*-----------------Sensor Read Function------------------*/

//Read data from all sensors based on their types
uint32_t* readSensors() {
    Serial.println(F("\nReading Sensors"));
    uint32_t* ans= malloc(sizeof(uint32_t) * (NUMBER_SENSORS + 1));
    ans[0] = millis();
    for (uint8_t i = 0; i < NUMBER_SENSORS; i++) {
        uint32_t data;
        if (strcmp(sensorTypes[i], "Dissolved_Oxygen") == 0) { //DO
            float tmp = readDO(i);
            memcpy(&data, &tmp, sizeof(uint32_t));
        } else if (strcmp(sensorTypes[i], "Conductivity") == 0) { //EC
            float tmp = readEC(i);
            memcpy(&data, &tmp, sizeof(uint32_t));
        } else if (strcmp(sensorTypes[i], "Turbidity") == 0) { //TB
            float tmp = readTB(i);
            memcpy(&data, &tmp, sizeof(uint32_t));
        } else {
            data = -1;
        }
        memcpy(&ans[i+1], &data, sizeof(uint32_t));
    }
    return ans;
}

//Read data from an Atlas Scientific dissolved oxygen sensor at index i
float readDO(uint8_t i) {
    SoftwareSerial sensor(sensorPorts[i][0], sensorPorts[i][1]);
    sensor.begin(9600);
    
    //Wake the sensor up
    sensor.print(F("R\r"));
    
    char buf[16];
    do {
        delay(1000);
        sensor.print(F("R\r"));
        while (sensor.available() <= 0);
        delay(100);
        
        int dataIndex = sensor.readBytesUntil('\r', buf, 16);
        buf[dataIndex] = 0;
    } while ((buf[0]<48) || (buf[0]>57));
    
    float data = atof(buf);
    
    Serial.print(F("Read DO sensor: "));
    Serial.println(data);
    
    //Put sensor to sleep
    sensor.print(F("Sleep\r"));
    
    sensor.end();
    
    return data;
}

//Read data from an Atlas Scientific conductivity sensor at index i
float readEC(uint8_t i) {
    SoftwareSerial sensor(sensorPorts[i][0], sensorPorts[i][1]);
    sensor.begin(9600);
    
    //Wake the sensor up
    sensor.print(F("R\r"));
    
    float data = 0.0;
    
    char buf[40];
    while (!data) { //Makes sure to get a non-zero number, for some reason the EC sensors like to read as 0.00 sometimes
        do {
            delay(1000);
            sensor.print(F("R\r"));
            while (sensor.available() <= 0);
            delay(100);

            int dataIndex = sensor.readBytesUntil('\r', buf, 40);
            buf[dataIndex] = 0;
        } while ((buf[0]<48) || (buf[0]>57));

        data = atof(buf);
    }
    
    Serial.print(F("Read EC sensor: "));
    Serial.println(data);
    
    //Put sensor to sleep
    sensor.print(F("Sleep\r"));
    
    sensor.end();
    
    return data;
}

//Read data from a DFRobot turbidity sensor at index i
float readTB(uint8_t i) {
    digitalWrite(8, HIGH);
    delay(100);
    
    float data = analogRead(sensorPorts[i][0]);
    
    digitalWrite(8, LOW);
    
    Serial.print(F("Read TB sensor: "));
    Serial.println(data);
    
    return data;
}


/*---------------Send Sensor Data Function----------------*/

//Sends saved data from EEPROM through LoRa
bool sendSensorData() {
    int memAddress = 2;
    EEPROM.get(0, memAddress);
    
    int numBytes = memAddress - 2;
    
    if (numBytes == 0) {
        return true;
    }
    
    if (numBytes > 111) {
        numBytes = 111;
    }
    
    numBytes -= numBytes % (4 * (NUMBER_SENSORS + 1));
    
    uint8_t* dataArr = malloc(sizeof(uint8_t) * (numBytes + 1));
    
    dataArr[0] = 0b00010000 | ((uint8_t) NUMBER_SENSORS & 0b00001111);
    
    uint8_t curr = 1;
    for (int i = memAddress - 4 * (NUMBER_SENSORS + 1); i >= (memAddress - numBytes); i -= 2 * 4 * (NUMBER_SENSORS + 1)) {
        for (uint8_t j = 0; j < NUMBER_SENSORS + 1; j++) {
            uint32_t currNum;
            EEPROM.get(i, currNum);
            memcpy(&dataArr[curr], &currNum, sizeof(uint8_t) * 4);
            curr += 4;
            i += 4;
        }
    }
    
    Serial.println(F("Sending:"));
    
    sendData(loraPort, GATEWAY_ADDRESS, numBytes + 1, dataArr);
    
    free(dataArr);

    uint8_t ack = ackWait(loraPort, GATEWAY_ADDRESS, 5000);

    if (ack == 0) {
        EEPROM.put(0, memAddress - numBytes);
        return true;
    }
    
    return false;
}


/*-------------------Helper Functions-------------------*/

//Wait time for acknoledgement from address, returns first byte of acknoledgement
uint8_t ackWait(Stream& port, uint16_t address, unsigned long time) {
    unsigned long beforeAck = millis();
    uint8_t ans = 0xFF;
    
    unsigned long currTime = 0;
    do {
        if (port.available()) {
            uint8_t* data = readData(port);
            uint16_t dataAdd;
            uint8_t dataAns;
            
            memcpy(&dataAdd, &data[1], sizeof(uint8_t) * 2);
            memcpy(&dataAns, &data[3], sizeof(uint8_t));
            free(data);
            if (dataAdd == address) { //Check that acknoledgement is from address
                ans = dataAns;
                break;
            }
        } else {
            if (millis() < beforeAck) {
                currTime = millis() + (4294967295 - beforeAck);
            } else {
                currTime = millis() - beforeAck;
            }
        }
    } while (currTime < time);
    
    return ans;
}

//Delay for an amount of time but accounts for start time and overflow
void delayOverflow(unsigned long time, unsigned long start) {
    if (millis() < start) { //Overflow check
        delay(time - (millis() + (4294967295-start)));
    } else {
        delay(time - (millis() - start));
    }
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
//These were all written by Spencer Park

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