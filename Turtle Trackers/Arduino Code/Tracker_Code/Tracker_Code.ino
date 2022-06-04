#include <SPI.h>
#include <NMEAGPS.h>
#include <LoRa.h>
#include <NeoSWSerial.h>
#include "LowPower.h"

#define GPS_TX 3
#define GPS_RX 4
#define GPS_EN 6 // GPS Enable Pin

#define GPS_TIMEOUT 40000

#define SLEEP_CYCLES 112 // Number of loops the tracker will sleep for 112 is ~15min

#define DEBUG true // Set to true for debug output, false for no output
#define DEBUG_SERIAL if(DEBUG)Serial

NeoSWSerial gpsPort(GPS_TX, GPS_RX);

NMEAGPS gps;
gps_fix fix;

byte msgCount = 0;        // Keeps track of how many messages were attempted to be sent
byte localAddress = 2;    // Node ID

void setup() {
  pinMode(GPS_EN, OUTPUT);
  
  DEBUG_SERIAL.begin(115200);
  
  if (!LoRa.begin(915E6)) {
    while (1);
  }
  LoRa.enableCrc(); // Enables the LoRa module's built in error checking
  
  gpsPort.begin(9600);
  
  DEBUG_SERIAL.println("started.");
}

void loop() {
  gpsPort.listen();
  digitalWrite(GPS_EN, HIGH); // Enable GPS

  // Run the GPS for some time to try and get a fix
  long start = millis();
  DEBUG_SERIAL.println("readGPS.");
  while (millis() - start < GPS_TIMEOUT && (!fix.valid.location || !fix.valid.time)) {
    readGPS();
  }

  #if DEBUG == true
  DEBUG_SERIAL.println(millis() - start);
  delay(100);
  DEBUG_SERIAL.println(fix.valid.location);
  delay(100);
  DEBUG_SERIAL.println(fix.latitudeL());
  delay(100);
  DEBUG_SERIAL.println(fix.longitudeL());
  #endif  //DEBUG == true

  // If no satellites are found, skip the main routine and sleep the module
  if (fix.satellites == 0)
    goto lowPowerMode;

  while (millis() - start < GPS_TIMEOUT && !fix.valid.location) {
    readGPS();
  }

  digitalWrite(GPS_EN, LOW); // Power off GPS

  sendMessage();

lowPowerMode:
  DEBUG_SERIAL.println("lowPowerMode Start.");
  delay(100);
  digitalWrite(GPS_EN, LOW); // Make sure GPS is off before sleep
  LoRa.sleep();
  for (int i = 0; i < SLEEP_CYCLES; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); // Sleeps the tracker for ~15 mins
  }
  DEBUG_SERIAL.println("lowPowerMode End.");
}

void readGPS() {
  while (gps.available(gpsPort)) {
    fix = gps.read();
  }
}

void sendMessage() {
  DEBUG_SERIAL.println("sendMessage.");
  long latitude = fix.latitudeL();
  long longitude = fix.longitudeL();
  unsigned long currTime = (NeoGPS::clock_t) fix.dateTime + 946684800; //Get Unix Time (946684800 is because conversion gives Y2K epoch)

  if(currTime <= 1000000000){ // The time is not fixed
    DEBUG_SERIAL.println("timestamp Error.");
    // TODO Error timestamp
  }

  if (latitude == 0 || longitude == 0){ // latitude / longitude get zero
    DEBUG_SERIAL.println("latitude / longitude Error.");
    return;
  }

  String message = "";
  message += String(currTime);
  message += ",";
  message += String(latitude);
  message += ",";
  message += String(longitude);
  
  // ------- core part ------ //
  // LoRa.beginPacket()
  // Start the sequence of sending a packet.
  LoRa.beginPacket();
  // LoRa.Writing()
  // Write data to the packet. Each packet can contain up to 255 bytes.
  LoRa.write(77); // Tell the gateway this is a turtle tracker
  LoRa.write(localAddress);
  LoRa.write(msgCount);
  LoRa.write(message.length());
  // Note: Other Arduino Print API's can also be used to write data into the packet
  LoRa.print(message);
  // LoRa.endPacket()
  // End the sequence of sending a packet.
  LoRa.endPacket();
  // ------- core part ------ //
  msgCount++;
}
