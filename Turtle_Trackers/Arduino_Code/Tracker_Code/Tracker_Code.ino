/*
  Turtle Tracker Arduino code used in the re-mote setup found at 
  https://gitlab.cas.mcmaster.ca/re-mote
*/

// LoRa Library
// An Arduino library for sending and receiving data using LoRa radios.
// https://www.arduino.cc/reference/en/libraries/lora/
#include <LoRa.h>

// NMEA and ublox GPS parser for Arduino, configurable to use as few as 10 bytes of RAM
// https://github.com/SlashDevin/NeoGPS/blob/master/src/NMEAGPS.h
#include <NMEAGPS.h>

// Overwrite the GPS hardware's configuration
#include <TurtleTracker_UBX_2022.h>

// NeoSWSerial Library
// An efficient replacement for SoftwareSerial, Simultaneous RX & TX
// https://www.arduino.cc/reference/en/libraries/neoswserial/
#include <NeoSWSerial.h>

// Lightweight low power library for Arduino.
// https://github.com/rocketscream/Low-Power
#include "LowPower.h"

/* ------------------------ Config ------------------------- */

#define NODE_ID 0x01 // Unique Node ID

#define GPS_TX 3 // GPS TX Pin
#define GPS_RX 4 // GPS RX Pin
#define GPS_EN 6 // GPS Enable Pin

#define LoRa_RESET 9 // LoRa reset Pin

#define GPS_TIMEOUT 80000 // GPS read timeout (ms) (Average read time 31sec on GP735T)
#define ACK_TIMEOUT 10000 // ack waiting time (ms) for sending sensor data
#define DEFAULT_SLEEP_CYCLES 112 // Number of loops the tracker will sleep 8s, for 112 is ~15min

// refer to message encoding in this file:
// https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/-/blob/master/Turtle_Trackers/Docs/message_format_turtle_tracker.xlsx
#define REG_LEN 4 // registration message length (in bytes)
#define SEN_LEN 14 // sensor data message length (in bytes)
#define ACK_LEN 3 // ack messgae length (in bytes)
#define DATA_POINT_NUM 19 // the maximum num of data points that can be stored in lora buff
#define DATA_POINT_SIZE 13 // size of one data points (in bytes)
#define BUF_SIZE DATA_POINT_NUM*DATA_POINT_SIZE // no more than 256 bytes (maximum len of a lora message)
#define REG_AND_NUM 0xc0 // registration message type and sensor number 0
#define SEN_AND_NUM 0xd0 // sensor data message type and sensor number 0

#define LORA_TX_POWER 20 // Output power of the RFM95 (23 is the max)
#define INVERT_IQ_MODE false // currently use InvertIQ mode, set it to false to stop INVERTIQ

#define PC_BAUDRATE 9600L   // Debug Serial Baudrate
#define GPS_BAUDRATE 9600L  // Debug GPS Baudrate

#define DEBUG true // Set to true for debug output, false for no output
#define DEBUG_SERIAL if(DEBUG) Serial

/* ------------------------ Constructors ------------------------- */

NeoSWSerial gpsPort(GPS_TX, GPS_RX);
NMEAGPS gps;
gps_fix fix;

/* ------------------------ Golbal Variables ------------------------- */

boolean ackReceived = false; // flag used in automatically detecting ack
long lastTime = 0; // Save the timestamp for the last GPS fix data
uint16_t serialNum = 0; // count how many messages (registration or sensor data) has been sent.
uint8_t trackerSleepCycles = DEFAULT_SLEEP_CYCLES; // initially, set the tracker sleep cycles to default.

// a circular buf for storing the body (data points) of the lora message
// maximum supported size is 256, as lora can only packet 256 bytes into one message.
typedef struct CircularBuffer {
  uint8_t bufArray[BUF_SIZE] = {};
  uint8_t writeIndex = 0;
  uint8_t readIndex = 0;
  uint8_t bufLen = 0;
} circularBuffer;
circularBuffer loraBuf;

/* ------------------------ Setup ------------------------- */

// Main task: send registration
void setup() {
  initilaize();

  unsigned long lastSent = millis();  // track what is the last sent time
  unsigned long sent_time = 0; // dynamic time interval for sent
  while (!ackReceived) {
    if ((millis() - lastSent) > sent_time){
      DEBUG_SERIAL.print(F("Wait for (millis): "));
      DEBUG_SERIAL.println(sent_time);
      
       /*---------- send registration -----------*/
      DEBUG_SERIAL.println(F("Trying to send registration..."));
      // allocate space for message
      uint8_t message[REG_LEN];
      // write message type and sensor#
      message[0] = (uint8_t) REG_AND_NUM;
      // write node id
      message[1] = (uint8_t) NODE_ID;
      // creat a helper pointer for mem copy
      uint8_t* temp; 
      // write serial number (16 bits = 2 bytes)
      temp = (uint8_t*) &serialNum;
      message[2] = (uint8_t) *(temp);
      message[3] = (uint8_t) *(temp + 1);
      // send message
      LoRa_sendMessage(message, REG_LEN);
      serialNum++; // message sent done
      /*-----------------------------------------*/
      sent_time = 1000 + random(1000); // (ms) based on experiments, the minimum progation time is 1s
      lastSent = millis();
    }
  }

  // put LoRa to end mode before using it to save power
  LoRa.sleep();
  resetLoRa();
  
  DEBUG_SERIAL.println(F("Initialize Successfully!"));
}

/* ------------------------ Loop ------------------------- */

// Main task: send sensor data
void loop() {
  // Initialize
  ackReceived = false; // reset ackReceived to FALSE

//  // Read time & gps
//  enableGPS();
//  readGPSValid();
//  disableGPS();
//  
//  #if DEBUG
//  printLocationData();
//  #endif  //DEBUG == true
//
//  // If No valid data, then skip sending data and sleep the module
//  if (!ifVaildFix()) {
//    enterLowPowerMode(trackerSleepCycles);
//    return;
//  }

  // Now, fix should hold the time and location data

  // Store the time and location data into buff
  if (isFullBuf(&loraBuf)) {
    // if buff is full, throw that message then skip this data points 
    DEBUG_SERIAL.println(F("BUFF IS FULL. will skip collecting datapoints and try sending the buff FIRST."));
  } else {
    // only collect data if the buff is not full
  // Read data from fix
//    unsigned long unixTime = (NeoGPS::clock_t) fix.dateTime + 946684800; // 32 bits i.e 4 bytes
//    long latitude = fix.latitudeL(); // 32 bits i.e 4 bytes
//    long longitude = fix.longitudeL(); // 32 bits i.e 4 bytes
    unsigned long unixTime = 1656555632; // 32 bits i.e 4 bytes
    long latitude = 432582727; // 32 bits i.e 4 bytes
    long longitude = -799207620; // 32 bits i.e 4 bytes
    uint8_t temperature = 0x02; // 8 bits i.e. 1 byte (HARDCODED)
    
    uint8_t* temp; // a helper pointer
    // write unixTime (32 bits = 4 bytes)
    temp = (uint8_t*) &unixTime;
    for (uint8_t i = 0; i < 4; i++) {
      writeBuf(&loraBuf, *(temp + i));
    }
    // write latitude (32 bits = 4 bytes)
    temp = (uint8_t*) &latitude;
    for (uint8_t i = 0; i < 4; i++) {
      writeBuf(&loraBuf, *(temp + i));
    }
    // write longitude (32 bits = 4 bytes)
    temp = (uint8_t*) &longitude;
    for (uint8_t i = 0; i < 4; i++) {
      writeBuf(&loraBuf, *(temp + i));
    }
    // write tempurature
    writeBuf(&loraBuf, temperature);
  }

  // Always try sending the WHOLE buff using LoRa
  
  unsigned long ackStartTime = millis(); // track the ack start waiting time
  uint8_t millis_count = 0; // count of iteration
  unsigned long lastSent = millis();  // track what is the last sent time
  unsigned long sent_time = 0; // dynamic time interval for sent
  while (!ackReceived && millis() - ackStartTime < ACK_TIMEOUT) {    
    if ((millis() - lastSent) > sent_time){
      DEBUG_SERIAL.print(F("Wait for (millis): "));
      DEBUG_SERIAL.println(sent_time);
      
      DEBUG_SERIAL.println(F("Trying to send the WHOLE buff..."));
      LoRa_txMode();
      LoRa.beginPacket();
      // write message type & sensor num
      LoRa.write((uint8_t) SEN_AND_NUM);
      // write node id
      LoRa.write((uint8_t) NODE_ID);
      // creat a helper pointer for mem copy
      uint8_t* temp; 
      // write serial number (16 bits = 2 bytes)
      temp = (uint8_t*) &serialNum;
      LoRa.write((uint8_t) *(temp));
      LoRa.write((uint8_t) *(temp + 1));
      // write battery percentage (HARDCODED)
      LoRa.write((uint8_t) 0x40);
      // write datapoint count
      LoRa.write((uint8_t) (loraBuf.bufLen / DATA_POINT_SIZE));
      // debug buffer size
      DEBUG_SERIAL.print(F("Buffer Size: "));
      DEBUG_SERIAL.println(loraBuf.bufLen);
      // write buff into lora message
      LoRa_writeFromBuff(&loraBuf);
      LoRa.endPacket(true);
      serialNum++; // message sent done
  
      sent_time = (2 << millis_count) * 1000 + random(1000); // (ms) based on experiments, the minimum progation time is 1s
      lastSent = millis();
      millis_count++;
    }
  }

  // if received ACK, then we need to "fake" read the buff (i.e. update buffLen and readIndex)
  if (ackReceived) {
    readNFromBuf(&loraBuf, loraBuf.bufLen);
  }

  // enter lowpower mode
  enterLowPowerMode(trackerSleepCycles);
}

/* ------------------------ Callback Functions ------------------------- */

// if LoRa received any message, it will interrupt any current execution line,
// then begin call this onReceive function
void onReceive(int packetSize) {  
    if (packetSize == ACK_LEN) {
      // CASE: received registration ack
      uint8_t ack = (uint8_t) LoRa.read();
      uint8_t nodeID = (uint8_t) LoRa.read();
      if (ack == 0x00 && nodeID == NODE_ID) {
        #if DEBUG
        DEBUG_SERIAL.println(F("Received ACK:"));
        DEBUG_SERIAL.print(F("ACK type: "));
        printByte(ack);
        DEBUG_SERIAL.println();
        DEBUG_SERIAL.print(F("Node ID: "));
        printByte(nodeID);
        DEBUG_SERIAL.println();
        #endif

        // accept remote control from gateway
        trackerSleepCycles = (uint8_t) LoRa.read();
        
        // set the ack flag to true to break to read loop.
        ackReceived = true;
      }
    } else {
      // CASE: received noisy information
      String temp = "";
      while (LoRa.available()) {
        temp += (char) LoRa.read();
      }
      #ifdef DEBUG
      DEBUG_SERIAL.print(F("LoRa received: "));
      DEBUG_SERIAL.println(temp);
      #endif
    }
}

// if LoRa finish transmitted message, it will interrupt any current execution line,
// then begin call this onReceive function
void onTxDone() {
  DEBUG_SERIAL.println(F("Message Sent."));
  LoRa_rxMode();
}
