# Planner

## Pre-Steps:
1) Load Pre-made config settings to GPS reciever
2) Allow GPS to run for at least 12.5min to downoad Almanac data

## Arduino Cycle:
1) Arduino will sleep during the night, run during the day.
    * This cycle will be maintained using the watchdog timer and corrected using GPS time when a fix is obtained
        * see if GPS RTC is usable (not sure if it continues running or not)
        

## GPS Cycle:
1) Send LoRa transmission with empty payload. If no ACK is received, Tracker is underwater (or Gateway is off/damaged/far away). Regardless of the reason, power back down
2) Power on GPS
3) Read RTC to determine time since last fix
4) Allow GPS to get a fix
4.5) While GPS is getting a fix, read the following
    * RTC
    * # SVs Used
    * Valid Fix
    * Lat/lng

* Use NeoGPS Library


## LoRa Cycle:



# Notes:
* need to know max time BBR can last and if Arduino does in fact recharge it

* Turns out you are limited by what messages can be read via the Arduino only PUBX (Proprietary UBX Messages) messages can be read by the Arduino. This data is much similar to that provided by NMEA. I do believe it is formated differently to iincrease transmission speed