/*
Header for some config parameters, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Author: Victor Vezina, Ryan Tyrrell and Tianyu Zhou
Last updated: August 7, 2022
Released into the public domain
*/

/*-----------------------------------------------------------------*/
//Look at (LINK!) for information on how to configure this correctly
/*-----------------------------------------------------------------*/

#ifndef remoteConfig_h
#define remoteConfig_h

// Defines used for internal comparisons, ignore these
#define Sensor_End_Node 0
#define Gateway 1
#define Turtle_Tracker 2

/*---------Type of node---------*/
#define Node_Type Turtle_Tracker       // Either Sensor_End_Node, Gateway, or Turtle_Tracker


/*---------Overall Network Settings---------*/
#define GATEWAY_ID 0x301                       // LoRa ID of the Gateway Node
#define NETWORK_ID 0x02                         // LoRa Network ID

#define URL_Host "macwater.cas.mcmaster.ca"     // Host of the URL to post data to
#define URL_Path ""                             // Path after the URL to post data to
#define URL_Port 80                             // Port of the URL to open an HTTPS session with

/*---------Type specific Settings---------*/

/*-----------------------------------------------------------*/
/*----------------------Sensor_End_Node----------------------*/
/*-----------------------------------------------------------*/
#if Node_Type == Sensor_End_Node
    /*---------End Node settings---------*/

    /*---------Base Parameters---------*/
    #define NAME "Node_1"                // Name of this end node
    #define NODE_ID 0x01                // ID of this end node
    #define LORA_RX A4                  // Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
    #define LORA_TX A5                  // Pin that the LoRa RXD pin is connected to
    #define GPS_RX 5                    // Pin that the GPS TXD pin is connected to
    #define GPS_TX 4                    // Pin that the GPS RXD pin is connected to
    #define GPS_EN 2                    // Pin that the GPS EN pin is connected to

    /*---------Sensor definitions---------*/
    #define NUMBER_SENSOR_NAMES 5   // The number of sensor names on the end node
    #define NUMBER_SENSOR_TYPES 5   // The number of sensor types on the end node
    #define MAX_NUMBER_PINS 3       // The max number of pins that one sensor uses

    extern const char* sensorNames[NUMBER_SENSOR_NAMES];
    extern const char* sensorTypes[NUMBER_SENSOR_TYPES];
    extern const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][MAX_NUMBER_PINS];
    #ifdef MAIN

        const char* sensorNames[NUMBER_SENSOR_NAMES] = {"Water_Temperature", "pH", "Dissolved_Oxygen", "Turbidity", "Conductivity"};
        const char* sensorTypes[NUMBER_SENSOR_TYPES] = {"DF_Temp", "AS_pH", "AS_DO", "DF_TB", "AS_EC"};
        const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][MAX_NUMBER_PINS] = { {6}, {8, 9, 7}, {11, 12, 10}, {A3, A2}, {A0, A1, 13} };

    #endif

    // All the sensor types that are used in this end node
    #define AS_DO_Sensor true       // Set this value to if the EC sensor has an enable pin or not
    #define AS_EC_Sensor true       // Set this value to if the EC sensor has an enable pin or not
    #define AS_pH_Sensor true       // Set this value to if the pH sensor has an enable pin or not
    #define DF_TB_Sensor
    #define DF_Temp_Sensor
    // #define DHT22_Sensor

    // Temperature and salinity compensation defines
    #define Temperature_Comp 0
    // #define Temperature_Comp_Index 0
    #define Salinity_Comp 1
    // #define Salinity_Comp_Index 0

    /*---------Extra Parameters---------*/
    #define Data_Type EEPROM_Type                   // Type of data storage being used
    #define GPS_Time 60000                          // Time this node will try the GPS every time it takes a measurement
    #define LoRa_Read_Timeout 10000                 // Time that it will try to read from LoRa before giving up
    #define Sleep_Time 3600000                       // Time that the node sleeps on success 240000
    #define Fail_Sleep_Time (Sleep_Time * 3) / 4    // Time that this node sleeps on fail
    #define Sleep_Type SLEEP_MODE_PWR_DOWN          // Type of sleep this node uses

    #define DEBUG                                   // Put code in debug mode
    #define DEBUG_NO_GPS                            // Disable GPS

/*-----------------------------------------------------------*/
/*--------------------------Gateway--------------------------*/
/*-----------------------------------------------------------*/
#elif Node_Type == Gateway
    /*---------Gateway settings---------*/

    /*---------Base Parameters---------*/
    #define NAME "Gateway_1"        // Name of this gateway
    #define ENABLE_LORA             // Comment out to disable the LoRa module
        #define LORA_RX 6           // Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
        #define LORA_TX 7           // Pin that the LoRa RXD pin is connected to
    #define cell3G_RX 4             // Pin that the 3G module TX pin is connected to
    #define cell3G_TX 5             // Pin that the 3G module RX pin is connected to
    #define cell3G_EN 8             // Pin that the 3G module KEY pin is connected to

    /*---------Sensor definitions---------*/
    #define NUMBER_SENSOR_NAMES 2   // The number of sensor names on the gateway
    #define NUMBER_SENSOR_TYPES 1   // The number of sensor types on the gateway
    #define MAX_NUMBER_PINS 1       // The max number of pins that one sensor uses

    extern const char* sensorNames[NUMBER_SENSOR_NAMES];
    extern const char* sensorTypes[NUMBER_SENSOR_TYPES];
    extern const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][MAX_NUMBER_PINS];
    #ifdef MAIN
    
        const char* sensorNames[NUMBER_SENSOR_NAMES] = {"Air_Temperature", "Humidity"};
        const char* sensorTypes[NUMBER_SENSOR_TYPES] = {"DHT22"};
        const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][MAX_NUMBER_PINS]= { {2} };
        
    #endif

    // All the sensor types that are used in this end node
    // #define AS_DO_Sensor true     // Set this value to if the pH sensor has an enable pin or not
    // #define AS_EC_Sensor true     // Set this value to if the EC sensor has an enable pin or not
    // #define AS_pH_Sensor true     // Set this value to if the pH sensor has an enable pin or not
    // #define DF_TB_Sensor
    // #define DF_Temp_Sensor
    #define DHT22_Sensor

    // Temperature and salinity compensation defines
    // https://files.atlas-scientific.com/pH_EZO_Datasheet.pdf#page=31
    #define Temperature_Comp 0
    // #define Temperature_Comp_Index 0
    #define Salinity_Comp 1
    // #define Salinity_Comp_Index 0

    /*---------Extra Parameters---------*/
    #define Data_Type SD_Type                   // Data storage type being used
    #define Post_Time 3600000                    // Time interval that the gateway will post at
    #define GPS_Time 60000                      // Time this node will try the GPS every time it takes a measurement
    #define LoRa_Read_Timeout 10000             // Time that it will try to read from LoRa before giving up
    #define cell3G_Make Tinysine                // The make of the 3G module being used
    #define SD_CS 10                            // The pin that cs on the sd card module is connected to

    #define ENABLE_RTC                          // Enable Reading time from RTC Module, you need to setup correct time before using
    
    // #define ENABLE_TPL5110                   // A timer circuit turns arduino to very low-power device and wake up every 100ms up to once every two hours
    //     #define TPL5110_DONE 23              // Send done signal to TPL5110

    // #define DEMO_MODE                        // 3G Shield always enable, sensor light on

    #define DEBUG                                   // Put code in debug mode
    // #define DEBUG_NO_GPS                            // Disable GPS
    //     #define DEBUG_NO_GPS_DEFAULT_LAT 43.000000
    //     #define DEBUG_NO_GPS_DEFAULT_LON -80.000000
    //     #define DEBUG_NO_GPS_DEFAULT_TIME 1665087790
    // #define DEBUG_GetewayData                       // Put GatewayData library in debug mode


/*-----------------------------------------------------------*/
/*-----------------------Turtle_Tracker----------------------*/
/*-----------------------------------------------------------*/
#elif Node_Type == Turtle_Tracker

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
    #define UBLOX_PROTOCOL_AVAILABLE                        // Uncomment if The GPS Module uses the UBLOX Protocol
    #define RTC_AVAILABLE                                   // Uncomment if the GPS module is capable of retaining RTC data (i.e. is has a Battery Backup Pin)

    /*--------------------------------------PIN DECLARATION SETUP-------------------------------*/
    // Software Serial baud rate must be very large
    // Hardware Serial cannot be larger than Software Serial
    // AltSoftwaSerial must be 10x smaller than Software Serial
    // Avoidi using the same buadrate for different Serial communications

    // Pin declarations for GPS Module
    // GPS default baud rate is 9600 -> NeoSWSerial;  Supported baud rates are 9600 (default), 19200 and 38400.
    #define GPS_RX 3        
    #define GPS_TX 4
    #define GPS_VBAT_PIN 5                                  // Pin used to power the GPS to retain RTC and position data
    #define GPS_ENABLE_PIN 6                                // Pin used to power on/off GPS (Low = Disable, High = Enable)

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

    #define WAKE_TIME_HOUR 14                                   // Hour of day to end the long sleep
    #define WAKE_TIME_MIN 11                                    // Minute of the day to end long sleep
    #define NUM_FIXES_PER_DAY 3                                 // Number of times the GPS should get a fix before entering Long Sleep again

    #define CONSTANT_SLEEP 60                                   // IF no RTC, have Arduino sleep periodically for 60min
    #define SHORT_SLEEP_TIME 2                                  // Set the time the Arduino will sleep for during the short sleep interval, in minutes - Max value of 60

    #define SLEEP_THRESHOLD 30                                  /* The Arduino will continuosly loop between sleeping, waking up, correcting
                                                                 the sleep time, and sleeping again until the sleep time is below this value.
                                                                 Once below this value, the Arduino will sleep once more, wake-up, and enter the GPS cycle.
                                                                 A larger value will introduce a larger error from the watchdog, causing the fix times to be
                                                                 more off the target fix time
                                                                 (in seconds)
                                                                */

    #define NO_SLEEP 0                                          // Indicates no additional sleep is required
    #define SHORT_SLEEP 1                                       // Indicates another short sleep is required
    #define LONG_SLEEP 2                                        // Indiactes another long sleep is required
    #define WATCHDOG_CORRECTION 0.15                            // Value that the computed long sleep time is multiplied & subtracted by to account for the 
                                                                // Watchdog timer error and prevent overshooting the target wakeup time

    /*--------------------------------------GPS CONFIGURATION----------------------------------------*/

    #define GPS_FIX 40000                                   // maximum time spent getting GPS fix (in milliseconds)
    #define GPS_LOOP_TIME 2000                              // Min. time spent in GPSLoop() to ensure fix is updated at least once
    #define GPS_PRERUN 5000                                 // Amount of time to let the GPS initially run. Depending on the data collected, either shut down the GSP or continue letting
                                                            // it run (since it may be underwater)


    # define GPS_TIME_ADJUSTMENT 946684800                  // Amount of seconds to change UNIX epoch time from 2000 to 1970
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

    // Enable MON-HW (Hardware Status)
    const uint8_t MON_HW[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x0A, 0x09, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x23, 0x37 };
    // Enable MON-TXBUF (TX Buffer)
    const uint8_t MON_TXBUF[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x0A, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x22, 0x30 };

    // Enable NAV-AOPSTATUS (AssistNow Autonomous Status)
    const uint8_t NAV_AOPSTATUS[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x60, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x71, 0x50 };
    // Enable NAV-DGPS (DGPS Data)
    const uint8_t NAV_DGPS[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x31, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42, 0x07 };
    // Enable NAV-DOP (Dilution of Precision)
    const uint8_t NAV_DOP[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x15, 0xCC };
    // Enable NAV-SOL (Navigation Solution)
    const uint8_t NAV_SOL[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x17, 0xDA };
    // Enable NAV-STATUS (Navigation Status)
    const uint8_t NAV_STATUS[] PROGMEM = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x14, 0xC5 };


    /*________GPS Configurtion_________*/
    // CFG-NAV5 - Configure NAV5
    const uint8_t CFG_NAV5[] PROGMEM = { 0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4F, 0x82 };

    // CFG-NAVX5 - Configure NAVX5
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
    //#define TX_POWER 23                                       // Output power of the RFM95 (23 is the max)
    //#define FREQUENCY 915.0
    //#define PREAMBLE_LENGTH 550                               // Preamble length of LoRa message, in bytes
    //
    //// Timeout values for various blocking functions included in the Mesh Network Protocol
    //#define ACK_SEND_TIMEOUT 10000                            // Timeout for waiting for ACK
    //#define NODE_ACK_TIMEOUT 5000                         // Timeout for Node waitig to receive ACK from Gateway
    //#define RELAY_TIMEOUT 10000                               // Timeout for Relay/Gateway to recieve data and sed ACK
    //#define MESH_ROUTE_TIMEOUT 7000                           // Timeout for aquiring a route
    //
    //
    //#define GATEWAY_ACK_MESSAGE_LENGTH 1                  // TODO
    //#define GATEWAY_ACK_MESSAGE 1                         // TODO
    //
    //// Various length of values used in the payload
    //#define RH_PAYLOAD_LEN 30                             // Length of the payload sent via LoRa
    //
    //// ID's for the Network
    //#define MAX_NETWROK_SIZE 10                               // Totoal number of End-Nodes, Relayer's, and Gateway's in the network
    //#define NODE_ID 1                                     // TODO
    //#define RELAY_ID 2                                        // TODO
    //#define GATEWAY_ID 3                                  // TODO
    //
    //#define DATA_TRANSFER_MESSAGE "      44444"

    /*--------------------------------------LORA CONFIURATION------------------------------------*/

    // Pin declarations for RFM95 Module
    #define RFM95_SLAVE 10
    #define RFM95_RESET 9
    #define RFM95_INTERUPT 2

    // Module Settings
    #define TX_POWER 23                                     // Output power of the RFM95 (23 is the max)
    #define FREQUENCY 915.0
    #define PREAMBLE_LENGTH 550                             // Preamble length of LoRa message, in bytes

    #define CAD_SLEEP 300

    // # of Nodes in the network and ID's for the Network
    #define MAX_NUM_END_NODES 1
    #define NODE_ID 1
    #define RELAY_ID 2
    //#define GATEWAY_ID 3

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

#else
    #error Please set Node_Type to either Sensor_End_Node, Gateway, or Turtle_Tracker
#endif

// Some defines used for internal comparisons
#define EEPROM_Type 0
#define SD_Type 1

#define Tinysine 0
#define Adafruit 1

#endif // remoteConfig_h