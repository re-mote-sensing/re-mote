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
#define Node_Type Sensor_End_Node       // Either Sensor_End_Node, Gateway, or Turtle_Tracker


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

#else
    #error Please set Node_Type to either Sensor_End_Node, Gateway, or Turtle_Tracker
#endif

// Some defines used for internal comparisons
#define EEPROM_Type 0
#define SD_Type 1

#define Tinysine 0
#define Adafruit 1

#endif // remoteConfig_h