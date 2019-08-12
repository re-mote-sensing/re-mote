/*
Library for reading from a GPS, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated July 25, 2019
Released into the public domain
*/

#include "remoteGPS.h"
#include <NMEAGPS.h>
#include <NeoSWSerial.h>
#include <remoteConfig.h>

#ifdef DEBUG
#include <MemoryFree.h>
#endif

//Set the pins for the GPS
remoteGPS::remoteGPS() {
}

//Initialise the GPS
void remoteGPS::initialise() {
    pinMode(GPS_EN, OUTPUT);
    digitalWrite(GPS_EN, LOW);
}

//Offset the time that the GPS last updated (needed if you are sleeping the Arduino and millis() isn't incremented)
void remoteGPS::offsetTime(long offset) {
    unixTime += offset/1000;
}

//Get data from the GPS
void remoteGPS::getData(unsigned long* time, float* lat, float* lon, unsigned long timeout) {
    unsigned long start = millis();
    
    //Start GPS
    digitalWrite(GPS_EN, HIGH);
    NMEAGPS gps;
    NeoSWSerial gpsSS(GPS_RX, GPS_TX);
    gpsSS.begin(9600);
    delay(250); //Delay to make sure GPS is working properly
    
    //If it's the first run, it will force all data to be updated
    bool force = unixTime == 0;
    bool gotLoc = !force && (lat == NULL) && (lon == NULL);
    bool gotTime = !force && (time == NULL);
    
    #ifdef DEBUG
    Serial.println(F("About to wait for GPS"));
    Serial.println(freeMemory());
    delay(100);
    #endif
    
    bool first = false;
    
    //Try for timeout time
    while (force || ((millis() - start) < timeout)) {
        if (gotLoc && gotTime) { //If everything has been updated
            break;
        }
        
        if (gps.available(gpsSS)) { //Returns true if we have a fix
            gps_fix fix = gps.read();
            
            //The first fix the GPS gets is sometimes garbage, so ignore it
            if (!first) {
                first = true;
                continue;
            }
            
            if (fix.valid.time && fix.valid.date) {
                unsigned long currTime = ((NeoGPS::clock_t) fix.dateTime) + 946684800; //Get Unix Time (946684800 is because conversion gives Y2K epoch)
                bool err = false;
                
                //Check to make sure number makes sense
                if (unixTime == 0) {
                    err = err || currTime < 1562862367; //The time that I wrote this, best I could do
                    err = err || currTime > 2000000000; //In the year 2033
                } else {
                    err = err || currTime < unixTime;
                    err = err || (currTime - unixTime > 2678400); //2678400 is a month
                }
                
                if (!err) {
                    #ifdef DEBUG
                    //Serial.print(F("Got time: "));
                    //Serial.println(currTime);
                    #endif
                    
                    unixTime = currTime;
                    timeLastUpdated = millis();
                    gotTime = true;
                } else {
                    #ifdef DEBUG
                    Serial.println(F("Got garbage time"));
                    #endif
                }
            }
            
            if (fix.valid.location) {
                float currLat = fix.latitude();
                float currLon = fix.longitude();
                bool err = false;
                
                //Check to make sure numbers make sense
                
                err = err || abs(currLat) < 0.00001;
                err = err || abs(currLon) < 0.00001;
                err = err || (currLat < -90.0) || (currLat > 90.0);
                err = err || (currLon < -180.0) || (currLon > 180.0);
                
                if (latitude != 0.0 && longitude != 0.0) {
                    err = err || abs(currLat - latitude) > 0.1; //0.1 degrees is about 11 km
                    err = err || abs(currLon - longitude) > 0.1;
                }
                
                if (!err) {
                    #ifdef DEBUG
                    Serial.print(F("Got location: "));
                    Serial.print(currLat, 6);
                    Serial.print(F(", "));
                    Serial.println(currLon, 6);
                    delay(250);
                    #endif
                    
                    latitude = currLat;
                    longitude = currLon;
                    gotLoc = true;
                } else {
                    #ifdef DEBUG
                    Serial.println(F("Got garbage location"));
                    #endif
                }
            }
        }
    }
    
    //Turn off GPS
    digitalWrite(GPS_EN, LOW);
    gpsSS.end();
    
    if (lat != NULL)
        (*lat) = latitude;
    
    if (lon != NULL)
        (*lon) = longitude;
    
    if (time != NULL) {
        (*time) = unixTime + (millis() - timeLastUpdated)/1000;
    }
}