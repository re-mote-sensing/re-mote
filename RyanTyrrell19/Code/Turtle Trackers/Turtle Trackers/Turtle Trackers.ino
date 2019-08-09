/*
 Name:		Turtle_Trackers.ino
 Created:	6/17/2019 2:35:01 PM
 Author:	tyrre
*/


/*----------------------------------QUESTIONS--------------------------------------*/


/*----------------------------------TODO---------------------------------------------*/

// Add support for different GPS Modules & GPS protocols
// Self-correcting function using RTC to continue running during correct time
// Algorithm that takes into account time spent underwater to determine allowable time to aquire GPS + aided data
// Check for when Underwater (use LoRa?)

/*-----------------------------------NOTES------------------------------------------*/



/*------------------------------INCLUDES---------------------------*/

#include <NeoSWSerial.h>
#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <LowPower.h>       
#include <NMEAGPS.h>

/*--------------------------------------HARDWARE CONFIURATION-------------------------------*/

#define RTC_AVAILABLE
#define EN_AVAILABLE
#define UBLOX_PROTOCOL_AVAILABLE

/*-----------------------------------UBLOX CONFIGURATION----------------------------------------*/
/*
Byte 1 & 2: Always 0xB5, 0x62
Byte 2: Always 0x62
Byte 3: Class Field
Byte 4: ID Field
Byte 5 & 6: Payload Length
*/

/*________Enable Messages__________*/
// Enable AID_INI (Initial Data)
uint8_t AID_INI[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x0B, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x07 };

//Enable MON-HW (Hardware Status)
uint8_t MON_HW[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x0A, 0x09, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x23, 0x37 };
//Enable MON-TXBUF (TX Buffer)
uint8_t MON_TXBUF[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x0A, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x22, 0x30 };

//Enable NAV-AOPSTATUS (AssistNow Autonomous Status)
uint8_t NAV_AOPSTATUS[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x60, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x71, 0x50 };
//Enable NAV-DGPS (DGPS Data)
uint8_t NAV_DGPS[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x31, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42, 0x07 };
// Enable NAV-DOP (Dilution of Precision)
uint8_t NAV_DOP[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x15, 0xCC };
// Enable NAV-SOL (Navigation Solution)
uint8_t NAV_SOL[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x17, 0xDA };
// Enable NAV-STATUS (Navigation Status)
uint8_t NAV_STATUS[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x14, 0xC5 };


/*________GPS Configurtion_________*/
//CFG-NAV5 - Configure NAV5
uint8_t CFG_NAV5[] = { 0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4F, 0x82 };

//CFG-NAVX5 - Configure NAVX5
uint8_t CFG_NAVX5[] = { 0xB5, 0x62, 0x06, 0x23, 0x28, 0x00, 0x00, 0x00, 0x4C, 0x66, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x16, 0x07, 0x00, 0x00, 0x00, 0x00, 0x01, 0x9B, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEA, 0xF7 };

// CFG-SBAS - Configure SBAS
uint8_t CFG_SBAS[] = { 0xB5, 0x62, 0x06, 0x16, 0x08, 0x00, 0x01, 0x03, 0x03, 0x00, 0x00, 0xE8, 0x04, 0x00, 0x17, 0x79 };

// CFG-CFG - Save Config. setings
uint8_t CFG_CFG[] = { 0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1D, 0xAB };

/*-------------------------------ARRAY SETUP OF UBX CONFIG PAYLAODS-------------------------*/
#define CONFIG_SIZE 12
uint8_t* config[CONFIG_SIZE] = { &AID_INI[0], &MON_HW[0], &MON_TXBUF[0], &NAV_AOPSTATUS[0], &NAV_DGPS[0], &NAV_DOP[0], &NAV_SOL[0], &NAV_STATUS[0], &CFG_NAV5[0], &CFG_NAVX5[0], &CFG_SBAS[0], &CFG_CFG[0] };
uint8_t config_size[CONFIG_SIZE] = { sizeof(AID_INI) / sizeof(uint8_t),
									sizeof(MON_HW) / sizeof(uint8_t),
									sizeof(MON_TXBUF) / sizeof(uint8_t),
									sizeof(NAV_AOPSTATUS) / sizeof(uint8_t),
									sizeof(NAV_DGPS) / sizeof(uint8_t),
									sizeof(NAV_DOP) / sizeof(uint8_t),
									sizeof(NAV_SOL) / sizeof(uint8_t),
									sizeof(NAV_STATUS) / sizeof(uint8_t),
									sizeof(CFG_NAV5) / sizeof(uint8_t),
									sizeof(CFG_NAVX5) / sizeof(uint8_t),
									sizeof(CFG_SBAS) / sizeof(uint8_t),
									sizeof(CFG_CFG) / sizeof(uint8_t) };

/*---------------------------------NEO GPS CONFIGURATION CHECK------------------------------*/

#if !defined(GPS_FIX_TIME) | !defined(GPS_FIX_DATE)
#error You must define GPS_FIX_TIME and DATE in GPSfix_cfg.h!
#endif

#if !defined(NMEAGPS_PARSE_RMC) & !defined(NMEAGPS_PARSE_ZDA)
#error You must define NMEAGPS_PARSE_RMC or ZDA in NMEAGPS_cfg.h!
#endif

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

/*______NEOGPS TIME ZONE SETUP_____*/

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

/*------------------------------------------SERIAL SETUP------------------------------------*/
// Software Serial baud rate must be very large
// Hardware Serial cannot be larger than Software Serial
// AltSoftwaSerial must be 10x smaller than Software Serial

#define GPS_PORT_NAME "gpsPort"
#define LORA_RX 2
#define LORA_TX 3

SoftwareSerial loraPort(LORA_RX, LORA_TX);
#define LORA_BAUDRATE 115000L

AltSoftSerial gpsPort;				//RX - 8, TX - 9
#define GPS_BAUDRATE 9600L

#define DEBUG_PORT Serial
#define PC_BAUDRATE 9600L

/*--------------------------------------------GPS SETUP-------------------------------------*/

#define GPS_ENABLE_PIN 12								// Pin used to power on/off GPS (Low = Disable, High = Enable)
#define GPS_VBAT_PIN 11									// Pin used to power to power the GPS to retain RTC and position data
#define	GPS_FIX 10000									// time spent getting GPS fix

static NMEAGPS  gps;									// This parses received characters from the GPS
static gps_fix  fix;									// This contains all the parsed pieces


/*------------------------------------------LOW POWER SETUP----------------------------------*/

#define SHORT_SLEEP 1									// Set the time the Arduino will sleep for during the short sleep interval, in minutes
#define LONG_SLEEP 1									// Set the time the Arduino will sleep for during the long sleep interval, in minutes


uint16_t short_sleep_time = SHORT_SLEEP * 60000;		// Time spent asleep during short interval, in minutes
uint16_t long_sleep_time = SHORT_SLEEP * 60000;			// Time spent asleep during long interval, in minutes


/*--------------------------------------------LORA SETUP-------------------------------------*/

#define NODE_ID 0x0001				// ID of the Turtle Tracker Node
#define NETWROK_ID	0x0000			// ID of the MESH Network

uint16_t GATEWAY_ID = 0x4444;		// ID of the Gateway data wil be transmitted too

uint8_t nodeId;
String latS, lngS;
bool dataSent;



/*---------------------------------ADDITIONAL GPS METHODS--------------------------*/

// Converts UTC time to the time of the predefined time zone
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


/*---------------------------------------------UBX METHODS----------------------------------*/

void configGPS()
{
	int gps_set_sucess;

	for (int i = 0; i < CONFIG_SIZE; i++) {
		gps_set_sucess = 0;
		while (!gps_set_sucess) {
			sendUBX(config[i], config_size[i]);
			gps_set_sucess = getUBX_ACK(config[i]);
		}
	}
}

void sendUBX(uint8_t* MSG, uint8_t len) {
	Serial.flush();
	Serial.write(0xFF);
	_delay_ms(500);
	for (int i = 0; i < len; i++) {
		Serial.write(MSG[i]);
	}
}

boolean getUBX_ACK(uint8_t* MSG) {
	uint8_t b;
	uint8_t ackByteID = 0;
	uint8_t ackPacket[10];
	unsigned long startTime = millis();

	// Construct the expected ACK packet
	ackPacket[0] = 0xB5; // header
	ackPacket[1] = 0x62; // header
	ackPacket[2] = 0x05; // class
	ackPacket[3] = 0x01; // id
	ackPacket[4] = 0x02; // length
	ackPacket[5] = 0x00;
	ackPacket[6] = MSG[2]; // ACK class
	ackPacket[7] = MSG[3]; // ACK id
	ackPacket[8] = 0; // CK_A
	ackPacket[9] = 0; // CK_B

   // Calculate the checksums
	for (uint8_t ubxi = 2; ubxi < 8; ubxi++) {
		ackPacket[8] = ackPacket[8] + ackPacket[ubxi];
		ackPacket[9] = ackPacket[9] + ackPacket[8];
	}

	while (1) {
		// Test for success
		if (ackByteID > 9) {
			// All packets in order!
			return true;
		}

		// Timeout if no valid response in 3 seconds
		if (millis() - startTime > 3000) {
			return false;
		}

		// Make sure data is available to read
		if (Serial.available()) {
			b = Serial.read();

			// Check that bytes arrive in sequence as per expected ACK packet
			if (b == ackPacket[ackByteID]) {
				ackByteID++;
			}
			else {
				ackByteID = 0; // Reset and look again, invalid order
			}
		}
	}
}

/*-----------------------------------------IMPORTANT LORA METHODS---------------------------*/

/*
Read, process, and send the Lattitude and Longitude values
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
	sendData(loraPort, GATEWAY_ID, (payloadLen * 2), payload);

	free(payload);
	free(payload1);
}
/*
Conversion process:
1) check sign of value and remove -ve sign if it exists
	Set the value of the first Byte of data packet accordingly
			- = 0x01
			+ = 0x00
2) Break the data into 2-byte values and add the data (in order) to the payload array
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
  Convert the long GPS data into String values as if they were high-precision floats
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
	int32_t deg = degE7 / 1000000L;
	data.concat(deg);
	data.concat('.');


	// Get fractional degrees
	degE7 -= deg * 1000000L;

	// Print leading zeroes, if needed
	int32_t factor = 100000L;
	while ((degE7 < factor) && (factor > 1L)) {
		data.concat('0');
		factor /= 10L;
	}

	// Print fractional degrees
	data.concat(degE7);

	return data;
}

/*-----------------------------------IMPORTANT GPS METHODS---------------------------*/
/*
Prints various GPS data fpr debugging purposes
*/
static void printGpsData()
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
	DEBUG_PORT.print(dataToString(fix.latitudeL())); // prints int like a float
	DEBUG_PORT.print(',');
	// DEBUG_PORT.print( fix.longitude(), 6 ); // floating-point display
	// DEBUG_PORT.print( fix.longitudeL() );  // integer display
	DEBUG_PORT.print(dataToString(fix.longitudeL())); // prints int like a float
	DEBUG_PORT.println();

	DEBUG_PORT.print("# SVs Used:     ");
	DEBUG_PORT.print(fix.valid.satellites);
	DEBUG_PORT.print("   ,   ");
	DEBUG_PORT.print(fix.satellites);

	DEBUG_PORT.println();
	DEBUG_PORT.println();


}

/*
Reads from the GPS
*/
static void GPSloop()
{
	while (gps.available(gpsPort)) {
		fix = gps.read();
	}
}

/*---------------------------------ADDITIONAL LORA METHODS--------------------------*/


/*
Prints 1-byte values in Hexadecimal format
*/
void printByte(uint8_t b) {
	DEBUG_PORT.print("0x");
	if (b <= 0xF)
		DEBUG_PORT.print("0");
	DEBUG_PORT.print(b, HEX);
}

/*
Prints 2-byte values in Hexadecimal format
*/
void printShort(uint16_t s) {
	DEBUG_PORT.print("0x");
	if (s <= 0xFFF)
		DEBUG_PORT.print("0");
	if (s <= 0xFF)
		DEBUG_PORT.print("0");
	if (s <= 0xF)
		DEBUG_PORT.print("0");
	DEBUG_PORT.print(s, HEX);
}

/*
Reads values from the predefined LoRa port
*/
uint8_t readByte(Stream & port) {
	while (!port.available());
	return port.read();
}

/*
Processes the Frame type from the LoRa Module?
*/
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

/*
Properly formats the frame before transmission?
*/
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

/*
Formats the payload
Global variable dataSent checks if data has been received by the Gateway (used to check if underwater)
Max length of payload is 111 bytes
*/
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

		// Global  varaible that saves whether data was sent or not
		dataSent = status;
	}
	free(responsePayload);
}

/*
Reads the received payload
*/
void readData(Stream & port) {
	uint8_t frameType = 0;
	uint8_t cmdType = 0;
	uint8_t* payload = NULL;
	int len = readFrame(port, &frameType, &cmdType, &payload);

	// What does BAD TYPE mean and how should it be handled?
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

/*
Configurs certain parameters of the LoRa MESH Module
@params: port:
		 netID: Desired ID of the Node
		 nodeID: Desired ID of the Network the Node will use
*/
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
Read the configurations of the LoRa MESH Module
NodeID is defined globally so it can be used in data transmissions
*/
void readConfig(Stream& port) {
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
	nodeId = (payload[8] << 8) | payload[9];
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



/*----------------------------------SETUP AND LOOP FUNCTION------------------------*/

void setup() {
	delay(3000);

	//Initilie Serial Ports & Pins
	loraPort.begin(LORA_BAUDRATE);
	gpsPort.begin(GPS_BAUDRATE);
	DEBUG_PORT.begin(PC_BAUDRATE);

	pinMode(GPS_ENABLE_PIN, OUTPUT);
	digitalWrite(GPS_ENABLE_PIN, LOW);
	delay(1000);

	// Configure the GPS Module (if it posses the UBLOX Protocol)
#if defined(UBLOX_PROTOCOL_AVAILABLE)
	DEBUG_PORT.println("Configurating GPS...");		
	configGPS();
	DEBUG_PORT.println("GPS successfully configured");
	delay(500);
#endif

	// Initilize LoRa Module and display debugging data
	DEBUG_PORT.println("Configuration Setup:");	
	DEBUG_PORT.println();
	writeConfig(loraPort, NODE_ID, NETWROK_ID);		// Edit the Nodes Network ID and Node ID
	delay(1000);
	readConfig(loraPort);

	// Initialize NEOGPS and display debugging data for GPS
	DEBUG_PORT.println(F("NMEAtimezone.INO: started"));
	DEBUG_PORT.println(F("Looking for GPS device on " GPS_PORT_NAME));
	DEBUG_PORT.println(F("Local time"));

	DEBUG_PORT.println(F("NMEAloc.INO: started"));
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

	DEBUG_PORT.println("Starting...");

	// Data was sent successfully if dataSent equals 0x00
	DEBUG_PORT.println("Sending Dummy Transmission");
	sendData(loraPort, GATEWAY_ID, 1, 0x00);

	if (dataSent != 0x00) {
		goto lowPowerMode;
	}

	DEBUG_PORT.println("Running GPS");
	digitalWrite(GPS_ENABLE_PIN, HIGH);

	long gpsRuntime = millis();	// Tracks time that GPS has been running for

	while (millis() - gpsRuntime < GPS_FIX) { GPSloop(); }

	digitalWrite(GPS_ENABLE_PIN, LOW);

	DEBUG_PORT.println("GPS Data:    ");
	printGpsData();
	delay(500);
	DEBUG_PORT.println("Sending Data");
	prepareData();
	DEBUG_PORT.println("Data Sent");
	delay(100);

	lowPowerMode:

	
	Serial.println("Going to Sleep");
	Serial.println();
	delay(1000);

	/*for (int i = 0; i < sleepTime; i++) {
		LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
	}*/
	Serial.println("Waking Up");
	delay(500);
	

}