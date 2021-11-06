#include <SPI.h>
#include <NMEAGPS.h>
#include <LoRa.h>
#include <NeoSWSerial.h>
#include "LowPower.h"

#define GPS_RX 4
#define GPS_TX 3
#define GPS_EN 6

#define GPS_TIMEOUT 40000

#define SLEEP_CYCLES 112 // Number of loops the tracker will sleep for 112 is ~15min

NeoSWSerial gpsPort(GPS_TX, GPS_RX);

NMEAGPS gps;
gps_fix fix;

byte msgCount = 0;        // Keeps track of how many messages were attempted to be sent
byte localAddress = 2;    // Node ID

void setup() {
  pinMode(GPS_EN, OUTPUT);
  
  Serial.begin(115200);
  
  if (!LoRa.begin(915E6)) {
    while (1);
  }

  LoRa.enableCrc();       // Enables the LoRa module's built in error checking
  
  gpsPort.begin(9600);
}

void loop() {
  gpsPort.listen();
  digitalWrite(GPS_EN, HIGH);

  // Run the GPS for some time to try and get a fix
  long start = millis();
  while (millis() - start < GPS_TIMEOUT && !fix.valid.location) {
    readGPS();
  }

  // If no satellites are found, skip the main routine and sleep the module
  if (fix.satellites == 0)
    goto lowPowerMode;

  while (millis() - start < GPS_TIMEOUT && !fix.valid.location) {
    readGPS();
  }

  digitalWrite(GPS_EN, LOW);

  sendMessage();

  delay(100);

lowPowerMode:
  LoRa.sleep();
  for (int i = 0; i < SLEEP_CYCLES; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);     // Sleeps the tracker for ~15 mins
  }
}

void readGPS() {
  while (gps.available(gpsPort)) {
    fix = gps.read();
  }
}

void sendMessage() {
  long latitude = fix.latitudeL();
  long longitude = fix.longitudeL();
  unsigned long currTime = (NeoGPS::clock_t) fix.dateTime;

  String message = "";
  message += String(currTime);
  message += ",";
  message += String(latitude);
  message += ",";
  message += String(longitude);
  
  LoRa.beginPacket();
  LoRa.write(localAddress);
  LoRa.write(msgCount);
  LoRa.write(message.length());
  LoRa.print(message);
  LoRa.endPacket();
  msgCount++;
}
