/*
 Name:		MESH_and_Antenna_Test_Gateway.ino
 Created:	6/11/2019 4:04:12 PM
 Author:	tyrre
*/

/*-------------------------Notes-----------------------------*/

// LoRa uses Alternate Software Serial
// Debugging done using Hardware Serial

/*----------------------INCLUDES/DEFINES----------------------*/
#include <AltSoftSerial.h>
#include <EEPROMEx.h>


#define PC_BAUDRATE 9600L
#define LORA_BAUDRATE 9600L

/*---------------------LORA CONSTRUCTORS------------------------*/

AltSoftSerial loraPort;
String latS, lngS;
long lat, lng;


/*---------------------------SETUP------------------------------*/

boolean dataReceived = false;
int nodeId;
uint8_t rssi;
int address;

void setup() {

	loraPort.begin(LORA_BAUDRATE);
	Serial.begin(PC_BAUDRATE);

	//EEPROM Setup
	Serial.println("Clearing Memory");
	for (int i = 0; i < 1024; i++) {
		EEPROM.writeByte(i, 0);
	}
	Serial.println("Memory Cleared");


	delay(1000);
	Serial.println(F("Configuration Setup:"));		// F has it store string in memory rather than RAM
	Serial.println();

	//writeConfig(loraPort, 0x0000, 0x4444);		// Edit the Nodes Network ID and Node ID
	readConfig(loraPort);

	delay(1000);
	pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {

	Serial.println("Waiting for Data...");
	readData(loraPort);

	while (!dataReceived);

	dataReceived = false;

	Serial.println("--------------------------------------");
	Serial.print("Recieved Data from Node #: ");
	Serial.println(nodeId);
	Serial.print("GPS coordinates:  ");

	Serial.print(latS);
	Serial.print(" , ");
	Serial.print(lngS);
	Serial.println();

	Serial.println("--------------------------------------");

	WriteEE();
	delay(500);

	Serial.println();
	Serial.println();
	delay(10);
}


// Arduino has 1024 bytes of memory on EEPROM
void WriteEE() {
	Serial.println(rssi);
	address = EEPROM.getAddress(sizeof(byte));
	EEPROM.writeByte(address, rssi);
	Serial.println(EEPROM.readByte(address));

	Serial.println(lat);
	address = EEPROM.getAddress(sizeof(long));
	EEPROM.writeLong(address, lat);
	Serial.println(EEPROM.readLong(address));

	Serial.println(lng * -1L);
	address = EEPROM.getAddress(sizeof(long));
	EEPROM.writeLong(address, (lng * -1L));
	Serial.println(EEPROM.readLong(address));

	delay(100);

}

long HexToDouble(uint8_t* payload, uint8_t payloadLength) {
	long data = 0;
	char s1[20], s2[20];

	delay(1000);
	data += (payload[1] * 100000000L);
	data += (payload[2] * 1000000L);
	data += (payload[3] * 10000L);
	data += (payload[4] * 100L);
	data += (payload[5] * 1L);

	if (payload[0] == 0x01)		// data was a negative
		data *= -1;

	return data;	
}

/*
  Print the 32-bit integer degrees *as if* they were high-precision floats
*/
static String dataToString(int32_t degE7)
{
	String data;

	// Extract and print negative sign
	if (degE7 < 0) {
		degE7 = -degE7;
		data.concat('-');
	}

	// Whole degrees
	int32_t deg = degE7 / 10000000L;
	data.concat(deg);
	data.concat('.');

	// Get fractional degrees
	degE7 -= deg * 10000000L;

	// Print leading zeroes, if needed
	int32_t factor = 1000000L;
	while ((degE7 < factor) && (factor > 1L)) {
		data.concat('0');
		factor /= 10L;
	}
	// Print fractional degrees
	data.concat(degE7);

	return data;
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

// Commands

uint8_t readByte(Stream & port) {
	while (!port.available());
	return port.read();
}

int readFrame(Stream & port, byte * rFrameType, byte * rCmdType, byte * *rPayload) {
	uint8_t checksum = 0;

	uint8_t frameType = readByte(port);
	uint8_t frameNum = readByte(port);
	uint8_t cmdType = readByte(port);
	uint8_t payloadLen = readByte(port);

	checksum ^= frameType;
	checksum ^= frameNum;
	checksum ^= cmdType;
	checksum ^= payloadLen;

	uint8_t* payload = (uint8_t*)malloc(sizeof(uint8_t) * payloadLen);
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

void writeFrame(Stream & port, uint8_t frameType, uint8_t cmdType, uint8_t payloadLen, uint8_t * payload) {
	uint8_t checksum = 0;

	checksum ^= frameType;
	checksum ^= 0; // frameNum which is unused and always 0
	checksum ^= cmdType;
	checksum ^= payloadLen;

	printByte(frameType);
	printByte((uint8_t)0); // frameNum
	printByte(cmdType);
	printByte(payloadLen);

	port.write(frameType);
	port.write((uint8_t)0); // frameNum
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

// Max len is 111 bytes
void sendData(Stream & port, uint16_t target, uint8_t dataLen, uint8_t * data) {

	// We add predataLength bytes to the head of data for this payload
	uint8_t predataLength = 6;
	uint8_t payloadLen = predataLength + dataLen;
	uint8_t* payload = (uint8_t*)malloc(sizeof(uint8_t) * payloadLen);

	// target address as big endian short
	payload[0] = (uint8_t)((target >> 8) & 0xFF);
	payload[1] = (uint8_t)(target & 0xFF);

	// ACK request == 1 -> require acknowledgement of recv
	payload[2] = (uint8_t)0;//1;

	// Send radius: which defaults to max of 7 hops, we can use that
	payload[3] = (uint8_t)7;

	// Discovery routing params == 1 -> automatic routing
	payload[4] = (uint8_t)1;

	//// Source routing domain: unused when automatic routing enabled
	////    - number of relays is 0
	////    - relay list is therefor non-existent
	//payload[5] = (uint8_t)1;

	//// Relay List
	//// Address of Intermediate Node written as 2-byte short addresses (lower byte first)
	//uint16_t inter = 0x2048;
	//payload[6] = (uint8_t)((inter >> 8) & 0xFF);
	//payload[7] = (uint8_t)(inter & 0xFF);

	// Data length
	payload[5] = dataLen;

	// Data from index 'predataLength' to the end should be the data
	memcpy(payload + (sizeof(uint8_t) * predataLength), data, dataLen);

	// frameType = 0x05, cmdType = 0x01 for sendData
	writeFrame(port, 0x05, 0x01, payloadLen, payload);

	free(payload);

	uint8_t frameType = 0;
	uint8_t cmdType = 0;
	uint8_t * responsePayload = NULL;
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

void readData(Stream & port) {

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

	uint8_t * lat_payload = payload + 4;
	uint8_t * lng_payload = payload + 4 + (userPayloadLength / 2);

	Serial.print("srcAddr = "); printShort(srcAddr); Serial.println();
	Serial.print("power = "); printByte(power); Serial.println();
	Serial.print("userPayloadLength = "); printByte(userPayloadLength); Serial.println();
	Serial.println("Payload:");
	for (int i = 0; i < userPayloadLength; i++)
		printByte(payload[4 + i]);

	Serial.println();
	Serial.println();

	/*for (int i = 0; i < userPayloadLength; i++)
		Serial.print(payload[4 + i]);

	Serial.println();*/

	lat = HexToDouble(lat_payload, (userPayloadLength / 2));
	lng = HexToDouble(lng_payload, (userPayloadLength / 2));

	latS = dataToString(lat); // prints int like a float
	lngS = dataToString(lng); // prints int like a float

	nodeId = srcAddr;
	rssi = power;
	dataReceived = true;

	free(payload);
}

// Max len is 111 bytes
void writeConfig(Stream & port, uint16_t netID, uint16_t nodeID) {

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
		Serial.print("status="); printByte(status);
		Serial.println();
	}
	free(responsePayload);
}

void readConfig(Stream & port) {
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

	Serial.println();
	Serial.print("FrameType ="); printByte(frameType); Serial.println();
	Serial.print("CmdType ="); printByte(cmdType);  Serial.println();

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

	Serial.print("flag ="); printShort(flag); Serial.println();
	Serial.print("channel ="); printByte(channel); Serial.println();
	Serial.print("txPower ="); printByte(txPower); Serial.println();
	Serial.print("uiMode ="); printByte(uiMode); Serial.println();
	Serial.print("eqType ="); printByte(eqType); Serial.println();
	Serial.print("netId ="); printShort(netId); Serial.println();
	Serial.print("nodeId ="); printShort(nodeId); Serial.println();
	Serial.print("serPortParam ="); printByte(serPortParam); Serial.println();
	Serial.print("airRate ="); printShort(airRate); Serial.println();
	Serial.println();
}