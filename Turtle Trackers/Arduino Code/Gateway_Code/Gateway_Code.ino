/*
  Turtle Tracker Arduino code used in the re-mote setup found at 
  https://gitlab.cas.mcmaster.ca/re-mote
*/

#include <SPI.h>
#include <LoRa.h>
#include <NeoSWSerial.h>
#include "SdFat.h"

/* ------------------------ Config ------------------------- */
#define GATEWAY_ID 0x01 // Unique Gateway ID

// Sever Info
#define host "turtletracker.cas.mcmaster.ca" // Change this to the public IP address / domain of the pi server's network
#define endpoint "/data"  // Server endpoint
#define port 80 // Server port number

// Length of variable in postToServerCSV()
#define CHTTPSOPSE_LENGTH 53 // Command:22 + $Port:2 + $host:29
#define msgBody_LENGTH 55 // Command:5 + $CsvLineLength:50
#define POST_COMMAND_LENGTH 117 // Command:33 + $host:29 + $endpoint:5 + $msgBody_LENGTH:50

// Length of each line in csv: 
// NodeID,Count,TimeStamp,Lat,Lon
// 6,0,1656458252,432585465,-799211845
#define CSV_LINE_LENGTH 50

#define POST_TIME 30000   // Time between each post

// SD Card
#define SD_CS_PIN A5      // SD Card CS Pin
#define SD_SOFT_MISO_PIN A2  // SD Card MISO Pin
#define SD_SOFT_MOSI_PIN A3  // SD Card MOSI Pin
#define SD_SOFT_SCK_PIN A4   // SD Card SCK Pin
// SdFat software SPI
SoftSpiDriver<SD_SOFT_MISO_PIN, SD_SOFT_MOSI_PIN, SD_SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)
#if SPI_DRIVER_SELECT != 2  // Must be set in SdFat/SdFatConfig.h
#error SPI_DRIVER_SELECT must be 2 in SdFat/SdFatConfig.h
#endif  //SPI_DRIVER_SELECT

// Tinysine 3G Shield
#define SIM3G_EN A1                   // 3G Enable Pin
#define SIM3G_TX 4                    // 3G TX Pin
#define SIM3G_RX 5                    // 3G RX Pin
#define SIM3G_POWER_ON_TIME 180       // 3G Power on time for enable pin
#define SIM3G_POWER_OFF_TIME 4000     // 3G Power off time for enable pin
#define SIM3G_HTTP_TIMEOUT 4000       // 3G http timeout
#define SIM3G_POWER_ON_TIMEOUT 20000  // 3G power on timeout

// LoRa
//#define LORA_TX_POWER 23      // Output power of the RFM95 (23 is the max)
#define INVERT_IQ_MODE false  // InvertIQ mode
#define REG_LEN 6             // registration message length (in bytes)
#define SEN_LEN 14            // sensor data message length (in bytes)

// Serial
#define PC_BAUDRATE 9600L     // Debug Serial Baudrate

#define DEBUG true // Set to true for debug output, false for no output

#if DEBUG == true
#include <MemoryFree.h>
#endif

/* ------------------------ Constructors ------------------------- */
#define DEBUG_SERIAL if(DEBUG)Serial
NeoSWSerial ss(SIM3G_TX, SIM3G_RX);

/* ------------------------ Golbal Variables ------------------------- */
unsigned long lastPost = millis();
bool readyToPost = true; // Ready to post to server

/* ------------------------ Setup ------------------------- */
void setup() {
  DEBUG_SERIAL.begin(9600); // Start Debug Serial

  // Power on 3G then turn it off
  pinMode(SIM3G_EN, OUTPUT);
  powerOn3G();
  delay(1000);
  powerOff3G();
  
  // Start 3G Serial
  ss.begin(9600);
  DEBUG_SERIAL.println(F("3G Ok"));

  // Start SD and create ToSend.csv
  createToSendFile(); 

  // Start LoRa
  if (!LoRa.begin(915E6)) {
    DEBUG_SERIAL.println(F("LoRa init failed. Check your connections."));
    while (true);
  }
//  LoRa.setPins(10, 9, 3);
  LoRa.enableCrc(); // Enables the LoRa module's built in error checking
  DEBUG_SERIAL.println(F("LoRa init succeeded."));
  //LoRa.onReceive(onReceive);
  //LoRa.onTxDone(onTxDone);
  //LoRa_rxMode();
  
  // Start 3G Shield HTTP Request
  start3GHTTP();
  
  DEBUG_SERIAL.println(F("Gateway initialized successfully!"));
}

/* ------------------------ Loop ------------------------- */
void loop() {
  // Post to server every POST_TIME ms
  if (readyToPost && (millis() - lastPost) > POST_TIME) {
    if (postToServerCSV()){ // Post data under ToSend.csv
      DEBUG_SERIAL.println(freeMemory());
      resetToSend(); // Reset ToSend.csv
      // createToSendFile(); // Create ToSend.csv
    }else{
      DEBUG_SERIAL.println(freeMemory());
      DEBUG_SERIAL.println(F("No data to post"));
    }
    lastPost = millis();
    readyToPost = false;
  }

  int packetSize = LoRa.parsePacket();
  String result;
  if (packetSize) {
    onReceive(packetSize);
  }
}

/* ------------------------ below are LoRa helper functions ------------------------- */

void LoRa_rxMode(){
  #if INVERT_IQ_MODE
  LoRa.disableInvertIQ();               // normal mode
  #endif
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  #if INVERT_IQ_MODE
  LoRa.enableInvertIQ();                // active invert I and Q signals
  #endif
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void LoRa_sendMessage(const uint8_t* message, size_t messageLen) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.write(message, messageLen);      // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  // filter by size
  if (packetSize != SEN_LEN && packetSize != REG_LEN) {
    return;
  }
  
  DEBUG_SERIAL.print(F("Size: "));
  DEBUG_SERIAL.println(packetSize);
  
  // read type and sensor#
  uint8_t typeAndSensor = (uint8_t) LoRa.read();
  uint8_t type = typeAndSensor >> 4;
  
  DEBUG_SERIAL.print("Type: ");
  printByte(type);
  DEBUG_SERIAL.println();
  
  // do action according to message type
  switch (type) {
    // <--- CASE: Registration ---> //
    case 0x0c: {
      // read nodeID
      uint8_t nodeID = (uint8_t) LoRa.read();
      // read unixTime
      uint8_t* unixTime = (uint8_t*) malloc(sizeof(uint8_t) * 4);
      // !!! reverse order <= little edian
      unixTime[0] = (uint8_t) LoRa.read();
      unixTime[1] = (uint8_t) LoRa.read();
      unixTime[2] = (uint8_t) LoRa.read();
      unixTime[3] = (uint8_t) LoRa.read();
      // READ DATA TEST
      #ifdef DEBUG_SERIAL
      DEBUG_SERIAL.print(F("nodeID: "));
      printByte(nodeID);
      DEBUG_SERIAL.println();
      DEBUG_SERIAL.print(F("unixTime: "));
      printByte(unixTime[0]);
      printByte(unixTime[1]);
      printByte(unixTime[2]);
      printByte(unixTime[3]);
      DEBUG_SERIAL.println();
      #endif
      // covert data to string
      DEBUG_SERIAL.print(F("nodeID: "));
      DEBUG_SERIAL.println(String(nodeID));
      DEBUG_SERIAL.print(F("unixTime: "));
      DEBUG_SERIAL.println(String(* (unsigned long*) unixTime));
      // clear
      free(unixTime); 
      // register
      registerTracker((char) nodeID);
      // send ACK
      sendAck(nodeID);     
      break;
    }
      
    // <--- CASE: Sensor Data ---> //
    case 0x0d: {
      // read nodeID
      uint8_t nodeID = (uint8_t) LoRa.read();
      // read unixTime
      uint8_t* unixTime = (uint8_t*) malloc(sizeof(uint8_t) * 4);
      unixTime[0] = (uint8_t) LoRa.read();
      unixTime[1] = (uint8_t) LoRa.read();
      unixTime[2] = (uint8_t) LoRa.read();
      unixTime[3] = (uint8_t) LoRa.read();
      // read latitude
      uint8_t* latitude = (uint8_t*) malloc(sizeof(uint8_t) * 4);
      latitude[0] = (uint8_t) LoRa.read();
      latitude[1] = (uint8_t) LoRa.read();
      latitude[2] = (uint8_t) LoRa.read();
      latitude[3] = (uint8_t) LoRa.read();
      // read longitude
      uint8_t* longitude = (uint8_t*) malloc(sizeof(uint8_t) * 4);
      longitude[0] = (uint8_t) LoRa.read();
      longitude[1] = (uint8_t) LoRa.read();
      longitude[2] = (uint8_t) LoRa.read();
      longitude[3] = (uint8_t) LoRa.read();
      // READ DATA TEST
      #ifdef DEBUG_SERIAL
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
      #endif    
      // covert data to string
      DEBUG_SERIAL.print(F("nodeID: "));
      DEBUG_SERIAL.println(String(nodeID));
      DEBUG_SERIAL.print(F("unixTime: "));
      DEBUG_SERIAL.println(String(* (unsigned long*) unixTime));
      DEBUG_SERIAL.print(F("latitude: "));
      DEBUG_SERIAL.println(String(* (long*) latitude));
      DEBUG_SERIAL.print(F("longitude: "));
      DEBUG_SERIAL.println(String(* (long*) longitude));
      // store data into csv
      addData((char) nodeID, 
              "0", 
              String(* (unsigned long*) unixTime),
              String(* (long*) latitude),
              String(* (long*) longitude));
      // send ACK
      sendAck(nodeID);
      // clear
      free(unixTime);
      free(latitude);
      free(longitude);
      readyToPost = true;
      break;
    }
      
    // <--- CASE: Sensor Data ---> //      
  }
}

// send ack to targe node
void sendAck(uint8_t nodeID) {
  uint8_t* message = (uint8_t*) malloc(sizeof(uint8_t) * 2);
  message[0] = (uint8_t) 0x00;
  message[1] = (uint8_t) nodeID;
  
  DEBUG_SERIAL.print(F("ACK TARGET NodeID: "));
  printByte(nodeID);
  DEBUG_SERIAL.println();
  
  LoRa_sendMessage(message, sizeof(uint8_t) * 2);
  DEBUG_SERIAL.println(F("ACK Sent."));
  free(message);
}

void onTxDone() {
  DEBUG_SERIAL.println(F("TxDone"));
  LoRa_rxMode();
}
