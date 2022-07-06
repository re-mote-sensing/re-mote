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

#define NODE_ID 0x98 // Unique Node ID

#define GPS_TX 3 // GPS TX Pin
#define GPS_RX 4 // GPS RX Pin
#define GPS_EN 6 // GPS Enable Pin
#define LoRa_RESET 9 // LoRa reset Pin

#define GPS_TIMEOUT 36000 // GPS read timeout (Average read time 31sec on GP735T)
#define ACK_TIMEOUT 10000 // ack waiting time for sending sensor data
#define DEFAULT_SLEEP_CYCLES 112 // Number of loops the tracker will sleep 8s, for 112 is ~15min

#define REG_LEN 6 // registration message length (in bytes)
#define SEN_LEN 14 // sensor data message length (in bytes)
#define ACK_LEN 3 // ack messgae length (in bytes)

// below are main configurations needed to changed
#define LORA_TX_POWER 20 // Output power of the RFM95 (23 is the max)
#define INVERT_IQ_MODE false // currently use InvertIQ mode, set it to false to stop INVERTIQ
#define BUF_SIZE 240 // set LoRa buff size to a mutipler of 12 (bytes of one data point) but no more than 256 (maximum len of a lora message)

#define PC_BAUDRATE 9600L
#define GPS_BAUDRATE 9600L

#define DEBUG true // Set to true for debug output, false for no output
#define DEBUG_SERIAL if(DEBUG) Serial

/* ------------------------ Constructors ------------------------- */

NeoSWSerial gpsPort(GPS_TX, GPS_RX);
NMEAGPS gps;
gps_fix fix;

/* ------------------------ Golbal Variables ------------------------- */

boolean ackReceived = false; // flag used in automatically detecting ack
long lastTime = 0; // Save the timestamp for the last GPS fix data
uint8_t sleep_cycles = DEFAULT_SLEEP_CYCLES; // Dynamic Sleep Cycles

uint8_t millis_count = 0; // second counts for calculating the wait time
unsigned long lastSent = millis();  // Track what is the last sent time
unsigned long sent_time = 0; // dynamic time interval for sent

uint8_t trackerSleepCycles = DEFAULT_SLEEP_CYCLES; // initially, set the tracker sleep cycles to default.

// a circular buf for storing the body (data points) of the lora message
// NOTE:
//  As the buff ONLY stores the body of sensor data message (with each data point is 4 bytes * 3 = 12 bytes),
//  So num of data points in buff = bufLen / 12
uint8_t buf[BUF_SIZE];
int writeIndex = 0;
int readIndex = 0;
int bufLen = 0;

/* ------------------------ Setup ------------------------- */

// Main task: send registration
void setup() {
  initilaize();

  millis_count = 0;
  while (!ackReceived) {
    if ((millis() - lastSent) > sent_time){
      DEBUG_SERIAL.print(F("Wait for (millis): "));
      DEBUG_SERIAL.println(sent_time);
      
       /*---------- send registration -----------*/
      DEBUG_SERIAL.println(F("Trying to send registration..."));
      // allocate space for message
      uint8_t* message = (uint8_t*) malloc(sizeof(uint8_t) * REG_LEN);
      // write message type and sensor#
      message[0] = (uint8_t) 0xc0;
      // write node id
      message[1] = (uint8_t) NODE_ID;
      
      // write unix time (HARDCODED)
      // In [4]: int('0xca35115d', base=16)
      // Out[4]: 3392475485
      // !!! reverse <= little edian
      message[2] = (uint8_t) 0x5D;
      message[3] = (uint8_t) 0x11;
      message[4] = (uint8_t) 0x35;
      message[5] = (uint8_t) 0xCA;
      // send message
      LoRa_sendMessage(message, REG_LEN);
      // free variable
      free(message);
      /*-----------------------------------------*/
      
//      sent_time = (2 << millis_count) * 1000 + random(1000); // based on experiments, the minimum progation time is 1s
      sent_time = 1000 + random(1000); // based on experiments, the minimum progation time is 1s
      lastSent = millis();
      millis_count++;
    }
    
    
  }

  // put LoRa to end mode before using it to save power
  LoRa.sleep();
//  DEBUG_SERIAL.println("MODE: END -- i.e. end");
  
  DEBUG_SERIAL.println(F("Initialize Successfully!"));
}

/* ------------------------ Loop ------------------------- */

// Main task: send sensor data
void loop() {
  // Initialize
  ackReceived = false; // reset ackReceived to FALSE

  // Read time & gps
//  enableGPS();
//  readGPSvaild();
//  disableGPS();
//  
//  #if DEBUG == true
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
  if (isFullBuf(bufLen, BUF_SIZE)) {
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
    
    uint8_t* temp; // a helper pointer
    // write unixTime
    temp = (uint8_t*) &unixTime;
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp));
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp + 1));
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp + 2));
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp + 3));
    // write latitude
    temp = (uint8_t*) &latitude;
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp));
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp + 1));
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp + 2));
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp + 3));
    // write longitude
    temp = (uint8_t*) &longitude;
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp));
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp + 1));
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp + 2));
    writeBuf(&buf[0], &writeIndex, &bufLen, *(temp + 3));
  }

  // Always try sending the WHOLE buff using LoRa
  unsigned long ackStartTime = millis();
  
  millis_count = 0; // reset millis count to 0
  lastSent = millis();  // reset lastSent to current time
  sent_time = 0; // reset sent_time to 0
  while (!ackReceived && millis() - ackStartTime < ACK_TIMEOUT) {    
    if ((millis() - lastSent) > sent_time){
      DEBUG_SERIAL.print(F("Wait for (millis): "));
      DEBUG_SERIAL.println(sent_time);
      
      DEBUG_SERIAL.println(F("Trying to send the WHOLE buff..."));
      LoRa_txMode();
      LoRa.beginPacket();
      // write header into lora message
      LoRa.write((uint8_t) 0xd0);
      LoRa.write((uint8_t) NODE_ID);
      // debug buffer size
      DEBUG_SERIAL.print(F("Buffer Size: "));
      DEBUG_SERIAL.println(bufLen);
      // write buff into lora message
      LoRa_writeFromBuff(&buf[0], &readIndex, &bufLen);
      LoRa.endPacket(true); 
  
      sent_time = (2 << millis_count) * 1000 + random(1000); // based on experiments, the minimum progation time is 1s
      lastSent = millis();
      millis_count++;
    }
  }

  // if received ACK, then we need to "fake" read the buff (i.e. update buffLen and readIndex)
  if (ackReceived) {
    lazyDeleteNFromBuf(&buf[0], &readIndex, &bufLen, bufLen);
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
        #if DEBUG == true
        DEBUG_SERIAL.println(F("Received ACK:"));
        DEBUG_SERIAL.print(F("ACK type: "));
        printByte(ack);
        DEBUG_SERIAL.println();
        DEBUG_SERIAL.print(F("Node ID: "));
        printByte(nodeID);
        DEBUG_SERIAL.println();
        #endif  //DEBUG == true

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
