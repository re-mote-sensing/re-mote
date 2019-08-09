/*
 Name:		Turtle_Trackers___RFM95.ino
 Created:	7/19/2019 2:00:57 PM
 Author:	tyrre
*/

/*----------------------------------QUESTIONS--------------------------------------*/


/*----------------------------------TODO---------------------------------------------*/


// Add support for different GPS Modules & GPS protocols
// Algorithm that takes into account time spent underwater to determine allowable time to aquire GPS + aided data

// Add a better check for if underwater
// Check for when Underwater (use LoRa?)
// Counter the keep track of time sent underwater - can report this value during next successful transmission

// save data AND resend data if route not found
// add code to also sleep the rfm95

// Issue - Possible for MESH Netwrok to briefly turn off while sending saved data

/*-----------------------------------NOTES------------------------------------------*/

// uint_8 = char		(1 byte)
// uint16_t = short		(2 bytes)
// uint32_t = int		(4 bytes)
// uint64_t = long		(8 bytes)

// When EN pin is LOW, RTC is not retained. Must use the V_bAT pin (The datasheet made it sound as if this was not necessary
// EN Pin and V_BAT Pin work as expected
// Having V_BAT Pin powered while EN Pin is LOW (i.e.GPS is at full power) does not cause excess current drawn.So the V_BAT pin can be left HIGH indefinitely(may require additional tests to confirm)
// V_BAT Pin output HIGH when Arduino In Low Power? - Yes
// GPS needs full Position data before it saves the RTC and position. After that, Enable and Backup work as expected

// Check output of I/O pins in Low Power Mode - Output stays as what it was set to
// LowPower will NOT work if GPS is left on


// Put GPSLoop() in do...while loop so the data is updated before being checked
	// Nah; still chance of not being updated. Need to have it wait untill .avaialble returns true so fix is updated
	// Edit GPSLoop itself. Make sure it stays there for set amount of time (2 seconds seems good)

// Altered GPSfix_cfg.h in an attempt to reduce RAM usage
// Modified RHMesh.h to include getter/setter for Mesh Route discovery timeout
// Modified RHMesh.cpp - recvfromAck - line 223; additional else statment for if only receiving a broadcast

// Make the code prepar to reset at any point

/*------------------------------INCLUDES---------------------------*/

#include <NMEAGPS.h>
#include <TurtleTracker_LowPower.h>
#include <TurtleTracker_UBX.h>
#include <TurtleTracker_EEPROM.h>
#include <TurtleTrackers_LoRa_Flooding.h>


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

#if !defined( GPS_FIX_SATELLITES )
#error You must uncomment GPS_FIX_SATELLITES in GPSfix_cfg.h!
#endif

#ifdef NMEAGPS_INTERRUPT_PROCESSING
#error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
#endif

/*______NEOGPS TIME ZONE SETUP (Configured for the US)_____*/

// Set these values to the offset of your timezone from GMT

static const int32_t          zone_hours = -5L; // EDT
static const int32_t          zone_minutes = 0L; // usually zero
static const NeoGPS::clock_t  zone_offset =
zone_hours * NeoGPS::SECONDS_PER_HOUR +
zone_minutes * NeoGPS::SECONDS_PER_MINUTE;

static const uint8_t springMonth = 3;
static const uint8_t springDate = 14; // latest 2nd Sunday
static const uint8_t springHour = 2;
static const uint8_t fallMonth = 11;
static const uint8_t fallDate = 7; // latest 1st Sunday
static const uint8_t fallHour = 2;
#define CALCULATE_DST
/*-------------------------------------SERIAL CONFIGURATION---------------------------------*/
// Software Serial baud rate must be very large
// Hardware Serial cannot be larger than Software Serial
// AltSoftwaSerial must be 10x smaller than Software Serial
// GPS default baudrate is 9600
// NeoSWSerial - Supported baud rates are 9600 (default), 19200 and 38400.


// Serial Setup
#define DEBUG_PORT Serial
#define PC_BAUDRATE 115200L

#define GPS_PORT_NAME "gpsPort"
NeoSWSerial gpsPort(GPS_RX, GPS_TX);
#define GPS_BAUDRATE 9600L

/*--------------------------------------------GPS SETUP-------------------------------------*/

static NMEAGPS  gps;									// This parses received characters from the GPS
static gps_fix  fix;									// This contains all the parsed pieces

/*--------------------------------------------LORA SETUP-------------------------------------*/

RH_RF95 rf95(RFM95_SLAVE, RFM95_INTERUPT);						// Singleton instance of the radio driver
//RH_RF95 rf95;														// Singleton instance of the radio driver
RHReliableDatagram *RHReliableDatagramManager;												// Class to manage message delivery and receipt, using the driver declared above

char data[RH_MESSAGE_LEN];										// This is the payload that is tranmistted

/*-----------------------------------------LIBRARY DEFINITIONS------------------------------------*/
TurtleTracker_UBX turtleTracker_UBX(DEBUG_PORT, gpsPort);
TurtleTracker_LowPower turtleTracker_LowPower(DEBUG_PORT);
TurtleTrackers_LoRa_Flooding turtleTracker_LoRa_Flooding(DEBUG_PORT);
TurtleTracker_EEPROM turtleTracker_EEPROM;

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

/*---------------------------------------------------------------------------------*/
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
	int32_t deg = degE7 / 10000000L;
	data.concat(deg);
	data.concat('.');


	// Get fractional degrees
	degE7 -= deg * 10000000L;

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

	DEBUG_PORT.println(F("Time:   "));
	DEBUG_PORT.print(F("Valid:   "));
	DEBUG_PORT.print(fix.valid.time ? "YES" : "NO");
	DEBUG_PORT.print(F("   |   "));
	adjustTime(fix.dateTime);
	DEBUG_PORT << fix.dateTime;
	DEBUG_PORT.println();

	DEBUG_PORT.println(F("Time (UTC - Seconds:   "));
	DEBUG_PORT.print(F("Valid:   "));
	DEBUG_PORT.print(fix.valid.time ? "YES" : "NO");
	DEBUG_PORT.print(F("   |   "));
	unsigned long currTime = ((NeoGPS::clock_t) fix.dateTime) + 946684800; //Get Unix Time (946684800 is because conversion gives Y2K epoch)
	DEBUG_PORT.println(currTime);

	DEBUG_PORT.println(F("Lat/Lon:"));
	DEBUG_PORT.print(F("Valid:   "));
	DEBUG_PORT.print(fix.valid.location ? "YES" : "NO");
	DEBUG_PORT.print(F("   |   "));
	// DEBUG_PORT.print( fix.latitude(), 6 ); // floating-point display
	// DEBUG_PORT.print( fix.latitudeL() ); // integer display
	DEBUG_PORT.print(dataToString(fix.latitudeL())); // prints int like a float
	DEBUG_PORT.print(F(" , "));
	// DEBUG_PORT.print( fix.longitude(), 6 ); // floating-point display
	// DEBUG_PORT.print( fix.longitudeL() );  // integer display
	DEBUG_PORT.print(dataToString(fix.longitudeL())); // prints int like a float
	DEBUG_PORT.println();

	DEBUG_PORT.println(F("# SVs Used:"));
	DEBUG_PORT.print(F("Valid:   "));
	DEBUG_PORT.print(fix.valid.satellites ? "YES" : "NO");
	DEBUG_PORT.print(F("   ,   "));
	DEBUG_PORT.print(fix.satellites);

	DEBUG_PORT.println();
	DEBUG_PORT.println();
}

/*
Reads from the GPS
*/
static void GPSloop()
{
	long time = millis();
	while (millis() - time < GPS_LOOP_TIME) {
		while (gps.available(gpsPort)) {
			fix = gps.read();
		}
	}
}


/*----------------------------------SETUP AND LOOP FUNCTION------------------------*/
void setup() {
	
/*** Initilize Serial Ports, Pins, and Random Generator ***/
	randomSeed(analogRead(0));
	gpsPort.begin(GPS_BAUDRATE);
	DEBUG_PORT.begin(PC_BAUDRATE);
	delay(500);


#if defined(RTC_AVAILABLE)
	pinMode(GPS_ENABLE_PIN, OUTPUT);
	pinMode(GPS_VBAT_PIN, OUTPUT);
	digitalWrite(GPS_VBAT_PIN, HIGH);		// Leave the V_BAT Pin HIGH to ensure GPS is always powered (to retain RTC and position data)
											// Doing so does not cause additional power drawn (as far as I can tell). Will remain HIGH in Low Power Mode
	digitalWrite(GPS_ENABLE_PIN, LOW);

#endif

/*** Initilize LoRa Module and display debugging data	***/
	//Uncomment to set the ID of the Node
	if (!turtleTracker_LoRa_Flooding.setNodeId(NODE_ID))
		return;

	uint8_t nodeId = turtleTracker_LoRa_Flooding.getNodeId();
	RHReliableDatagramManager = new RHReliableDatagram(rf95, nodeId);

	if (turtleTracker_LoRa_Flooding.init(rf95, *RHReliableDatagramManager))
		DEBUG_PORT.println(F("RF95 ready\n"));
	else
		DEBUG_PORT.println(F("RF95 init Failed\n"));

	delay(1000);

	/*** Check how much Memory is available for use ***/
	DEBUG_PORT.print(F("Memory Available = "));
	DEBUG_PORT.println(freeMemory());
	DEBUG_PORT.println();
	// Placed before Pin definition because undefined behaviour occurs once the EN pin is defined
	/*** Configure the GPS Module (if it supports the the UBLOX Protocol) ***/
#if defined(UBLOX_PROTOCOL_AVAILABLE)
	digitalWrite(GPS_ENABLE_PIN, HIGH);
	delay(1000);
	turtleTracker_UBX.configGPS();
	digitalWrite(GPS_ENABLE_PIN, LOW);
	DEBUG_PORT.println();
	delay(500);
#endif

/*** Initialize NEOGPS and display debugging data for GPS ***/
	DEBUG_PORT.println(F("NMEAtimezone.INO: started"));
	DEBUG_PORT.println(F("Looking for GPS device on " GPS_PORT_NAME));
	DEBUG_PORT.println(F("Local time\n"));

	DEBUG_PORT.println(F("NMEAloc.INO: started"));
	DEBUG_PORT.print(F("fix object size = "));
	DEBUG_PORT.println(sizeof(gps.fix()));
	DEBUG_PORT.print(F("NMEAGPS object size = "));
	DEBUG_PORT.println(sizeof(gps));
	DEBUG_PORT.println(F("Looking for GPS device on " GPS_PORT_NAME));
	DEBUG_PORT.println();

	/*** Check how much Memory is available for use ***/
	DEBUG_PORT.print(F("Memory Available = "));
	DEBUG_PORT.println(freeMemory());
	DEBUG_PORT.println();
	delay(1000);

	DEBUG_PORT.flush();
}

void loop() {

	DEBUG_PORT.println(F("Starting..."));

	/*** Let GPS run to aquire data ***/
	DEBUG_PORT.println(F("Running GPS"));
	gpsPort.listen();
	digitalWrite(GPS_ENABLE_PIN, HIGH);

	long gpsRuntime = millis();	// Tracks time that GPS has been running for
	// Let the GPS run for a few seconds to determine if it's underwater
	while (millis() - gpsRuntime < GPS_FIX && !fix.valid.location) {
		GPSloop();
	}
	if (fix.satellites == 0)
		goto lowPowerMode;

	// All good. Continue running to aquire a fix
	while (millis() - gpsRuntime < GPS_FIX && !fix.valid.location) { 
		GPSloop(); 
	}

	DEBUG_PORT.print(F("RUNTIME:     "));
	DEBUG_PORT.println(millis() - gpsRuntime);

	digitalWrite(GPS_ENABLE_PIN, LOW);

	DEBUG_PORT.println(F("GPS Data:    "));
	printGpsData();
	delay(500);

	/*** Construct Payload and Send Data using LoRa RFM95 ***/
	String latS = dataToString(fix.latitudeL());
	String lngS = dataToString(fix.longitudeL());
	unsigned long currTime = ((NeoGPS::clock_t) fix.dateTime) + GPS_TIME_ADJUSTMENT;
	char currTimeS[sizeof(currTime) / sizeof(unsigned long)];
	ltoa(currTime, currTimeS, 10);

	/*DEBUG_PORT.println(F("Size of Lat, Lng, and Time:"));
	DEBUG_PORT.println(sizeof(latS));
	DEBUG_PORT.println(sizeof(lngS));
	DEBUG_PORT.println(sizeof(currTimeS));

	const uint8_t size_t = sizeof(latS) + sizeof(lngS) + sizeof(currTimeS) + 3;
	DEBUG_PORT.println(size_t);
	
	char payload[size_t];										// This is the payload that is tranmistted
	*/

	strcpy(data, latS.c_str());
	strcat(data, " , ");
	strcat(data, lngS.c_str());
	strcat(data, " , ");
	strcat(data, currTimeS);

	DEBUG_PORT.print(F("Data:   "));
	DEBUG_PORT.println(data);

	/*
	Add code here for sending saved data. Maybe just put a large delay (tracker can sleep during it) to ensure all broadcasts have finished, then send next payload
	
	
	
	*/

	// Broadcast Message. Save is send is unsuccessful	
	if (!turtleTracker_LoRa_Flooding.sendData(data))
		turtleTracker_EEPROM.savePayload(data);

	//// Establish connection with a Relayer
	//TurtleTracker_LoRa turtleTracker_LoRa(DEBUG_PORT);
	//bool error = false;
	//
	//uint8_t from;
	//error = turtleTracker_LoRa.initConnection(&from);
	//
	//if (!error) {
	//	DEBUG_PORT.println(F("Data NOT Sent"));
	//	goto lowPowerMode;
	//}
	//DEBUG_PORT.println(F("Sending Data"));
	//
	//// Determine how many payloads are to be sent
	//uint8_t count = turtleTracker_EEPROM.getSavedPayloadsCounter();
	//
	//// Send this value to the Relayer so it knows how long to wait for
	//strcpy(payload, count);
	//error = turtleTracker_LoRa.sendDataToRelayer(&from, payload);
	//if (!error) {
	//	DEBUG_PORT.println(F("Connection Lost. Abort"));
	//	goto lowPowerMode;
	//}
	//
	//// Send any saved data
	//for (uint8_t i = count - 1; i >= 0; i--) {
	//	turtleTracker_EEPROM.getSavedPayload(i, payload);
	//
	//	if (turtleTracker_LoRa.sendDataToRelayer(&from, payload)) {
	//		DEBUG_PORT.print(F("Saved payload Sent - #"));
	//		DEBUG_PORT.println(i);
	//	}
	//	else {
	//		DEBUG_PORT.println(F("Unable to send saved payload. Abort"));
	//		goto lowPowerMode;
	//	}
	//	// decrement saved payload counter so next payload to not be sent will override it
	//	turtleTracker_EEPROM.setSavedPayloadsCounter(turtleTracker_EEPROM.getSavedPayloadsCounter() - 1);
	//}

	delay(100);

lowPowerMode:
	rf95.sleep();			// Sleep the RFM95 Module
	bool isValid = true;

	do {
		// Power on the GPS brief enough to update available data
		DEBUG_PORT.println("Getting Time from GPS");
		gpsPort.listen();
		digitalWrite(GPS_ENABLE_PIN, HIGH);

		// update fix variable with latest time from GPS
		GPSloop();

		digitalWrite(GPS_ENABLE_PIN, LOW);

		// Print the time for debugging purposes
		DEBUG_PORT.println("GPS Data:    ");
		// Below calls Adjust time. If removed, need to put adjustTime() in its place
		printGpsData();

		isValid = false;
#if defined(RTC_AVAILABLE)
		if (fix.valid.time)
			isValid = true;
#endif

		// If the RTC data is not valid, use the default sleep time (1 hour)
		if (!isValid) 
			DEBUG_PORT.println(F("RTC Time not Valid or Available. Using Default Sleep time"));
		else
			DEBUG_PORT.println(F("RTC Time Valid"));


	} while (!turtleTracker_LowPower.enterLowPowerMode(fix.dateTime.hours, fix.dateTime.minutes, fix.dateTime.seconds, isValid));
}