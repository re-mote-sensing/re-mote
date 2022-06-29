/*
  Turtle Tracker Arduino code used in the re-mote setup found at 
  https://gitlab.cas.mcmaster.ca/re-mote
*/

#include <SPI.h>
#include <LoRa.h>
#include <NMEAGPS.h>
#include <NeoSWSerial.h>
#include "LowPower.h"

/* ------------------------ Config ------------------------- */

#define GPS_TX 3
#define GPS_RX 4
#define GPS_EN 6 // GPS Enable Pin
#define LoRa_RESET 9 // LoRa reset Pin

#define GPS_TIMEOUT 80000
#define ACK_TIMEOUT 10000 // ack waiting time for sending sensor data
#define DEFAULT_SLEEP_CYCLES 112 // Number of loops the tracker will sleep 8s, for 112 is ~15min

#define REG_LEN 6 // registration message length (in bytes)
#define SEN_LEN 14 // sensor data message length (in bytes)

// below are main configurations needed to changed
#define LORA_TX_POWER 23 // Output power of the RFM95 (23 is the max)
#define INVERT_IQ_MODE false // currently use InvertIQ mode, set it to false to stop INVERTIQ
#define DEBUG true // Set to true for debug output, false for no output
#define DEBUG_SERIAL if(DEBUG) Serial
#define NODE_ID 0x06

/* ------------------------ Constructors ------------------------- */

NeoSWSerial gpsPort(GPS_TX, GPS_RX);
NMEAGPS gps;
gps_fix fix;

/* ------------------------ Golbal Variables ------------------------- */

boolean ackReceived = false; // flag used in automatically detecting ack
long lastTime = 0; // Save the timestamp for the last GPS fix data
uint8_t sleep_cycles = DEFAULT_SLEEP_CYCLES; // Dynamic Sleep Cycles
long millis_count; // second counts for calculating the wait time

/* ------------------------ Setup ------------------------- */

// Main task: send registration
void setup() {
  initilaize();

  millis_count = random(201); // generate a random number from 0 to 200 (E(X) = 100)
  while (!ackReceived) {
    sendRegistration();
    // wait random time for solving collision among multiple trackers
    DEBUG_SERIAL.print("Wait for (millis): ");
    DEBUG_SERIAL.println(millis_count * 10);
    delay(millis_count * 10);  // in average, wait for 1000 millis
    millis_count *= 2; // then every time not received ack, wait current_interval * 2
  }

  // put LoRa to end mode before using it to save power
  LoRa.end();
//  DEBUG_SERIAL.println("MODE: END -- i.e. end");
  
  DEBUG_SERIAL.println("Initialize Successfully!");
}

/* ------------------------ Loop ------------------------- */

// Main task: send sensor data
void loop() {
  // Initialize
  ackReceived = false; // reset ackReceived to FALSE

  // Read time & gps
  enableGPS();
  readGPSvaild();
  disableGPS();
  #if DEBUG == true
  printLocationData();
  #endif  //DEBUG == true

  // If No valid data, then skip sending data and sleep the module
  if (!ifVaildFix()) {
    
    enterLowPowerMode();
    return;
  }

  // Now, fix should hold the time and location data
  // Send time and gps data
  millis_count = random(201); // generate a random number from 0 to 200 (E(X) = 100)
  unsigned long ackStartTime = millis();
  while (!ackReceived && millis() - ackStartTime < ACK_TIMEOUT) {
    sendSensorData();
    // wait random time for solving collision among multiple trackers
    DEBUG_SERIAL.print("Wait for (millis): ");
    DEBUG_SERIAL.println(millis_count * 10);
    delay(millis_count * 10); // in average, wait for 1000 millis
    millis_count *= 2; // then every time not received ack, wait current_interval * 2
  }
  
  // enter lowpower mode
  enterLowPowerMode();
}

/* ------------------------ Callback Functions ------------------------- */

// if LoRa received any message, it will interrupt any current execution line,
// then begin call this onReceive function
void onReceive(int packetSize) {  
    if (packetSize == 2) {
      // receive registration ack
      uint8_t ack = (uint8_t) LoRa.read();
      uint8_t nodeID = (uint8_t) LoRa.read();
      if (ack == 0x00 && nodeID == NODE_ID) {
        DEBUG_SERIAL.println("Received ACK:");
        DEBUG_SERIAL.print("ACK type: ");
        printByte(ack);
        DEBUG_SERIAL.println();
        DEBUG_SERIAL.print("Node ID: ");
        printByte(nodeID);
        DEBUG_SERIAL.println();
        // success receive ack
        ackReceived = true;
      }
    } else {
      String temp = "";
      while (LoRa.available()) {
        temp += (char) LoRa.read();
      }
      #ifdef DEBUG
      DEBUG_SERIAL.print("LoRa received: ");
      DEBUG_SERIAL.println(temp);
      #endif
    }
}

// if LoRa finish transmitted message, it will interrupt any current execution line,
// then begin call this onReceive function
void onTxDone() {
  DEBUG_SERIAL.println("Message Sent.");
  LoRa_rxMode();
}
