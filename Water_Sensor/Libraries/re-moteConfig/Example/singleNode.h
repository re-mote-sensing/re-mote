/*
This is an example config file with the following configuration

A single node with 3G and sensors (No Gateway needed)
 - WaterTemperature
    - Connect to PIN 26(data)
 - pH
    - Connect to PIN A8(tx), 45(rx), 44(off)
 - Dissolved_Oxygen
    - Connect to PIN A9(tx), 47(rx), 46(off)
 - Turbidity
    - Connect to PIN A14(vcc), A15(data)
 - Conductivity
    - Connect to PIN A11(tx), 49(rx), 48(off)
 - DHT22 (AirTemperature & Humidity)
    - Connect to PIN 25(data)

Author: Tianyu Zhou
Last updated: Mar 14, 2023
*/

#ifndef remoteConfig_h
#define remoteConfig_h

// Defines used for internal comparisons, ignore these
#define Sensor_End_Node 0
#define Gateway 1
#define Turtle_Tracker 2

/*---------Type of node---------*/
#define Node_Type Gateway       // Either Sensor_End_Node, Gateway, or Turtle_Tracker


/*---------Overall Network Settings---------*/
#define GATEWAY_ID 0x02                         // LoRa ID of the Gateway Node

#define URL_Host "macwater.cas.mcmaster.ca"     // Host of the URL to post data to
#define URL_Path ""                             // Path after the URL to post data to
#define URL_Port 80                             // Port of the URL to open an HTTPS session with
#define API_KEY "remote"                        // Server api key - Default is 'remote'

/*-----------------------------------------------------------*/
/*--------------------------Gateway--------------------------*/
/*-----------------------------------------------------------*/
#if Node_Type == Gateway
    /*---------Gateway settings---------*/

    /*---------Base Parameters---------*/
    #define NAME "Node_2"             // Name of this gateway
    // #define ENABLE_LORA            // Comment out to disable the LoRa module
        // #define LORA_RX 6          // Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
        // #define LORA_TX 7          // Pin that the LoRa RXD pin is connected to
    #define cell3G_RX 10              // Pin that the 3G module TX pin is connected to
    #define cell3G_TX 11              // Pin that the 3G module RX pin is connected to
    #define cell3G_EN 8               // Pin that the 3G module KEY pin is connected to

    /*---------Sensor definitions---------*/
    #define NUMBER_SENSOR_NAMES 7   // The number of sensor names on the gateway
    #define NUMBER_SENSOR_TYPES 6   // The number of sensor types on the gateway
    #define MAX_NUMBER_PINS 3       // The max number of pins that one sensor uses

    extern const char* sensorNames[NUMBER_SENSOR_NAMES];
    extern const char* sensorTypes[NUMBER_SENSOR_TYPES];
    extern const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][MAX_NUMBER_PINS];
    #ifdef MAIN

        const char* sensorNames[NUMBER_SENSOR_NAMES] = {"Water_Temperature", "pH", "Dissolved_Oxygen", "Turbidity", "Conductivity", "Air_Temperature", "Humidity"};
        const char* sensorTypes[NUMBER_SENSOR_TYPES] = {"DF_Temp", "AS_pH", "AS_DO", "DF_TB", "AS_EC", "DHT22"};
        const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][MAX_NUMBER_PINS] = { {26}, {A8, 45, 44}, {A9, 47, 46}, {A14, A15}, {A11, 49, 48},  {25} };     
    
    #endif

    // All the sensor types that are used in this end node
    #define AS_DO_Sensor true     // Set this value to if the pH sensor has an enable pin or not
    #define AS_EC_Sensor true     // Set this value to if the EC sensor has an enable pin or not
    #define AS_pH_Sensor true     // Set this value to if the pH sensor has an enable pin or not
    #define DF_TB_Sensor
    #define DF_Temp_Sensor
    #define DHT22_Sensor

    // Temperature and salinity compensation defines
    // https://files.atlas-scientific.com/pH_EZO_Datasheet.pdf#page=31
    #define Temperature_Comp 0
    // #define Temperature_Comp_Index 0
    #define Salinity_Comp 1
    // #define Salinity_Comp_Index 0

    /*---------Extra Parameters---------*/
    #define Data_Type SD_Type                   // Data storage type being used
    #define Post_Time 3600000                   // Time interval that the gateway will post at
    #define GPS_Time 60000                      // Time this node will try the GPS every time it takes a measurement
    #define LoRa_Read_Timeout 10000             // Time that it will try to read from LoRa before giving up
    #define cell3G_Make Tinysine                // The make of the 3G module being used
    #define SD_CS 53                            // The pin that cs on the sd card module is connected to

    #define ENABLE_RTC                          // Enable Reading time from RTC Module, you need to setup correct time before using
    
    #define ENABLE_TPL5110                      // A timer circuit turns arduino to very low-power device and wake up every 100ms up to once every two hours
        #define TPL5110_DONE 23                 // Send done signal to TPL5110

    // #define DEMO_MODE                        // 3G Shield always enable, sensor light on

    #define DEBUG                                   // Put code in debug mode
    #define DEBUG_NO_GPS                            // Disable GPS
        #define DEBUG_NO_GPS_DEFAULT_LAT 43.000000
        #define DEBUG_NO_GPS_DEFAULT_LON -80.000000
        #define DEBUG_NO_GPS_DEFAULT_TIME 1665087790
    #define DEBUG_GetewayData                       // Put GatewayData library in debug mode

#else
    #error Please set Node_Type to either Sensor_End_Node, Gateway, or Turtle_Tracker
#endif

// Some defines used for internal comparisons
#define EEPROM_Type 0
#define SD_Type 1

#define Tinysine 0
#define Adafruit 1

#endif // remoteConfig_h