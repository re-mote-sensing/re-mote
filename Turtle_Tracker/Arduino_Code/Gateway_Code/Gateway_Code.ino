/*
  Turtle Tracker Arduino code used in the re-mote setup found at 
  https://gitlab.cas.mcmaster.ca/re-mote
*/

/* ------------------------ Config ------------------------- */

#define GATEWAY_ID 0x64 // Unique Gateway ID, 100(0x64) <= GATEWAY_ID <= 200(0xc8)

// Sever Info
#define HOST "turtletracker.cas.mcmaster.ca"  // Change this to the public IP address / domain of the server's network
#define ENDPOINT "/data"                      // Server HTTP POST endpoint
#define SERVER_PORT 80                        // Server port number

// Length of each line in ToSend.bin: 
// Type,NodeID,TimeStamp,Lat,Lon
// \r\n0d065d1135ca15cd5b07b168de3a
#define BIN_LINE_LENGTH 40

// HTTP Post
#define POST_AMOUNT 4       // Define how many tracker data to read for a single post (Based on testing, 4 is a stable amount for Uno)
#define POST_TIME 2700000     // Time between each post (ms), 2700000ms ~45min
#define GATEWAY_REPORT_TIME 3600000 // Time between each gateway report, 3600000 ~a hour

// SD Card
// Hardware SPI is used by LoRa, so software SPI with different pins is required
#define SD_CS_PIN A5          // SD Card CS Pin
#define SD_SOFT_MISO_PIN A2   // SD Card MISO Pin
#define SD_SOFT_MOSI_PIN A3   // SD Card MOSI Pin
#define SD_SOFT_SCK_PIN A4    // SD Card SCK Pin

// Tinysine 3G Shield
// Two pins conflict with LoRa Shield
// Connect 8 => A1
//         9 => A0
// Config Bandrate to 9600 under "STEP2: 3G Baud Adjustment"
// https://www.tinyosshop.com/index.php?route=information/news&news_id=51
#define SIM3G_EN A1                   // 3G Enable Pin
#define SIM3G_TX 4                    // 3G TX Pin
#define SIM3G_RX 5                    // 3G RX Pin

// 3G Enable Pin Time
// https://www.tinyosshop.com/datasheet/3G%20Shield%20Datasheet.pdf
#define SIM3G_POWER_ON_TIME 180       // 3G Power on time for enable pin (ms)
#define SIM3G_POWER_OFF_TIME 4000     // 3G Power off time for enable pin (ms)

// 3G Timeout
#define SIM3G_HTTP_TIMEOUT 60000       // 3G http timeout (ms)

// Message encoding
// refer to message encoding in this file:
// https://github.com/re-mote-sensing/re-mote/blob/master/Turtle_Tracker/Docs/message_format_turtle_tracker.xlsx
#define REG_LEN 4 // registration message length (in bytes)
#define SEN_LEN 14 // sensor data message length (in bytes)
#define SEN_HEADER_LEN 6 // SEN_BODY_LEN = SEN_LEN - SEN_HEADER_LEN (in bytes)
#define ACK_LEN 3 // ack messgae length (in bytes)
#define DATA_POINT_NUM 19 // the maximum num of data points that can be stored in lora buff
#define DATA_POINT_SIZE 13 // size of one data points (in bytes)
#define REG_TYPE 0x0c // registration type in hex
#define SEN_TYPE 0x0d // sensor data type in hex
#define ACK_SUCCESS 0x00 // first byte in successful ack message

// LoRa
#define LORA_TX_POWER 20      // Output power of the RFM95
#define INVERT_IQ_MODE false  // InvertIQ mode
#define INTERUPT_MODE false   // Use interupt pin when receive a LoRa Message (Arduino Uno does not have enough SRAM)
#define LORA_FREQ 915E6       // Legal frequency in Canada

// Serial
#define PC_BAUDRATE 9600L     // Debug Serial Baudrate
#define SIM3G_BAUDRATE 9600L  // 3G Shield Serial Baudrate

// Debug
#define DEBUG false  // Set to true for debug Serial output, false for no output
#define DEBUG_SERIAL if(DEBUG) Serial // Serial for debuging

/* ------------------------ Librarys ------------------------- */

// LoRa Library
// An Arduino library for sending and receiving data using LoRa radios.
// https://www.arduino.cc/reference/en/libraries/lora/
#include <LoRa.h>

// NeoSWSerial Library
// An efficient replacement for SoftwareSerial, Simultaneous RX & TX
// https://www.arduino.cc/reference/en/libraries/neoswserial/
#include <NeoSWSerial.h>

// SdFat Library
// Provides access to SD memory cards. 
// SPI_DRIVER_SELECT must be 2 in SdFat/SdFatConfig.h
// https://www.arduino.cc/reference/en/libraries/sdfat/
#include "SdFat.h"
#if SPI_DRIVER_SELECT != 2
#error SPI_DRIVER_SELECT must be 2 in SdFat/SdFatConfig.h
#endif  //SPI_DRIVER_SELECT

// MemoryFree Library
// Report memory usage for SRAM debuging
// https://github.com/maniacbug/MemoryFree
#if DEBUG
#include <MemoryFree.h>
#endif

/* ------------------------ Constructors ------------------------- */

NeoSWSerial ss(SIM3G_TX, SIM3G_RX);   // Serial for 3G Shield
// SdFat software SPI
// https://github.com/greiman/SdFat/blob/master/examples/SoftwareSpi/SoftwareSpi.ino
SoftSpiDriver<SD_SOFT_MISO_PIN, SD_SOFT_MOSI_PIN, SD_SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)

// Length of variable in postToServerWithBuffer()
const unsigned int CHTTPSOPSE_LENGTH = 22 + sizeof(SERVER_PORT) + sizeof(HOST); // Command:22 + $SERVER_PORT:3 + $HOST:29
const unsigned int msgBody_LENGTH = 2 + (BIN_LINE_LENGTH-2)*POST_AMOUNT; // Command:2 + ($BIN_LINE_LENGTH-2)*$POST_AMOUNT:6
const unsigned int POST_COMMAND_LENGTH = 33 + sizeof(HOST) + sizeof(ENDPOINT) + msgBody_LENGTH; // Command:33 + $HOST:29 + $ENDPOINT:5 + $msgBody_LENGTH

/* ------------------------ Global Variables ------------------------- */

unsigned long lastPost = millis();  // Track what is the last posted time
unsigned long lastReport = millis();  // Track what is the last gateway report time
bool readyToPost = true;            // Ready to post to server
uint8_t trackerSleepCycles = 28;   // Tracker sleep cycle remote control, 28 for ~30min
uint16_t serialNum = 0; // count how many messages has been sent.

/* ------------------------ Setup ------------------------- */

void setup() {
  DEBUG_SERIAL.begin(PC_BAUDRATE); // Start Debug Serial

  // Reset the 3G shield then turn it off
  pinMode(SIM3G_EN, OUTPUT);
  powerOn3G();
  delay(8000);  // Delay will ensure the 3G is fully turned on
  powerOff3G();
  
  // Start 3G Serial
  ss.begin(SIM3G_BAUDRATE);
  DEBUG_SERIAL.println(F("3G Ok"));

  // Start SD and create ToSend.bin
  createToSendFile(); 

  // Register gateway itself
  registerTracker((uint8_t)GATEWAY_ID);

  // Start LoRa
  if (!LoRa.begin(LORA_FREQ)) {
    DEBUG_SERIAL.println(F("LoRa init failed. Check your connections."));
    while (true);
  }
  LoRa.setTxPower(LORA_TX_POWER); // maximum tx power to get longest range
  //LoRa.setSpreadingFactor(12); // maximum SF to get longest range
  LoRa.enableCrc(); // Enables the LoRa module's built in error checking
  DEBUG_SERIAL.println(F("LoRa init succeeded."));
  
  #if INTERUPT_MODE == true
  // Enable the following three line to enable onReceive with interupt pins DIO0(D2)
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
  #endif
  
  DEBUG_SERIAL.println(F("Gateway initialized successfully!"));
}

/* ------------------------ Loop ------------------------- */

void loop() {
  // Post to server every POST_TIME ms
  if (readyToPost && (millis() - lastPost) > POST_TIME) {
    if (postToServerWithBuffer()){ // Post data under ToSend.bin
      resetToSend(); // Reset ToSend.bin
    }else{
      DEBUG_SERIAL.println(F("No data to post"));
    }
    lastPost = millis(); // Reset lastPost to current time
    readyToPost = false;
  }

  // Report gateway itself with serial number to check if gateway is working
  // (GPS time and coords has not been impenment yet)
  if ((millis() - lastReport) > GATEWAY_REPORT_TIME){ // Every GATEWAY_REPORT_TIME ms
    reportGateway();
    lastReport = millis(); // Reset lastReport to current time
    readyToPost = true;
  }

  #if INTERUPT_MODE == false
  int packetSize = LoRa.parsePacket();
  String result;
  if (packetSize) {
    onReceive(packetSize);
  }
  #endif
}

/* ------------------------ Callback Functions ------------------------- */

// If LoRa received any message, it will interrupt any current execution line,
// then begin call this onReceive function
void onReceive(int packetSize) {
  // filter by size
  if (packetSize != REG_LEN && (packetSize - SEN_HEADER_LEN) % DATA_POINT_SIZE != 0) {
    return;
  }
  
  DEBUG_SERIAL.print(F("Size: "));
  DEBUG_SERIAL.println(packetSize);
  
  // read type and sensor#
  uint8_t typeAndSensor = (uint8_t) LoRa.read();
  uint8_t type = typeAndSensor >> 4;

  #if DEBUG
  DEBUG_SERIAL.print("Type: ");
  printByte(type);
  DEBUG_SERIAL.println();
  #endif
  
  // do action according to message type
  switch (type) {
    // <--- CASE: Registration ---> //
    case REG_TYPE: {
      // read nodeID
      uint8_t nodeID = (uint8_t) LoRa.read();
      // read serial number (16 bits = 2 bytes)
      uint8_t serialNum[2];
      serialNum[0] = (uint8_t) LoRa.read();
      serialNum[1] = (uint8_t) LoRa.read();
      // read data debug
      #if DEBUG
      DEBUG_SERIAL.print(F("nodeID: "));
      printByte(nodeID);
      DEBUG_SERIAL.println();
      DEBUG_SERIAL.print(F("serialNum: "));
      printByte(serialNum[0]);
      printByte(serialNum[1]);
      DEBUG_SERIAL.println();
      #endif
      // covert data to string
      DEBUG_SERIAL.print(F("nodeID: "));
      DEBUG_SERIAL.println(String(nodeID));
      DEBUG_SERIAL.print(F("serialNum: "));
      DEBUG_SERIAL.println(String(* (uint16_t*) serialNum)); 
      // register
      registerTracker(nodeID);
      // send ACK
      sendAck(nodeID);     
      break;
    }
      
    // <--- CASE: Sensor Data ---> //
    case SEN_TYPE: {
      // read nodeID
      uint8_t nodeID = (uint8_t) LoRa.read();
      // read serial number (16 bits = 2 bytes)
      uint8_t serialNum[2];
      serialNum[0] = (uint8_t) LoRa.read();
      serialNum[1] = (uint8_t) LoRa.read();
      // read battery percentage
      uint8_t batteryPercentage = (uint8_t) LoRa.read();
      // read datapoint count
      uint8_t datapointCount = (uint8_t) LoRa.read();
      
      for (uint8_t i = 0; i < datapointCount; i++) {
        // read unixTime
        uint8_t unixTime[4];
        unixTime[0] = (uint8_t) LoRa.read();
        unixTime[1] = (uint8_t) LoRa.read();
        unixTime[2] = (uint8_t) LoRa.read();
        unixTime[3] = (uint8_t) LoRa.read();
        // read latitude
        uint8_t latitude[4];
        latitude[0] = (uint8_t) LoRa.read();
        latitude[1] = (uint8_t) LoRa.read();
        latitude[2] = (uint8_t) LoRa.read();
        latitude[3] = (uint8_t) LoRa.read();
        // read longitude
        uint8_t longitude[4];
        longitude[0] = (uint8_t) LoRa.read();
        longitude[1] = (uint8_t) LoRa.read();
        longitude[2] = (uint8_t) LoRa.read();
        longitude[3] = (uint8_t) LoRa.read();
        // read tempurature
        uint8_t tempurature = (uint8_t) LoRa.read();

        // add data into buffer
        addDataByte(
                type,
                nodeID, 
                serialNum,
                batteryPercentage,
                datapointCount,
                unixTime,
                latitude,
                longitude,
                tempurature
                );

        // read data debug
        #if DEBUG
        DEBUG_SERIAL.print(F("nodeID: "));
        printByte(nodeID);
        DEBUG_SERIAL.println();
        DEBUG_SERIAL.print(F("unixTime: "));
        printByte(unixTime[0]);
        printByte(unixTime[1]);
        printByte(unixTime[2]);
        printByte(unixTime[3]);
        DEBUG_SERIAL.println();
        DEBUG_SERIAL.print(F("latitude: "));
        printByte(latitude[0]);
        printByte(latitude[1]);
        printByte(latitude[2]);
        printByte(latitude[3]);
        DEBUG_SERIAL.println();
        DEBUG_SERIAL.print(F("longitude: "));
        printByte(longitude[0]);
        printByte(longitude[1]);
        printByte(longitude[2]);
        printByte(longitude[3]);
        DEBUG_SERIAL.println();
        DEBUG_SERIAL.print(F("tempurature: "));
        printByte(tempurature);
        DEBUG_SERIAL.println();
        // covert data to string
        DEBUG_SERIAL.print(F("nodeID: "));
        DEBUG_SERIAL.println(String(nodeID));
        DEBUG_SERIAL.print(F("unixTime: "));
        DEBUG_SERIAL.print(F("serialNum: "));
        DEBUG_SERIAL.println(String(* (uint16_t*) serialNum));
        DEBUG_SERIAL.print(F("batteryPercentage: "));
        DEBUG_SERIAL.println(String(batteryPercentage));
        DEBUG_SERIAL.print(F("datapointCount: "));
        DEBUG_SERIAL.println(String(datapointCount));
        DEBUG_SERIAL.println(String(* (unsigned long*) unixTime));
        DEBUG_SERIAL.print(F("latitude: "));
        DEBUG_SERIAL.println(String(* (long*) latitude));
        DEBUG_SERIAL.print(F("longitude: "));
        DEBUG_SERIAL.println(String(* (long*) longitude));
        DEBUG_SERIAL.print(F("tempurature: "));
        DEBUG_SERIAL.println(String(tempurature));
        #endif
      }       
      // send ACK
      sendAck(nodeID);
      readyToPost = true; // Ready to post to server
      break;
    }    
  }
}

// If LoRa finish transmitted message, it will interrupt any current execution line,
// then begin call this onReceive function
void onTxDone() {
  DEBUG_SERIAL.println(F("TxDone"));
  LoRa_rxMode();
}
