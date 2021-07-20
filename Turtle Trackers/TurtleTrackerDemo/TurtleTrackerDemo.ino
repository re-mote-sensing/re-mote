#include <SPI.h>
#include <NMEAGPS.h>
#include <LoRa.h>
#include <NeoSWSerial.h>

#define GPS_RX 4
#define GPS_TX 3
#define GPS_EN 6
#define GPS_BACK 5

NeoSWSerial gpsPort(GPS_TX, GPS_RX);

static NMEAGPS gps;
static gps_fix fix;

//  This function gets called about once per second, during the GPS
//  quiet time.  It's the best place to do anything that might take
//  a while: print a bunch of things, write to SD, send an SMS, etc.
//
//  By doing the "hard" work during the quiet time, the CPU can get back to
//  reading the GPS chars as they come in, so that no chars are lost.
static void sendGPSData() {
  LoRa.beginPacket();
  LoRa.print("none");
  LoRa.endPacket();
}

//  This is the main GPS parsing loop.
static void GPSloop() {
  while (gps.available(gpsPort)) {
    fix = gps.read();
    sendGPSData();
  }
}

void setup() {
//  GPS draws too much current
//  pinMode(GPS_EN, OUTPUT);
//  pinMode(GPS_BACK, OUTPUT);

//   digitalWrite(GPS_EN, HIGH); // Turn GPS on
//   digitalWrite(GPS_BACK, HIGH); // Keep GPS_BACK high to supply backup power, does not increase power consumption
  
  Serial.begin(115200);
  Serial.println("Starting tracker");
  
  //while (!Serial);
  if (!LoRa.begin(915E6)) {
    while (1);
  }
  gpsPort.begin(9600);
}

void loop() {
  LoRa.beginPacket();
  LoRa.print("none");
  LoRa.endPacket();

  delay(10000);
}
