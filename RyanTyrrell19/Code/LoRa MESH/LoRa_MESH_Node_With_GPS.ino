/*
 Name:		LoRa_MESH_Node_With_GPS.ino
 Created:	5/8/2019 12:19:02 PM
 Author:	tyrre
*/

/*-------------------------NOTES-----------------------------*/
/*
Latitude Value can range from -90 to 90, with up to 6 decimal places possible
Longitude Value can range from -180 to 180, with up to 6 decimal places possible
For simplicity, it will be assuemd that Latitude may be the same length as Longitude

*/



// Temporarly using hard-coded values for gps & sleep time
// Arduino can only has 6-7 decimal digits of precision 
// You have to hold down RST on LoRa+GPS Shields when uploading or you will get init failed

/*-------------------------INCLUDES-----------------------------*/

#include <SPI.h>

#include <SoftwareSerial.h>
#include <TinyGPS++.h>      
#include <LowPower.h>       
#include <Wire.h>

/*----------------------GPS DEFINITIONS-----------------------------*/
#define transistorGPS 10							// D10 Pin responsibile for supplying power to the transistor connected to the GPS

/*-------------------------GPS GLOBALS------------------------------*/

static const int stay_on = 1;						/*amount of time gps is active in minutes*/
static const int sleep_time = 7;					/*amount of time logger sleeps between readings in minutes*/

//uint32_t feedDuration = stay_on * 60000;			//time spent getting GPS fix
//uint32_t sleepTime = (sleep_time * 60) / 8;		//time sleeping (Will always round down if a deciaml)

uint32_t feedDuration = 2000;						//time spent getting GPS fix (10 seconds)
uint32_t sleepTime = 1;								//time spent getting GPS fix (8 seconds)

static const int RXPin = 9, TXPin = 3;				// Pinds responsible for reading from GPS; D9 Pin for receieving, D3 for transmitting (unused)
static const uint32_t GPSBaud = 9600;				// Buad rate for Reading from GPS via SoftwareSerial

uint32_t GPS_run;									// Tracks time that GPS has been running for

TinyGPSPlus gps;									// Object for accessing GPS Library
SoftwareSerial ss(RXPin, TXPin);					// SoftwareSerial Object for reading from GPS


/*---------------------LORA CONSTRUCTORS------------------------*/

#define MCU_LORA_RX 7								// Connects to LoRa TX, also is D7
#define MCU_LORA_TX 8								// Conencts to LoRa RX, also is D8

SoftwareSerial loraPort(MCU_LORA_RX, MCU_LORA_TX);	// SoftwareSerial Object for reading/writing to LoRa Module

uint16_t gatewayId = 0x1234;


/*---------------------------SETUP------------------------------*/

void setup() {
	while (!Serial);

	pinMode(MCU_LORA_RX, INPUT);
	pinMode(MCU_LORA_TX, OUTPUT);
	pinMode(transistorGPS, OUTPUT);
	digitalWrite(transistorGPS, LOW);

	loraPort.begin(9600);
	Serial.begin(9600);
	Wire.begin();
	ss.begin(GPSBaud);
	delay(1000);

	loraPort.listen();
	Serial.println(F("Configuration Setup:"));		// F has it store string in memory rather than RAM
	Serial.println();

	//writeConfig(loraPort, 0x0000, 0x0003);		// Edit the Nodes Network ID and Node ID
	readConfig(loraPort);							
	delay(1000);

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(250);
	digitalWrite(LED_BUILTIN, LOW);
	delay(250);
}

void loop() {

	Serial.println("Running GPS");
	ss.listen();
	digitalWrite(transistorGPS, HIGH);
	delay(500);
	GPS_run = millis();

	while (millis() - GPS_run < feedDuration) {  feedGPS();  }

	delay(100);
	digitalWrite(transistorGPS, LOW);
	delay(100);

	digitalWrite(LED_BUILTIN, HIGH);
	delay(250);
	digitalWrite(LED_BUILTIN, LOW);
	delay(250);

	Serial.print("GPS Data:    ");
	Serial.print(gps.location.lat(), 6); Serial.print("\t");
	Serial.print(gps.location.lng(), 6); Serial.print("\t  ");
	Serial.print(gps.time.hour()); Serial.print("/");
	Serial.print(gps.time.minute());Serial.println(" \t\t");
	delay(100);

	Serial.println("Sending Data");
	loraPort.listen();
	prepareData();
	Serial.println("Data Sent");
	/*
	Serial.println("Going to Sleep");
	Serial.println();
	delay(1000);

	for (int i = 0; i < sleepTime; i++) {
		LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
	}
	Serial.println("Waking Up");
	delay(500);
	*/
}

bool feedGPS() {
	while (ss.available()) {			 //Checks if data is available on those pins
		if (gps.encode(ss.read()))		 //Repeatedly feed it characters from your GPS device:
			return true;
	}
	return false;
}
/*
The maximum length of Longitude is 10 digits (- sign, 3 whole number, 6 decimals)
Latitude will be treated the same

When transmitting data, they must be broken into 2-byte packets.
1 Packet is used for the sign, the rest store the data
Therefore, the data can be broken down into 5 2-byte packets, plus 1packet for the sign
Datalength = 6

*/

void prepareData() {
	long lat, lng;
	uint8_t payloadLen = 6;
	uint8_t* payload = (uint8_t*)malloc(sizeof(uint8_t) * payloadLen);
	uint8_t* payload1 = (uint8_t*)malloc(sizeof(uint8_t) * payloadLen);

	lat = random(-90000000, 90000000);  //gps.location.lat();
	lng = random(-180000000, 180000000);  //gps.location.lng();

	Serial.print("GPS Data:    ");
	printL(Serial, lat); // prints int like a float
	Serial.print("\t");
	printL(Serial, lng); // prints int like a float
	Serial.println();

	doubleToHex(lat, payload);
	doubleToHex(lng, payload1);

	memcpy(payload + (sizeof(uint8_t) * payloadLen), payload1, payloadLen);
	/*
		Serial.println();

		for (int i = 0; i < payloadLen*2; i++) {
			Serial.print(payload[i]);
		}
		Serial.println();*/

	sendData(loraPort, gatewayId, (payloadLen * 2), payload);

	free(payload);
	free(payload1);
}
/*
Conversion process:
1) Convert data to a long value. Data will always have 6 decimal places, so multiply by 10^6
2) check sign of value and emove -ve sign if it exists
	Set the value of the birst Byte of data packet accordingly
			- = 0x01
			+ = 0x00
3) Break the data into 2-byte values and add the data (in order) to the payload array 
*/
void doubleToHex(long data, uint8_t* payload) {

	int i;
	//long data;
	uint8_t digit, digit1, digit2;

	// Will always be 6 decimals places so multiple by 10^6 to make a whole number
	// loss of precision on last decimal point occurs from this multiplications
	//data = (long)(a * pow(10, 6));

/*
	Serial.println();
	Serial.println(a, 6);
	Serial.println(pow(10, 6));
	Serial.println(data);*/

	//  Check if + or -, add to data packet, and remove - if it exists
	if (data < 0) {
		payload[0] = 0x01;
		data *= -1;
	}
	else
		payload[0] = 0x00;

	// Split the whole number into 2-digit numbers, convert to HEX, and add to payload
	// for loop done this way so that, in the end, the ordering of the numbers are correct
	for (i = 5; i >= 1; i--) {
		digit2 = data % 10;
		data /= 10;
		digit1 = data % 10;
		data /= 10;

		// for cases where the total # of digits is odd, to prevent the ones-digit from being multiplied by 10
		if (digit2 == 0 && i == 1)
			digit = digit1;
		else
			digit = (digit1 * 10) + digit2;

	//	Serial.println(digit);
		payload[i] = digit;
	}

	/*
		for (i = 0; i < 6; i++) {
			Serial.println(payload[i]);
		}*/
}


/*
  Print the 32-bit integer degrees *as if* they were high-precision floats
*/
static void printL(Print& outs, int32_t degE7)
{
	// Extract and print negative sign
	if (degE7 < 0) {
		degE7 = -degE7;
		outs.print('-');
	}

	// Whole degrees
	int32_t deg = degE7 / 1000000L;
	outs.print(deg);
	outs.print('.');

	// Get fractional degrees
	degE7 -= deg * 1000000L;

	// Print leading zeroes, if needed
	int32_t factor = 100000L;
	while ((degE7 < factor) && (factor > 1L)) {
		outs.print('0');
		factor /= 10L;
	}

	// Print fractional degrees
	outs.print(degE7);
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
	Serial.println("Payload:");
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
		Serial.print("targetAddr ="); printShort(targetAddr); Serial.println();
		Serial.print("status ="); printByte(status); Serial.println();
	//	Serial.println();
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
	Serial.print("srcAddr = "); printShort(srcAddr); Serial.println();
	Serial.print("power = "); printByte(power); Serial.println();
	Serial.print("userPayloadLength = "); printByte(userPayloadLength); Serial.println();
	for (int i = 0; i < userPayloadLength; i++)
		printByte(payload[4 + i]);

	Serial.println();

	free(payload);
}

// Max len is 111 bytes
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
		Serial.print("status="); printByte(status);
		Serial.println();
	}
	free(responsePayload);
}


/*
NodeID is defined globally so it can be used in data transmissions

*/
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