/*
  Turtle Tracker Arduino code used in the re-mote setup found at 
  https://gitlab.cas.mcmaster.ca/re-mote
*/

/* ------------------------ Config ------------------------- */

#define GATEWAY_ID 0x01 // Unique Gateway ID

// Sever Info
#define HOST "turtletracker.cas.mcmaster.ca"  // Change this to the public IP address / domain of the pi server's network
#define ENDPOINT "/data"                      // Server endpoint
#define SERVER_PORT 80                        // Server port number

// Length of variable in postToServerWithBuffer()
// Must change after changing "host" "endpoint" or "port"
#define CHTTPSOPSE_LENGTH 53    // Command:22 + $Port:2 + $host:29
#define msgBody_LENGTH 170      // Command:2 + ($CsvLineLength:32-4)*$POST_AMOUNT:6
#define POST_COMMAND_LENGTH 237 // Command:33 + $host:29 + $endpoint:5 + $msgBody_LENGTH:2 + ($CsvLineLength:32-4)*$POST_AMOUNT:6

// Length of each line in ToSend.bin: 
// Type,NodeID,TimeStamp,Lat,Lon
// \r\n0d065d1135ca15cd5b07b168de3a
#define CSV_LINE_LENGTH 32

// HTTP Post
#define POST_AMOUNT 6       // Define how many tracker data to read for a single post
#define POST_TIME 30000     // Time between each post

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
#define SIM3G_EN A1                   // 3G Enable Pin
#define SIM3G_TX 4                    // 3G TX Pin
#define SIM3G_RX 5                    // 3G RX Pin
#define SIM3G_POWER_ON_TIME 180       // 3G Power on time for enable pin
#define SIM3G_POWER_OFF_TIME 4000     // 3G Power off time for enable pin
#define SIM3G_HTTP_TIMEOUT 4000       // 3G http timeout
#define SIM3G_POWER_ON_TIMEOUT 20000  // 3G power on timeout

// LoRa
#define LORA_TX_POWER 23 // Output power of the RFM95 (23 is the max)
#define INVERT_IQ_MODE false  // InvertIQ mode
#define REG_LEN 6             // registration message length (in bytes)
#define SEN_LEN 14            // sensor data message length (in bytes)
#define INTERUPT_MODE false   // Use interupt pin when receive a LoRa Message (Arduino Uno does not have enough SRAM)

// Serial
#define PC_BAUDRATE 9600L     // Debug Serial Baudrate
#define SIM3G_BAUDRATE 9600L  // 3G Shield Serial Baudrate

#define DEBUG true  // Set to true for debug output, false for no output

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
#if DEBUG == true
#include <MemoryFree.h>
#endif

/* ------------------------ Constructors ------------------------- */

#define DEBUG_SERIAL if(DEBUG)Serial  // Serial for debuging
NeoSWSerial ss(SIM3G_TX, SIM3G_RX);   // Serial for 3G Shield
// SdFat software SPI
SoftSpiDriver<SD_SOFT_MISO_PIN, SD_SOFT_MOSI_PIN, SD_SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)

/* ------------------------ Golbal Variables ------------------------- */

unsigned long lastPost = millis();  // Track what is the last posted time
bool readyToPost = true;            // Ready to post to server
uint8_t trackerSleepCycles = 8; // Tracker remotge control

/* ------------------------ Setup ------------------------- */

void setup() {
  DEBUG_SERIAL.begin(PC_BAUDRATE); // Start Debug Serial

  // Power on 3G then turn it off
  pinMode(SIM3G_EN, OUTPUT);
  powerOn3G();
  delay(1000);
  powerOff3G();
  
  // Start 3G Serial
  ss.begin(SIM3G_BAUDRATE);
  DEBUG_SERIAL.println(F("3G Ok"));

  // Start SD and create ToSend.bin
  createToSendFile(); 

  // Start LoRa
  if (!LoRa.begin(915E6)) {
    DEBUG_SERIAL.println(F("LoRa init failed. Check your connections."));
    while (true);
  }
  LoRa.setTxPower(LORA_TX_POWER); // maximum tx power to get longest range
  LoRa.setSpreadingFactor(12); // maximum SF to get longest range
  LoRa.enableCrc(); // Enables the LoRa module's built in error checking
  DEBUG_SERIAL.println(F("LoRa init succeeded."));
  // Uncomment the following three line to enable onReceive with interupt pins DIO0(D2)
  #if INTERUPT_MODE == true
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
    lastPost = millis();
    readyToPost = false;
  }

  #if INTERUPT_MODE == false
  int packetSize = LoRa.parsePacket();
  String result;
  if (packetSize) {
    onReceive(packetSize);
  }
  #endif
}

/* ------------------------ LoRa helper functions ------------------------- */

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
  if ((packetSize - 2) % 12 != 0 && packetSize != REG_LEN) {
    return;
  }
  
  DEBUG_SERIAL.print(F("Size: "));
  DEBUG_SERIAL.println(packetSize);
  
  // read type and sensor#
  uint8_t typeAndSensor = (uint8_t) LoRa.read();
  uint8_t type = typeAndSensor >> 4;

  #ifdef DEBUG
  DEBUG_SERIAL.print("Type: ");
  printByte(type);
  DEBUG_SERIAL.println();
  #endif
  
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
      #ifdef DEBUG
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

      // calculate num of datapoints
      // num of datapoints = (messageLen - header lenfth) / (size of ONE data point (4 bytes) i.e. (unixTime, latitude, longtitude))
      int num_data = (packetSize - 2) / 12;
      for (int i = 0; i < num_data; i++) {
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
  
        // Add data into buffer
        addDataByte("0",
                type,
                nodeID, 
                unixTime,
                latitude,
                longitude);

        // READ DATA TEST
        #ifdef DEBUG
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
        // covert data to string
        DEBUG_SERIAL.print(F("nodeID: "));
        DEBUG_SERIAL.println(String(nodeID));
        DEBUG_SERIAL.print(F("unixTime: "));
        DEBUG_SERIAL.println(String(* (unsigned long*) unixTime));
        DEBUG_SERIAL.print(F("latitude: "));
        DEBUG_SERIAL.println(String(* (long*) latitude));
        DEBUG_SERIAL.print(F("longitude: "));
        DEBUG_SERIAL.println(String(* (long*) longitude));
        #endif
        // clear
        free(unixTime);
        free(latitude);
        free(longitude);
      }       
      // send ACK
      sendAck(nodeID);
      readyToPost = true; // Ready to post to server
      break;
    }    
  }
}

// send ack to targe node
void sendAck(uint8_t nodeID) {
  uint8_t* message = (uint8_t*) malloc(sizeof(uint8_t) * 3);
  message[0] = (uint8_t) 0x00;
  message[1] = (uint8_t) nodeID;
  message[2] = (uint8_t) trackerSleepCycles; 
  
  DEBUG_SERIAL.print(F("ACK TARGET NodeID: "));
  printByte(nodeID);
  DEBUG_SERIAL.println();
  
  LoRa_sendMessage(message, sizeof(uint8_t) * 3);
  DEBUG_SERIAL.println(F("ACK Sent."));
  free(message);
}

void onTxDone() {
  DEBUG_SERIAL.println(F("TxDone"));
  LoRa_rxMode();
}
