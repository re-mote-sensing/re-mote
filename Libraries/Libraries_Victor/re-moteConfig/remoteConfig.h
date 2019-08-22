/*
Header for some config parameters, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated August 12, 2019
Released into the public domain
*/

/*-----------------------------------------------------------------*/
//Look at (LINK!) for information on how to configure this correctly
/*-----------------------------------------------------------------*/

#ifndef remoteConfig_h
#define remoteConfig_h

/*---------Type of node---------*/
//#define End_Node
#define Gateway

#if (defined(End_Node) && defined(Gateway)) || (!defined(End_Node) && !defined(Gateway))
    #error Please set compilation to either End_Node or Gateway mode
#endif

/*---------Overall Network Settings---------*/
#define GATEWAY_ID 0x3000               //LoRa ID of the Gateway Node
#define NETWORK_ID 0x02                 //LoRa Network ID

#define URL_Host "www.cas.mcmaster.ca"  //Host of the URL to post data to
#define URL_Path "/ollie"               //Path after the URL to post data to
#define URL_Port 80                     //Port of the URL to open an HTTPS session with


/*---------Type specific Settings---------*/

#ifdef End_Node
    /*---------End Node settings---------*/

    /*---------Base Parameters---------*/
    #define NAME "Office"           //Name of this end node
    #define NODE_ID 0x0003          //ID of this end node
    #define LORA_RX 7               //Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
    #define LORA_TX 6               //Pin that the LoRa RXD pin is connected to
    #define GPS_RX 2                //Pin that the GPS TXD pin is connected to
    #define GPS_TX 3                //Pin that the GPS RXD pin is connected to
    #define GPS_EN 10               //Pin that the GPS EN pin is connected to

    /*---------Sensor definitions---------*/
    #define NUMBER_SENSOR_NAMES 4   //The number of sensor names on the end node
    #define NUMBER_SENSOR_TYPES 4   //The number of sensor types on the end node
    #define MAX_NUMBER_PINS 3       //The max number of pins that one sensor uses

    extern const char* sensorNames[NUMBER_SENSOR_NAMES];
    extern const char* sensorTypes[NUMBER_SENSOR_TYPES];
    extern const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][MAX_NUMBER_PINS];
    #ifdef MAIN

        const char* sensorNames[NUMBER_SENSOR_NAMES] = {"Water_Temperature", "Conductivity", "Dissolved_Oxygen", "Turbidity"};
        const char* sensorTypes[NUMBER_SENSOR_TYPES] = {"DF_Temp", "AS_EC", "AS_DO", "DF_TB"};
        const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][MAX_NUMBER_PINS] = { {4}, {14, 15, 13}, {9, 8}, {5, 20} };
/*
        const char* sensorNames[NUMBER_SENSOR_NAMES] = {"Water_Temperature", "pH", "Dissolved_Oxygen", "Turbidity"};
        const char* sensorTypes[NUMBER_SENSOR_TYPES] = {"DF_Temp", "AS_pH", "AS_DO", "DF_TB"};
        const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][MAX_NUMBER_PINS] = { {4}, {14, 15, 13}, {9, 8}, {5, 20} };
*/
    #endif

    //All the sensor types that are used in this end node
    #define AS_DO_Sensor
    #define AS_EC_Sensor true       //Set this value to if the EC sensor has an enable pin or not
    //#define AS_pH_Sensor true     //Set this value to if the pH sensor has an enable pin or not
    #define DF_TB_Sensor
    #define DF_Temp_Sensor
    //#define DHT22_Sensor

    //Temperature and salinity compensation defines
    #define Temperature_Comp 0
    //#define Temperature_Comp_Index 0
    #define Salinity_Comp 1
    //#define Salinity_Comp_Index 0

    /*---------Extra Parameters---------*/
    #define Data_Type EEPROM_Type                   //Type of data storage being used
    #define GPS_Time 60000                          //Time this node will try the GPS every time it takes a measurement
    #define LoRa_Read_Timeout 10000                 //Time that it will try to read from LoRa before giving up
    #define Sleep_Time 240000                       //Time that the node sleeps on success
    #define Fail_Sleep_Time (Sleep_Time * 3) / 4    //Time that this node sleeps on fail
    #define Sleep_Type SLEEP_MODE_PWR_DOWN          //Type of sleep this node uses

    #define DEBUG                                   //Put code in debug mode
#endif

#ifdef Gateway
    /*---------Gateway settings---------*/

    /*---------Base Parameters---------*/
    #define NAME "Office_Gateway"   //Name of this gateway
    #define LORA_RX 6               //Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
    #define LORA_TX 7               //Pin that the LoRa RXD pin is connected to
    #define cell3G_RX 4             //Pin that the 3G module TX pin is connected to
    #define cell3G_TX 5             //Pin that the 3G module RX pin is connected to
    #define cell3G_EN 8             //Pin that the 3G module KEY pin is connected to

    /*---------Sensor definitions---------*/
    #define NUMBER_SENSOR_NAMES 0   //The number of sensor names on the gateway
    #define NUMBER_SENSOR_TYPES 0   //The number of sensor types on the gateway
    #define MAX_NUMBER_PINS 0       //The max number of pins that one sensor uses

    extern const char* sensorNames[NUMBER_SENSOR_NAMES];
    extern const char* sensorTypes[NUMBER_SENSOR_TYPES];
    extern const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][MAX_NUMBER_PINS];
    #ifdef MAIN
        const char* sensorNames[NUMBER_SENSOR_NAMES] = {/*"Air_Temperature", "Humidity"*/};
        const char* sensorTypes[NUMBER_SENSOR_TYPES] = {/*"DHT22"*/};
        const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][MAX_NUMBER_PINS]= {/* {2} */};
        
    #endif

    //All the sensor types that are used in this end node
    //#define AS_DO_Sensor
    //#define AS_EC_Sensor false      //Set this value to if the EC sensor has an enable pin or not
    //#define AS_pH_Sensor true     //Set this value to if the pH sensor has an enable pin or not
    //#define DF_TB_Sensor
    //#define DF_Temp_Sensor
    //#define DHT22_Sensor

    /*---------Extra Parameters---------*/
    #define Data_Type SD_Type                   //Data storage type being used
    #define Post_Time 240000                    //Time interval that the gateway will post at
    #define GPS_Time 60000                      //Time this node will try the GPS every time it takes a measurement
    #define LoRa_Read_Timeout 10000             //Time that it will try to read from LoRa before giving up
    #define cell3G_Make Tinysine                //The make of the 3G module being used
    #define SD_CS 10                            //The pin that cs on the sd card module is connected to


    //#define DEBUG                   //Put code in debug mode
#endif

//Some defines used for internal comparisons
#define EEPROM_Type 0
#define SD_Type 1

#define Tinysine 0
#define Adafruit 1

#endif