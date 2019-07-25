/*
Header for some config parameters, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated July 25, 2019
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

#if defined(End_Node) && defined(Gateway)
    #error Please set compilation to either End_Node or Gateway mode
#endif

/*---------Overall Network Settings---------*/
#define GATEWAY_ID 0x1000               //LoRa ID of the Gateway Node
#define NETWORK_ID 0x01                 //LoRa Network ID

#define URL_Host "www.cas.mcmaster.ca"  //Host of the URL to post data to
#define URL_Path "/ollie"               //Path after the URL to post data to
#define URL_Port 80                     //Port of the URL to open an HTTPS session with


/*---------Type specific Settings---------*/

#ifdef End_Node
    /*---------End Node settings---------*/

    /*---------Base Parameters---------*/
    #define NAME "Office"           //Name of this end node
    #define NODE_ID 0x0000          //ID of this end node
    #define LORA_RX 7               //Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
    #define LORA_TX 6               //Pin that the LoRa RXD pin is connected to
    #define GPS_RX 2                //Pin that the GPS TXD pin is connected to
    #define GPS_TX 3                //Pin that the GPS RXD pin is connected to
    #define GPS_EN 10               //Pin that the GPS EN pin is connected to

    /*---------Sensor definitions---------*/
    #define NUMBER_SENSOR_NAMES 3
    #define NUMBER_SENSOR_TYPES 3

    extern const char* sensorNames[NUMBER_SENSOR_NAMES];
    extern const char* sensorTypes[NUMBER_SENSOR_TYPES];
    extern const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][2];
    #ifdef MAIN
        const char* sensorNames[NUMBER_SENSOR_NAMES] = {"Dissolved_Oxygen", /*"Conductivity",*/ "Turbidity", "Water_Temperature"};
        const char* sensorTypes[NUMBER_SENSOR_TYPES] = {"AS_DO", /*"AS_EC",*/ "DF_TB", "DF_Temp"};
        const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][2] = { {9, 8}, /*{12, 11},*/ {5, 20}, {4, 0} };
    #endif

    #define AS_DO_Sensor
    //#define AS_EC_Sensor false      //Set this value to if the EC sensor has an enable pin or not
    #define DF_TB_Sensor
    #define DF_Temp_Sensor
    //#define DHT22_Sensor

    /*---------Extra Parameters---------*/
    #define Data_Type EEPROM_Type                   //Type of data storage being used
    #define GPS_Time 60000                          //Time this node will try the GPS every time it takes a measurement
    #define LoRa_Read_Timeout 5000                  //Time that it will try to read from LoRa before giving up
    #define Sleep_Time 120000                       //Time that the node sleeps on success
    #define Fail_Sleep_Time (Sleep_Time * 3) / 4    //Time that this node sleeps on fail
    #define Sleep_Type SLEEP_MODE_PWR_DOWN          //Type of sleep this node uses

    #define DEBUG                   //Put code in debug mode

#endif

#ifdef Gateway
    /*---------Gateway settings---------*/

    /*---------Base Parameters---------*/
    #define NAME "Gateway"          //Name of this gateway
    #define LORA_RX 6               //Pin that the LoRa TXD pin is connected to (it's opposite because the output of the LoRa module is the input into the Arduino, and vice-versa)
    #define LORA_TX 7               //Pin that the LoRa RXD pin is connected to
    #define FONA_RX 4               //Pin that the FONA TX pin is connected to
    #define FONA_TX 5               //Pin that the FONA RX pin is connected to
    #define FONA_EN 8               //Pin that the FONA KEY pin is connected to

    /*---------Sensor definitions---------*/
    #define NUMBER_SENSOR_NAMES 2
    #define NUMBER_SENSOR_TYPES 1

    extern const char* sensorNames[NUMBER_SENSOR_NAMES];
    extern const char* sensorTypes[NUMBER_SENSOR_TYPES];
    extern const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][2];
    #ifdef MAIN
        const char* sensorNames[NUMBER_SENSOR_NAMES] = {"Air_Temperature", "Humidity"};
        const char* sensorTypes[NUMBER_SENSOR_TYPES] = {"DHT22"};
        const uint8_t sensorPorts[NUMBER_SENSOR_TYPES][2]= { {2, 0} };
    #endif
    
    //#define AS_DO_Sensor
    //#define AS_EC_Sensor false      //Set this value to if the EC sensor has an enable pin or not
    //#define DF_TB_Sensor
    //#define DF_Temp_Sensor
    #define DHT22_Sensor

    /*---------Extra Parameters---------*/
    #define Data_Type SD_Type                   //Data storage type being used
    #define Post_Time 120000                    //Time interval that the gateway will post at
    #define GPS_Time 60000                      //Time this node will try the GPS every time it takes a measurement
    #define LoRa_Read_Timeout 5000              //Time that it will try to read from LoRa before giving up
    #define Sleep_Time 1000                     //Time that the Gateway will sleep for between reading LoRa messages
    #define Sleep_Type SLEEP_MODE_PWR_DOWN      //Type of sleep the Gateway uses
#endif

//Some defines used for internal comparisons
#define EEPROM_Type 0
#define SD_Type 1

#endif