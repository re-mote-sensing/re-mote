/*
 Name:		MESH_and_Antenna_Test_Node.ino
 Created:	6/11/2019 4:03:43 PM
 Author:	tyrre
*/

/*--------------------------------NOTES--------------------------------------*/

// GPS uses Hardware Serial
// LoRa use Alternate Hardware Serial
// Debugging is done via Software Serial and viewed using Putty
/*
* Memory chip works in pages, where each page is made up of 64 bytes
* Page 1: 0 - 63
* Page 2 : 64 - 127, etc
*
* If config is saved half on one page, half on the other, it doesn't save properly and beomes corrupt
* Therefore, have address increment by 16 so that config is not saved on 2 seperate pages
* Also, when a write goes bad, it tends to corrupt address 0 on the memory chip.This can corrupt the entire first config
* Therefore, start at address 16 to prevent this
*/
/*---------------------------------INCLUDES----------------------------------*/

#include <SPI.h>
#include <SoftwareSerial.h>
#include <LowPower.h>       
#include <Wire.h>
#include <AltSoftSerial.h>
#include <NMEAGPS.h>
#include <EEPROMEx.h>

/*--------------------------CHECK GPS CONFIGURATION---------------------------*/

#if !defined(GPS_FIX_TIME) | !defined(GPS_FIX_DATE)
#error You must define GPS_FIX_TIME and DATE in GPSfix_cfg.h!
#endif

#if !defined(NMEAGPS_PARSE_RMC) & !defined(NMEAGPS_PARSE_ZDA)
#error You must define NMEAGPS_PARSE_RMC or ZDA in NMEAGPS_cfg.h!
#endif

//------------------------------------------------------------
// Check that the config files are set up properly

#if !defined( GPS_FIX_LOCATION )
#error You must uncomment GPS_FIX_LOCATION in GPSfix_cfg.h!
#endif

#if !defined( GPS_FIX_SPEED )
#error You must uncomment GPS_FIX_SPEED in GPSfix_cfg.h!
#endif

#if !defined( GPS_FIX_SATELLITES )
#error You must uncomment GPS_FIX_SATELLITES in GPSfix_cfg.h!
#endif

#ifdef NMEAGPS_INTERRUPT_PROCESSING
#error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
#endif


/*-------------------------GPS TIMEZONE SETUP-------------------------------------*/

// Set these values to the offset of your timezone from GMT

static const int32_t          zone_hours = -5L; // EDT
static const int32_t          zone_minutes = 0L; // usually zero
static const NeoGPS::clock_t  zone_offset =
zone_hours * NeoGPS::SECONDS_PER_HOUR +
zone_minutes * NeoGPS::SECONDS_PER_MINUTE;

// Uncomment one DST changeover rule, or define your own:
#define USA_DST
//#define EU_DST

#if defined(USA_DST)
static const uint8_t springMonth = 3;
static const uint8_t springDate = 14; // latest 2nd Sunday
static const uint8_t springHour = 2;
static const uint8_t fallMonth = 11;
static const uint8_t fallDate = 7; // latest 1st Sunday
static const uint8_t fallHour = 2;
#define CALCULATE_DST

#elif defined(EU_DST)
static const uint8_t springMonth = 3;
static const uint8_t springDate = 31; // latest last Sunday
static const uint8_t springHour = 1;
static const uint8_t fallMonth = 10;
static const uint8_t fallDate = 31; // latest last Sunday
static const uint8_t fallHour = 1;
#define CALCULATE_DST
#endif


/*-------------------------------SERIAL SETUP-----------------------------------*/
// Software Serial baud rate must be very large
// Hardware Serial cannot be larger than Software Serial
// AltSoftwaSerial must be 10x smaller than Software Serial

SoftwareSerial DEBUG_PORT(2, 3);
#define PC_BAUDRATE 115000

#define gpsPort Serial
#define GPS_BAUDRATE 9600L

AltSoftSerial loraPort;
#define LORA_BAUDRATE 9600L

#define GPS_PORT_NAME "Serial"

/*------------------------------GPS GLOBALS------------------------------*/

uint32_t feedDuration = 10000;						//time spent getting GPS fix
uint32_t GPS_run;									// Tracks time that GPS has been running for

static NMEAGPS  gps; // This parses received characters
static gps_fix  fix; // This contains all the parsed pieces

//#include <GPSport.h>

//AltSoftSerial gpsPort; // 8 & 9 for an UNO
//#define GPS_PORT_NAME "AltSoftSerial"
//#define DEBUG_PORT Serial
//
//#include <AltSoftSerial.h>
//#define gpsPort Serial;
//AltSoftSerial DEBUG_PORT; // 8 & 9 for an UNO
//


/*---------------------------LORA SETUP---------------------------------*/

uint16_t gatewayId = 0x4444;
uint8_t rssi = 0;
uint8_t dataSent;

/*-----------------------------DEBUGGING SETUP---------------------------------*/
//#define PC_RX 2								// Connects to LoRa TX, also is D7
//#define PC_TX 3								// Conencts to LoRa RX, also is D8
//SoftwareSerial ss(PC_RX, PC_TX);


/*---------------------------------EEPROM-------------------------------------*/

unsigned short address;     //number range is 0 - 65,535


/*---------------------------EXTRA LORA STUFF--------------------------------*/


void printByte(uint8_t b) {
	DEBUG_PORT.print(" 0x");
	if (b <= 0xF)
		DEBUG_PORT.print("0");
	DEBUG_PORT.print(b, HEX);
}

void printShort(uint16_t s) {
	DEBUG_PORT.print(" 0x");
	if (s <= 0xFFF)
		DEBUG_PORT.print("0");
	if (s <= 0xFF)
		DEBUG_PORT.print("0");
	if (s <= 0xF)
		DEBUG_PORT.print("0");
	DEBUG_PORT.print(s, HEX);
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

	DEBUG_PORT.println();
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
	DEBUG_PORT.println("Payload:");
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
		DEBUG_PORT.print("targetAddr ="); printShort(targetAddr); DEBUG_PORT.println();
		DEBUG_PORT.print("status ="); printByte(status); DEBUG_PORT.println();
		//	DEBUG_PORT.println();

		dataSent = status;
	}
	free(responsePayload);
}

void readData(Stream & port) {
	uint8_t frameType = 0;
	uint8_t cmdType = 0;
	uint8_t* payload = NULL;
	int len = readFrame(port, &frameType, &cmdType, &payload);

	if (frameType != 0x05 || cmdType != 0x82) {
		DEBUG_PORT.println("BAD TYPE");
		free(payload);
		return;
	}

	uint16_t srcAddr = (payload[0] << 8) | payload[1];
	uint8_t power = payload[2];
	uint8_t userPayloadLength = payload[3];
	DEBUG_PORT.print("srcAddr = "); printShort(srcAddr); DEBUG_PORT.println();
	DEBUG_PORT.print("power = "); printByte(power); DEBUG_PORT.println();
	DEBUG_PORT.print("userPayloadLength = "); printByte(userPayloadLength); DEBUG_PORT.println();
	for (int i = 0; i < userPayloadLength; i++)
		printByte(payload[4 + i]);

	DEBUG_PORT.println();

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
		DEBUG_PORT.print("status="); printByte(status);
		DEBUG_PORT.println();
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
		DEBUG_PORT.println("ERROR");
		free(payload);
		return;
	}
	DEBUG_PORT.println();
	DEBUG_PORT.print("FrameType ="); printByte(frameType); DEBUG_PORT.println();
	DEBUG_PORT.print("CmdType ="); printByte(cmdType);  DEBUG_PORT.println();

	if (frameType != 0x01 || cmdType != 0x82) {
		DEBUG_PORT.println("BAD TYPE");
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
	// 10, 11, 12 reservedhyj
	uint8_t serPortParam = payload[13];
	uint16_t airRate = (payload[14] << 8) | payload[15];

	DEBUG_PORT.print("flag ="); printShort(flag); DEBUG_PORT.println();
	DEBUG_PORT.print("channel ="); printByte(channel); DEBUG_PORT.println();
	DEBUG_PORT.print("txPower ="); printByte(txPower); DEBUG_PORT.println();
	DEBUG_PORT.print("uiMode ="); printByte(uiMode); DEBUG_PORT.println();
	DEBUG_PORT.print("eqType ="); printByte(eqType); DEBUG_PORT.println();
	DEBUG_PORT.print("netId ="); printShort(netId); DEBUG_PORT.println();
	DEBUG_PORT.print("nodeId ="); printShort(nodeId); DEBUG_PORT.println();
	DEBUG_PORT.print("serPortParam ="); printByte(serPortParam); DEBUG_PORT.println();
	DEBUG_PORT.print("airRate ="); printShort(airRate); DEBUG_PORT.println();
	DEBUG_PORT.println();
}



/*-----------------------------------EXTRA GPS STUFF-------------------------------*/

void adjustTime(NeoGPS::time_t& dt)
{
	NeoGPS::clock_t seconds = dt; // convert date/time structure to seconds

#ifdef CALCULATE_DST
  //  Calculate DST changeover times once per reset and year!
	static NeoGPS::time_t  changeover;
	static NeoGPS::clock_t springForward, fallBack;

	if ((springForward == 0) || (changeover.year != dt.year)) {

		//  Calculate the spring changeover time (seconds)
		changeover.year = dt.year;
		changeover.month = springMonth;
		changeover.date = springDate;
		changeover.hours = springHour;
		changeover.minutes = 0;
		changeover.seconds = 0;
		changeover.set_day();
		// Step back to a Sunday, if day != SUNDAY
		changeover.date -= (changeover.day - NeoGPS::time_t::SUNDAY);
		springForward = (NeoGPS::clock_t) changeover;

		//  Calculate the fall changeover time (seconds)
		changeover.month = fallMonth;
		changeover.date = fallDate;
		changeover.hours = fallHour - 1; // to account for the "apparent" DST +1
		changeover.set_day();
		// Step back to a Sunday, if day != SUNDAY
		changeover.date -= (changeover.day - NeoGPS::time_t::SUNDAY);
		fallBack = (NeoGPS::clock_t) changeover;
	}
#endif

	//  First, offset from UTC to the local timezone
	seconds += zone_offset;

#ifdef CALCULATE_DST
	//  Then add an hour if DST is in effect
	if ((springForward <= seconds) && (seconds < fallBack))
		seconds += NeoGPS::SECONDS_PER_HOUR;
#endif

	dt = seconds; // convert seconds back to a date/time structure

} // adjustTime


#ifdef NMEAGPS_INTERRUPT_PROCESSING
static void GPSisr(uint8_t c)
{
	gps.handle(c);
}
#endif


/*-----------------------------IMPORTANT LORA STUFF----------------------------*/

/*
The maximum length of Longitude is 10 digits (- sign, 3 whole number, 6 decimals)
Latitude will be treated the same

When transmitting data, they must be broken into 2-byte packets.
1 Packet is used for the sign, the rest store the data
Therefore, the data can be broken down into 5 2-byte packets, plus 1packet for the sign
Datalength = 6

*/

void prepareData() {
	long lat = 0, lng = 0;
	uint8_t payloadLen = 6;
	uint8_t* payload = (uint8_t*)malloc(sizeof(uint8_t) * payloadLen);
	uint8_t* payload1 = (uint8_t*)malloc(sizeof(uint8_t) * payloadLen);

	if (fix.valid.location) {
		lat = fix.latitudeL();
		lng = fix.longitudeL();
	}
	doubleToHex(lat, payload);
	doubleToHex(lng, payload1);

	memcpy(payload + (sizeof(uint8_t) * payloadLen), payload1, payloadLen);
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

		payload[i] = digit;
	}
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
	int32_t deg = degE7 / 10000000L;
	outs.print(deg);
	outs.print('.');

	// Get fractional degrees
	degE7 -= deg * 10000000L;

	// Print leading zeroes, if needed
	int32_t factor = 1000000L;
	while ((degE7 < factor) && (factor > 1L)) {
		outs.print('0');
		factor /= 10L;
	}

	// Print fractional degrees
	outs.print(degE7);
}



/*-----------------------------------IMPORTANT GPS STUFF-------------------------------*/
static void doSomeWork()
{
	//  This is the best place to do your time-consuming work, right after
	//     the RMC sentence was received.  If you do anything in "loop()",
	//     you could cause GPS characters to be lost, and you will not
	//     get a good lat/lon.
	//  For this example, we just print the lat/lon.  If you print too much,
	//     this routine will not get back to "loop()" in time to process
	//     the next set of GPS data.

	DEBUG_PORT.print("Time:   ");
	DEBUG_PORT.print(fix.valid.time);
	DEBUG_PORT.print("   ,   ");
	adjustTime(fix.dateTime);
	DEBUG_PORT << fix.dateTime;
	DEBUG_PORT.println();

	DEBUG_PORT.print("Lat/Lon:    ");
	DEBUG_PORT.print(fix.valid.location);
	DEBUG_PORT.print("   ,   ");
	// DEBUG_PORT.print( fix.latitude(), 6 ); // floating-point display
	// DEBUG_PORT.print( fix.latitudeL() ); // integer display
	printL(DEBUG_PORT, fix.latitudeL()); // prints int like a float
	DEBUG_PORT.print(',');
	// DEBUG_PORT.print( fix.longitude(), 6 ); // floating-point display
	// DEBUG_PORT.print( fix.longitudeL() );  // integer display
	printL(DEBUG_PORT, fix.longitudeL()); // prints int like a float
	DEBUG_PORT.println();

	DEBUG_PORT.print("# SVs Used:     ");
	DEBUG_PORT.print(fix.valid.satellites);
	DEBUG_PORT.print("   ,   ");
	DEBUG_PORT.print(fix.satellites);


	DEBUG_PORT.println();
	DEBUG_PORT.println();


} // doSomeWork

static void GPSloop()
{
	while (gps.available(gpsPort)) {
		fix = gps.read();
	}
} // GPSloop

/*--------------------------------------EEPROM STUFF-------------------------------------*/

// Arduino has 1024 bytes of memory on EEPROM
void WriteEE() {
	DEBUG_PORT.println(rssi);
	address = EEPROM.getAddress(sizeof(byte));
	EEPROM.writeByte(address, rssi);
	DEBUG_PORT.println(EEPROM.readByte(address));

	DEBUG_PORT.println(fix.latitudeL());
	address = EEPROM.getAddress(sizeof(long));
	EEPROM.writeLong(address, fix.latitudeL());
	DEBUG_PORT.println(EEPROM.readLong(address));

	DEBUG_PORT.println(fix.longitudeL() * -1L);
	address = EEPROM.getAddress(sizeof(long));
	EEPROM.writeLong(address, (fix.longitudeL() * -1L));
	DEBUG_PORT.println(EEPROM.readLong(address));

	delay(100);

}

/*-----------------------------------------SETUP AND LOOP--------------------------------*/

void setup() {
	delay(3000);

	//Initialize Serial
	pinMode(2, INPUT);
	pinMode(3, OUTPUT);

	loraPort.begin(LORA_BAUDRATE);
	gpsPort.begin(GPS_BAUDRATE);
	DEBUG_PORT.begin(PC_BAUDRATE);
//	Wire.begin();
	delay(1000);

	//EEPROM Setup
	DEBUG_PORT.println("Clearing Memory");		
	for (int i = 0; i < 1024; i++) {
		EEPROM.writeByte(i, 0);
	}
	DEBUG_PORT.println("Memory Cleared");

	// LoRa Setup Stuff
	DEBUG_PORT.println(F("Configuration Setup:"));		// F has it store string in memory rather than RAM
	DEBUG_PORT.println();
	////writeConfig(loraPort, 0x0000, 0x0003);		// Edit the Nodes Network ID and Node ID
	readConfig(loraPort);

	//GPS Setup Data
	DEBUG_PORT.print(F("NMEAtimezone.INO: started\n"));
	DEBUG_PORT.println(F("Looking for GPS device on " GPS_PORT_NAME));
	DEBUG_PORT.println(F("Local time"));

	DEBUG_PORT.print(F("NMEAloc.INO: started\n"));
	DEBUG_PORT.print(F("fix object size = "));
	DEBUG_PORT.println(sizeof(gps.fix()));
	DEBUG_PORT.print(F("NMEAGPS object size = "));
	DEBUG_PORT.println(sizeof(gps));
	DEBUG_PORT.println(F("Looking for GPS device on " GPS_PORT_NAME));


#ifdef NMEAGPS_NO_MERGING
	DEBUG_PORT.println(F("Only displaying data from xxRMC sentences.\n  Other sentences may be parsed, but their data will not be displayed."));
#endif

#ifdef NMEAGPS_INTERRUPT_PROCESSING
	gpsPort.attachInterrupt(GPSisr);
#endif

	DEBUG_PORT.flush();

}


void loop() {

	DEBUG_PORT.println("Running GPS");
	GPS_run = millis();

	while (millis() - GPS_run < feedDuration) { GPSloop(); }

	DEBUG_PORT.println("GPS Data:    ");
	doSomeWork();

	DEBUG_PORT.println("Sending Data");
	prepareData();
	DEBUG_PORT.println("Data Sent");

	delay(500);

	if (fix.valid.location && dataSent != 0) {
		WriteEE();
		DEBUG_PORT.println("Data Written");
	}
	else {
		DEBUG_PORT.println("Data NOT Written");
	}

	delay(100);

}