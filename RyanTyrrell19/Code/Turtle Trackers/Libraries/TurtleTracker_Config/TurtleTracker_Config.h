/*
 Name:		TurtleTracker_Config.h
 Created:	8/1/2019 10:53:04 AM
 Author:	tyrre
 Editor:	http://www.visualmicro.com
*/

#ifndef _TurtleTracker_Config_h
#define _TurtleTracker_Config_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <NeoSWSerial.h>
#include <HardwareSerial.h>
#include <EEPROM.h>
#include <MemoryFree.h>

/*--------------------------------------HARDWARE CONFIURATION-------------------------------*/
#define UBLOX_PROTOCOL_AVAILABLE						// Uncomment if The GPS Module uses the UBLOX Protocol
#define RTC_AVAILABLE									// Uncomment if the GPS module is capable of retaining RTC data (i.e. is has a Battery Backup Pin)

/*--------------------------------------PIN DECLARATION SETUP-------------------------------*/
// Software Serial baud rate must be very large
// Hardware Serial cannot be larger than Software Serial
// AltSoftwaSerial must be 10x smaller than Software Serial
// Avoidi using the same buadrate for different Serial communications

// Pin declarations for GPS Module
// GPS default baud rate is 9600 -> NeoSWSerial;  Supported baud rates are 9600 (default), 19200 and 38400.
#define GPS_RX 3		
#define GPS_TX 4
#define GPS_VBAT_PIN 5									// Pin used to power the GPS to retain RTC and position data
#define GPS_ENABLE_PIN 6								// Pin used to power on/off GPS (Low = Disable, High = Enable)

// Pin declarations for RFM95 Module (RFM95 uses SPI to communicate. If adding another SPI device, need to include slave selections)
//#define RFM95_SLAVE 10
//#define RFM95_RESET 7
//#define RFM95_INTERUPT 2

/*--------------------------------------SERIAL CONFIGURATION---------------------------------*/

//// Serial Setup
//#define DEBUG_PORT Serial
//#define PC_BAUDRATE 115200L
//
//#define GPS_PORT_NAME "gpsPort"
//#define GPS_BAUDRATE 9600L

/*--------------------------------------LOW POWER CONFIGURATION-----------------------------*/

#define WAKE_TIME_HOUR 14									// Hour of day to end the long sleep
#define WAKE_TIME_MIN 11									// Minute of the day to end long sleep
#define NUM_FIXES_PER_DAY 3									// Number of times the GPS should get a fix before entering Long Sleep again

#define CONSTANT_SLEEP 60									// IF no RTC, have Arduino sleep periodically for 60min
#define SHORT_SLEEP_TIME 2									// Set the time the Arduino will sleep for during the short sleep interval, in minutes - Max value of 60

#define SLEEP_THRESHOLD	30									/* The Arduino will continuosly loop between sleeping, waking up, correcting
															 the sleep time, and sleeping again until the sleep time is below this value.
															 Once below this value, the Arduino will sleep once more, wake-up, and enter the GPS cycle.
															 A larger value will introduce a larger error from the watchdog, causing the fix times to be
															 more off the target fix time
															 (in seconds)
															*/

#define NO_SLEEP 0											// Indicates no additional sleep is required
#define SHORT_SLEEP 1										// Indicates another short sleep is required
#define LONG_SLEEP 2										// Indiactes another long sleep is required
#define WATCHDOG_CORRECTION 0.15							// Value that the computed long sleep time is multiplied & subtracted by to account for the 
															// Watchdog timer error and prevent overshooting the target wakeup time

/*--------------------------------------GPS CONFIGURATION----------------------------------------*/

#define	GPS_FIX 40000									// maximum time spent getting GPS fix (in milliseconds)
#define GPS_LOOP_TIME 2000								// Min. time spent in GPSLoop() to ensure fix is updated at least once
#define GPS_PRERUN 5000									// Amount of time to let the GPS initially run. Depending on the data collected, either shut down the GSP or continue letting
														// it run (since it may be underwater)


# define GPS_TIME_ADJUSTMENT 946684800					// Amount of seconds to change UNIX epoch time from 2000 to 1970
/*--------------------------------------UBX CONFIURATION------------------------------------*/
/*
Byte 1 & 2: Always 0xB5, 0x62
Byte 2: Always 0x62
Byte 3: Class Field
Byte 4: ID Field
Byte 5 & 6: Payload Length
*/
// These variablesa are saved to FLASH memory to save SRAM (hence the PROGMEM)

/*________Enable Messages__________*/
// Enable AID_INI (Initial Data)
const uint8_t AID_INI[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x0B, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x07 };

//Enable MON-HW (Hardware Status)
const uint8_t MON_HW[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x0A, 0x09, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x23, 0x37 };
//Enable MON-TXBUF (TX Buffer)
const uint8_t MON_TXBUF[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x0A, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x22, 0x30 };

//Enable NAV-AOPSTATUS (AssistNow Autonomous Status)
const uint8_t NAV_AOPSTATUS[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x60, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x71, 0x50 };
//Enable NAV-DGPS (DGPS Data)
const uint8_t NAV_DGPS[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x31, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42, 0x07 };
// Enable NAV-DOP (Dilution of Precision)
const uint8_t NAV_DOP[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x15, 0xCC };
// Enable NAV-SOL (Navigation Solution)
const uint8_t NAV_SOL[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x17, 0xDA };
// Enable NAV-STATUS (Navigation Status)
const uint8_t NAV_STATUS[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x14, 0xC5 };


/*________GPS Configurtion_________*/
//CFG-NAV5 - Configure NAV5
const uint8_t CFG_NAV5[] PROGMEM = { 0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4F, 0x82 };

//CFG-NAVX5 - Configure NAVX5
const uint8_t CFG_NAVX5[] PROGMEM = { 0xB5, 0x62, 0x06, 0x23, 0x28, 0x00, 0x00, 0x00, 0x4C, 0x66, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x16, 0x07, 0x00, 0x00, 0x00, 0x00, 0x01, 0x9B, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEA, 0xF7 };

// CFG-SBAS - Configure SBAS
const uint8_t CFG_SBAS[] PROGMEM = { 0xB5, 0x62, 0x06, 0x16, 0x08, 0x00, 0x01, 0x03, 0x03, 0x00, 0x00, 0xE8, 0x04, 0x00, 0x17, 0x79 };

// CFG-CFG - Save Config. setings
const uint8_t CFG_CFG[] PROGMEM = { 0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1D, 0xAB };

#define CONFIG_SIZE 12

//
///*--------------------------------------LORA CONFIURATION------------------------------------*/
//// Mesh has much greater memory requirements, and you may need to limit the
//// max message length to prevent weird crashes
//
//// Module Settings
//#define TX_POWER 23										// Output power of the RFM95 (23 is the max)
//#define FREQUENCY 915.0
//#define PREAMBLE_LENGTH 550								// Preamble length of LoRa message, in bytes
//
//// Timeout values for various blocking functions included in the Mesh Network Protocol
//#define ACK_SEND_TIMEOUT 10000							// Timeout for waiting for ACK
//#define NODE_ACK_TIMEOUT 5000							// Timeout for Node waitig to receive ACK from Gateway
//#define RELAY_TIMEOUT 10000								// Timeout for Relay/Gateway to recieve data and sed ACK
//#define MESH_ROUTE_TIMEOUT 7000							// Timeout for aquiring a route
//
//
//#define GATEWAY_ACK_MESSAGE_LENGTH 1					// TODO
//#define GATEWAY_ACK_MESSAGE 1							// TODO
//
//// Various length of values used in the payload
//#define RH_PAYLOAD_LEN 30								// Length of the payload sent via LoRa
//
//// ID's for the Network
//#define MAX_NETWROK_SIZE 10								// Totoal number of End-Nodes, Relayer's, and Gateway's in the network
//#define NODE_ID 1										// TODO
//#define RELAY_ID 2										// TODO
//#define GATEWAY_ID 3									// TODO
//
//#define DATA_TRANSFER_MESSAGE "      44444"

/*--------------------------------------LORA CONFIURATION------------------------------------*/

// Pin declarations for RFM95 Module
#define RFM95_SLAVE 10
#define RFM95_RESET 9
#define RFM95_INTERUPT 2

// Module Settings
#define TX_POWER 23										// Output power of the RFM95 (23 is the max)
#define FREQUENCY 915.0
#define PREAMBLE_LENGTH 550								// Preamble length of LoRa message, in bytes

#define CAD_SLEEP 300

// # of Nodes in the network and ID's for the Network
#define MAX_NUM_END_NODES 1
#define NODE_ID 1
#define RELAY_ID 2
#define GATEWAY_ID 3

// Time to wait for message after preamble detected
#define WAIT_AVAILABLE_TIMEOUT 5000

// Various length of values used in the payload
#define RH_PAYLOAD_LEN 45				
#define RH_MESSAGE_ACK_LEN 2
#define RH_MESSAGE_LIFESPAN_LEN 2
#define RH_MESSAGE_ID_LEN 5
#define RH_MESSAGE_LEN 35
#define RH_MESSAGE_LIFESPAN "7"

#define MAX_SEND_MESSAGES 20

/*
An array of pointers to strings is an array of character pointers where each pointer points to the first character of the string or the base address of the string
So messageIdArray can contain up to MESSAGE_ID_ARRAY_SIZE different pointers
*/
// Used to store previously seen message ID's & their asscociated ACK
#define MESSAGE_ID_ARRAY_SIZE 10

#define ACK_WAIT_TIME 10000

#endif

