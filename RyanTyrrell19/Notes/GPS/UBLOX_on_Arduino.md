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
    * TTFF

* Use NeoGPS Library


## LoRa Cycle:



# Notes:

* Turns out you are limited by what messages can be read via the Arduino only PUBX (Proprietary UBX Messages) messages can be read by the Arduino. This data is much similar to that provided by NMEA. I do believe it is formated differently to iincrease transmission speed

* Return GPS reciever because it is defected

* BBR
    * need to know max time BBR can last and if Arduino does in fact recharge it
    * Test the BBR more. It may not work properly
        * Last time recorded on GPS was 1am. So it lasted about 8 hours
    * Difficult to say how long it'll last, how charged it is, etc. May be easier to use the Arduino?

* Cannot send/recive UBX commands (at least not with this module). Arduino cannot read the message format of the data
    * Different modules may support raw output, which could potentially be read by the Arduino
    * At least, there is minimal info out there on reading/writing UBX
* Ideally, want everything done via the Arduino to make it easier for the user. This may not be possible though for several reasons
    * Challenging to send UBX commands via the Arduino
    * would need additional connections to save data to the Arduinos EEPROM

* Really difficult to test something that should last so long with so many unknowns