# GPS Basics:

* satellites, equiped with extremely accurate atomic clocks, orbit the earth. The satellites continuously send data down to earth over dedicated RF frequencies. GPS receivers have tiny processors and antennas that directly receive the data sent by the satellites and compute your position and time [[1]]

* 24 satellites orbit the earth twice a day. The positions of the satellites are constructed in a way that the sky above your location will contain at least 4 satellites and, at most, 12 satellites. [[1]][[3]]

* To compute it's location, the GPS module recieves a timestamp from each of the visible satellites, along with data on where in the sky each one is located (among other pieces of data). If the GPS receiver’s antenna can see at least 4 satellites, it can accurately calculate its position and time using a method called **trilateration**. [[1]][[2]]

    * _Trilateration_: Computes the distance from the satellites knowing net time and velocity (speed of light). The intersection of the 3 (or 4) spheres is your location
    ![alt text][Trilateration]

    * GPS devices require **three satellites** for an accurate calculation of position. Data from a fourth satellite further enhances the precision of the point's location, and also allows factors such as elevation to be calculated [[3]]

* Experience shows that there are usually at least 5 satellites visible most of the time and quite often there are 6 or 7 satellites visible [[2]]


# GPS Accuracy:

* dependent on signal-to-noise ratio (noisy reception), satellite position, weather and obstructions such as buildings and mountains [[1]]
    * Signal noise usually creates an error from around one to ten meters
    *  Mountains, buildings and other things that might obstruct the path between the receiver and the satellite can cause **three times as much error as signal noise**

    *  most accurate read of your location comes when you have a clear view of a clear sky away from any obstructions and under more than four satellites

* Methods exists to improve GPS Accuracy. Most notably are **Assisted GPS (AGPS),  Differential GPS (DGPS), SBAS (Satellite Based Augmentation Systems)**[[1]]
    * Will discuss in more detail later

## Gauging the Accuracy of a GPS Signal:
### PDOP, HDOP, VDOP [[20]]

* PDOP (Position Dilution of Precision) describes error caused by the relative position of the GPS satellites. 
    * Basically, the more signals a GPS receiver can “see” (spread apart versus close together), the more precise it can be.
    ![alt-text][PDOP1] ![alt-text][PDOP2]

    * HDOP - Horizontal Dilution of Precision
    * VDOP - Vertical Dilution of Precision

## Sources of Error:
### Signal-to-Noise Ratio (SNR) & C/N0

* SNR is usually expressed in terms of decibels. It refers to the ratio of the signal power and noise power in a given bandwidth [[25]
    * **S is the signal power**, usually the carrier power expressed in units of decibel/milliwatt (dBm) or decibel/watts (dBW);
    * **N is the noise power** in a given bandwidth in units of dBm or dBW. 
* ***The SNR is very useful when evaluating the performance of the acquisition and tracking stages in a receiver** [[25]]


* C/N0, on the other hand, is usually expressed in decibel-Hertz (dB-Hz) and refers to the ratio of the carrier power and the noise power per unit bandwidth. [[25]]
    *  C/N0 output by a receiver clearly provides an indication of the signal power of the tracked satellite and the noise density as seen by the receiver’s front-end. [[25]]

### Ionosphere (Atmosphere Refraction) 
    * 

### Multipath Effects
* occurs when the GPS satellite signal bounces off of nearby structures like buildings and mountains.

## Improving GPS Accuracy
### Differnetial GPS
[DGNSS][DGNSS] is a kind of _GNSS Augmentation system_ based on an enhancement to primary GNSS constellation(s) information by the use of a **network of ground-based reference stations** which enable the broadcasting of _differential information_ to the user to improve the accuracy of the position  

### SBAS (Satellite Based Augmentation Systems) [[22]]
* is an augmentation technology for GPS, which calculates GPS integrity and correction data with RIMS (Ranging and Integrity Monitoring Stations) on the ground and uses geostationary satellites (GEOs) to broadcast GPS integrity and correction data to GPS users.

* WAAS (Wide Area Augmentation System) is the SBAS System for North America.

* To improve position accuracy SBAS uses different types of correction data:
    * Fast Corrections for short-term disturbances in GPS signals (due to clock problems, etc).
    * Long-term corrections for GPS clock problems, broadcast orbit errors etc.
    * Ionosphere corrections for Ionosphere activity

* The most important SBAS feature for accuracy improvement is Ionosphere correction.

![alth-text][23]

### Other
* GPS can be augmented with additional satellites from GLONASS and GALILEO [[21]]
    * Simply there are more AVAILABLE satellites to choose from

# Almanac Data
## Overview

* Tells the GPS receiver where each GPS satellite should be at any time throughout the day and shows the orbital information for that satellite and every other satellite in the system. [[5]]
    * The almanac gives the approximate orbit for each satellite and is valid for long time periods **(up to 180 days)**. The almanac is used to predict satelite visibility and estimate the pseudorange to a satellite, thereby narrowing the search window for a ranging code. [[13]]
    
* It takes up to 12 1/2 minutes to collect a complete almanac after initial acquisition. **An almanac can be obtained from any GPS satellite**. [[13]]

## In-Depth:

* contains less accurate orbital information than ephemerides [[18]]
* valid for a period of up to **90 days** [[18]]
* Used to speed up time to first fix by 15 seconds (compared to not having almanac stored) [[18]]

* **Many sites give conflicting data**

# Ephemeris Data
## Overview

* needed to determine a satellite's position and gives important information about the health of a satellite, current date and time [[5]]
    * ephemeris must be collected from each satellite being tracked on acquisition and at least once every hour. Ephemeris information is normally **valid for 4 hours from the time of transmission**, and a receiver can normally store up to 8 sets of ephemeris data in its memory [[13]]
    * Depending on the NAV msg collection scheme employed in a particular receiver, it can take between **30 seconds and 3 minutes to collect the ephemeris information**. [[13]]

## In-depth

* contains information on week number, satellite accuracy and health, age of data, satellite clock correction coefficients, orbital parameters [[18]]
* valid two hours before and two hours after time of ephemeris (toe). The toe can be thought of as when the data was computed from the GNSS control segment [[18]]
    * Most sources simplify this to 4 hours of validity
* Used for real time satellite coordinate computation which is required in position computation [[18]]

## Purpose/Data Provided

* assists in the acquisition of satellites at power-up by allowing the receiver to generate a list of visible satellites based on stored position and time [[19]]
* relating time derived from the GPS (called GPS time) to the international time standard of UTC [[19]]
* allows a single-frequency receiver to correct for ionospheric delay error by using a global ionospheric model (Klobuchar Model) [[19]]

# Transmission of Almanac and Ephemeris Data  [[18]]

GNSS almanacs and ephemerides both form the navigation message transmitted by each satellite. The navigation message consists of **5 subframes**, where **each frame is made up of 10 words and takes up 6 seconds to download**. The navigation message frame structure is as follows:

subframes 1 -3: ephemerides
subframes 4-5: almanac --> full almanac requires 25 pages to completely download. 

Thus one page of frames 1-5 takes 30 seconds to download.
Since the **almanac contains 25 pages**, the total time required to download almanac is: 25 pages x 30 seconds/page = 750 seconds = **12.5 minutes**
On the other hand, the ephemerides takes 6 seconds/subframe x 5 subframes = **30 seconds to download**. 

**Note that the almanac is the same for all satellites whereas the ephemeris is unique to each satellite.**

# GPS Startup
## Overview

* _Start-up Times (Hot, Warm, and Cold)_: Some GPS modules have a super-capacitor or battery backup to save previous satellite data in volatile memory after a power down. This helps decrease the TTFF on subsequent power-ups. **Also, a faster start time translates into less overall power draw** [[1]]

    * _Cold Start_: If you power down the module for a long period of time and the backup cap dissipates, the data is lost. On the next power up, the GPS will need to download new almanac and ephemeris data. [[1]]

    * _Warm Start_: Depending on how long your backup power lasts, you can have a warm start, which means some of the almanac and ephemeris data is preserved, but it might take a bit extra time to acquire a lock [[1]]

    * _Hot Start_: A hot start means all of the satellites are up to date and are close to the same positions as they were in the previous power on state. With a hot start the GPS can immediately lock. [[1]]

## Cold Start

* The GPS starts up in this mode when: [[24]]
    * It is manually reset
    * Receiver has moved more than 60 Miles from the location of the previous fix.
    * Current time is inaccurate or unknown.
    * Incoming signal levels are marginal. The predicted satellites are physically overhead but the receiver cannot see them, for example due to tall buildings, foliage etc


## Warm Start

* The GPS starts up in this mode when: [[24]]
    * It has a valid Almanac.
    * The current location is within 60 Miles of the last fix location.
    * The current time is known (the GPS has been active in the last three days).
    * No Ephemeris data has been stored or it has become stale (see section 2).
    * 4 or more SVs with HDOP < 6 and good signal strength (ie the SVs have good geometry and there is an uninterrupted view of the sky) are visible and above the mask angle (section 5).
* The receiver can predict which SVs are overhead but needs to download current Ephemeris data. TTFF for this start mode is typically 45 seconds. 

## Hot Start

* The GPS starts up in this mode when: [[24]]
    * A fix has been established within the last 2 hours.
    * The GPS has valid Ephemeris data for at least 5 satellites.
* In this mode the receiver rapidly tracks the overhead SVs and needs to download a minimum of data to establish a position. TTFF for a hot start is typically 22 seconds


# GPS Message Format & Processesing:
## NMEA protocol 
* "Today in the world of GPS, NMEA is a **standard data format** supported by all GPS manufacturers, much like ASCII is the standard for digital computer characters in the computer world."[[11]]
    * This is a common data format that most GPS modules use. NMEA data is displayed in sentences and sent out of the GPS modules serial transmit (TX) pin. The NMEA sentences contain all of the useful data, (position, time, etc.). [[1]]

* What makes NMEA a bit confusing is that there are quite a few “NMEA” messages, not just one. So, just like there are all kinds of GPS receivers with different capabilities, there are many different types of NMEA messages with different capabilities. [[10]]

* In the NMEA standard there are no commands to indicate that the gps should do something different. Instead each receiver just sends all of the data and expects much of it to be ignored. There is no way to indicate anything back to the unit as to whether the sentence is being read correctly or to request a re-send of some data you didn't get[[10]]

## [UBX (UBLOX) protocol][22]
* From what I can gather, this protocol is more sophisticated than NMEA. It allows the GPS to send data pertaining to other aspects of the GPS modules, rather than just the actual GPS data. Additionally, you can send commands to the GPS module, allowing you to configure it.

* The following are a summary of the sort of configurations you can work with using UBX Protocol:
    * _Settings that are changed to reflect the setting that the GPS reciever will be situated in_
        * Examples include Platform settings (Portable, Stationary, Pedestrian), Static Hold, Utilization of SBAS (Satellite Based Augmentation Systems), Peak Current Settings. All of these settings can improve the GPS's ability to get a fix.

    *  _Power Management Settings_
        * Provides either the best possible performance or good performance with reduced power consumption.

    * _Jamming/Interference Indicator & Jamming/Interference Monitor (ITFM)_ 
        * Basically, it can tell you if GPS signal is being jammed by comparing currently measured spectrum (i.e. various relevant data) with a reference from when a good fix was obtained
        * **Could be used to determine if the GPS is underwater**

    * _Assisted GPS_
        * Users expect instant position information. With standard GPS this is not always possible because at least four satellites must transmit their precise orbital position data, called ephemeris, to the GPS receiver. Under adverse signal conditions, data downloads from the satellites to the receiver can take minutes, hours or even fail altogether.

        * Two options exist: Assisted GPS (A-GPS) & AssistNow Autonomous
            * **Assisted GPS (A-GPS)** boosts acquisition performance by providing data such as ephemeris, almanac, accurate time and satellite status to the GPS receiver via mobile networks or the Internet. This data can also be downloaded in advance and saved to non-volitile memory for the GPS reciever to access on startup. **The aiding data enables the receiver to compute a position within seconds, even under poor signal conditions.**

            * **AssistNow Autonomous** - Based on a broadcast ephemeris downloaded from the satellite (or obtained by AssistNow Online) the receiver can **autonomously** (i.e. without any host interaction or online connection) **generate an accurate satellite orbit representatio**n («AssistNow Autonomous data») that is usable for navigation much longer than the underlying broadcast ephemeris was intended for. This makes downloading new ephemeris or aiding data for the first fix unnecessary for subsequent start-ups of the receiver.




# Terminology: [[1]]
---
* _GNSS_: GNSS stands for Global Navigation Satellite System, and is the standard generic term for satellite navigation systems that provide autonomous geo-spatial positioning with global coverage. This term includes e.g. the GPS, GLONASS, Galileo, Beidou and other regional systems. GNSS is a term used worldwide [[17]

* _Channels_: The number of channels that the GPS module runs will affect your time to first fix (TTFF). Since the module doesn’t know which satellites are in view, the more frequencies/channels it can check at once, the faster a fix will be found. After the module gets a lock or fix, some modules will shut down the extra blocks of channels to save power. If you don’t mind waiting a little longer for a lock, 12 or 14 channels will work just fine for tracking. [[1]]

* _Chipset_: The GPS chipset is responsible for doing everything from performing calculations, to providing the analog circuitry for the antenna, to power control, to the user interface. The differences between chipsets usually falls on a balance between **power consumption, acquisition times, and accessibility of hardware**. [[1]]

* _Gain_: The gain is the efficiency of the antenna in any given orientation [[1]]

* _NMEA_: This is a common data format that most GPS modules use. NMEA data is displayed in sentences and sent out of the GPS modules serial transmit (TX) pin. The NMEA sentences contain all of the useful data, (position, time, etc.). [[1]]

* _Power_: GPS modules are not power hogs, but they do need some juice to number crunch the data from the satellites and to obtain a lock. On average, a common GPS module, with a lock, draws around 30mA at 3.3V. Also, keeping the start-up time low, saves power. [[1]]

* _Update Rate_: The update rate of a GPS module is how often it calculates and reports its position. The standard for most devices is 1Hz (once per second). Higher update rates mean there’s more NMEA sentences flying out of the module.[[1]]

Source [[2]] is quite informative and goes very in-depth reagrding how GPS's work

[1]: https://learn.sparkfun.com/tutorials/gps-basics/all
[2]: http://webarchiv.ethz.ch/geometh-data/downloads/GPSBasics_en.pdf
[3]: https://www.lifewire.com/trilateration-in-gps-1683341
[MESH Network Site]: https://nootropicdesign.com/projectlab/2018/10/20/lora-mesh-networking/
[GPS Receiver - GP-20U7 Specs]: https://cdn.sparkfun.com/datasheets/GPS/GP-20U7.pdf
[4]: https://lastminuteengineers.com/neo6m-gps-arduino-tutorial/
[NEO-6M GPS Chip Data Sheet]:https://lastminuteengineers.com/datasheets/NEO-6M-GPS-DataSheet.pdf
[NEO-6M GPS Chip]: https://www.u-blox.com/en/product/neo-6-series#tab-documentation-resources
[NEO-6M GPS Breakout Board]: https://www.amazon.com/DIYmall-AeroQuad-Antenna-Arduino-Aircraft/dp/B01H5FNA4K#customerReviewshttps://www.u-blox.com/en/product/neo-6-series#tab-documentation-resources
[5]: https://www.hackster.io/ruchir1674/how-to-interface-gps-module-neo-6m-with-arduino-8f90ad
[TinyGPS++]: http://arduiniana.org/libraries/tinygpsplus/
[NeoGPS]: https://github.com/SlashDevin/NeoGPS
[6]:https://www.arduino.cc/reference/en/language/variables/data-types/float/
[7]: https://www.arduino.cc/reference/en/language/variables/data-types/long/
[8]: https://github.com/SlashDevin/NeoGPS/blob/master/extras/doc/Data%20Model.md
[9]: https://www.sparkfun.com/pages/GPS_Guide
[Trilateration]: https://gisgeography.com/wp-content/uploads/2016/11/GPS-Trilateration-Feature-678x322.png
[10]: https://www.gpsinformation.org/dale/nmea.htm
[11]: https://www.gpsworld.com/what-exactly-is-gps-nmea-data/
[SparkFun_Ublox_Arduino_Library]: https://github.com/sparkfun/SparkFun_Ublox_Arduino_Library
[12]: http://gauss.gge.unb.ca/gpsworld/EarlyInnovationColumns/Innov.1990.03-04.pdf
[13]: https://www.navcen.uscg.gov/pubs/gps/gpsuser/gpsuser.pdf
[14]: https://lastminuteengineers.com/neo6m-gps-arduino-tutorial/
[15]: http://wiki.sunfounder.cc/index.php?title=Ublox_NEO-6M_GPS_Module
[NEO-6M GPS Board Schematic]: http://wiki.sunfounder.cc/images/f/f1/Gsdg.png
[16]: https://www.spirent.com/blogs/positioning/2010/october/2010-10-07_what-is-the-klobuchar-model
[17]: https://www.semiconductorstore.com/blog/2015/What-is-the-Difference-Between-GNSS-and-GPS/1550/
[18]: https://www.novatel.com/support/known-solutions/gnss-ephemerides-and-almanacs/
[19]: https://en.wikipedia.org/wiki/GPS_signals
[20]: https://gisgeography.com/gps-accuracy-hdop-pdop-gdop-multipath/
[PDOP1]: https://gisgeography.com/wp-content/uploads/2016/10/GPS-GDOP-good-425x302.png
[PDOP2]: https://gisgeography.com/wp-content/uploads/2016/10/GPS-GDOP-poor-340x425.png
[21]: https://the5krunner.com/2018/07/18/gps-accuracy-hdop-pdop-gdop-multipath-the-atmosphere/
[DGNSS]: https://gssc.esa.int/navipedia/index.php/Differential_GNSS
[22]: https://www.u-blox.com/sites/default/files/products/documents/u-blox7-V14_ReceiverDescriptionProtocolSpec_%28GPS.G7-SW-12001%29_Public.pdf
[23]: https://i.ibb.co/mvgY3kZ/1.png
[24]: https://www.measurementsystems.co.uk/docs/TTFFstartup.pdf
[25]: https://insidegnss.com/measuring-gnss-signal-strength/