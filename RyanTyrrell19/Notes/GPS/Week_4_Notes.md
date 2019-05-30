# Week 4/5 Notes and Planner

## Outline (Remember to source sites and use authoritative sources)

1. Various factors relating to GPS
    * How GPS transmits data and what is being transmitted
    * What the hardware of the GPS modules offers
    * What the Arduino API offers (i.e. Libraries)
    * Power consumption in various situations (idle, transmitting data, etc.)
    * Limitations of GPS (signal penetration, antenna, etc.)
2. LoRa parameters
    * Review the paper relating to optimizing the LoRa parameters, summarize its finding to present
    * See what else is available for optimizing the LoRa parameters
        * **Using the MESH modules, what parameters can actually be changed?**
3. Research new Arduinos
    * Determine what external sensors (3D accelerometer, 3D gyroscope, etc.) may be of use
        * For example, if any can be used to detect when the turtles is underwater  
        * External modules include, but not limited to: 3D accelerometer, 3D gyroscope, 3D magnetometer, barometric pressure, environmental temperature, relative humidity, digital proximity ambient light, RGB and gesture sensors and a digital microphone
    * How exposed must the Arduino be to utilize these features?
4. Field work with the LoRa MESH Modules
    * Set up a protocol to test range, throughput, # lost packages, power used, etc.
    * Setting up this test should be similar to real-life applications (such as Cootes Paradise)
        * See what tests have already been conducted, or at least see if a testing protocol already exists
    * Use GPS and Google Maps to determine the distance

5. Brainstorm ideas to get the youth excited about technology. If able, somehow include turtles in the discussion
    * Idea: To make the data more exciting and meaningful, have it displayed in a fun way in real time. For example, [this site][MESH Network Site] displays the strength and connections of a MESH Network in real time. We can apply a similar approach, perhaps with the addition of diplaying GPS coordinates in realtime.
    * U-Center Display's **a lot** of data in a meaningful way (GPS data)


GPS Factors:
===
GPS Basics:
---
* satellites, equiped with extremely accurate atomic clocks, orbit the earth. The satellites continuously send data down to earth over dedicated RF frequencies. GPS receivers have tiny processors and antennas that directly receive the data sent by the satellites and compute your position and time [[1]]
* 24 satellites orbit the earth twice a day. The positions of the satellites are constructed in a way that the sky above your location will contain at least 4 satellites and, at most, 12 satellites. [[1]][[3]]
* To compute it's location, the GPS module recieves a timestamp from each of the visible satellites, along with data on where in the sky each one is located (among other pieces of data). If the GPS receiver’s antenna can see at least 4 satellites, it can accurately calculate its position and time using a method called **trilateration**. [[1]][[2]]
    * _Trilateration_: Computes the distance from the satellites knowing net time and velocity (speed of light). The intersection of the 3 (or 4) spheres is your location
    ![alt text][Trilateration]

    * GPS devices require **three satellites** for an accurate calculation of position. Data from a fourth satellite further enhances the precision of the point's location, and also allows factors such as elevation to be calculated [[3]]
* Experience shows that there are usually at least 5 satellites visible most of the time and quite often there are 6 or 7 satellites visible [[2]]

GPS Accuracy:
---
* dependent on signal-to-noise ratio (noisy reception), satellite position, weather and obstructions such as buildings and mountains [[1]]
    * Signal noise usually creates an error from around one to ten meters
    *  Mountains, buildings and other things that might obstruct the path between the receiver and the satellite can cause **three times as much error as signal noise**
    *  most accurate read of your location comes when you have a clear view of a clear sky away from any obstructions and under more than four satellites

* Methods exists to improve GPS Accuracy. Most notably are **Assisted GPS (AGPS),  Differential GPS (DGPS), SBAS (Satellite Based Augmentation Systems)**[[1]]
    * Will discuss in more detail later

Gauging the Accuracy of a GPs Signal:
---

Sources of Error:
---
* Ionosphere 
    * 

GPS Additional Info:
---

A bit of terminology to start, 

* _Almanac data_: tells the GPS receiver where each GPS satellite should be at any time throughout the day and shows the orbital information for that satellite and every other satellite in the system. [[5]]
    * The almanac gives the approximate orbit for each satellite and is valid for long time periods **(up to 180 days)**. The almanac is used to predict satelite visibility and estimate the pseudorange to a satellite, thereby narrowing the search window for a ranging code. [[13]]
    * It takes up to 12 1/2 minutes to collect a complete almanac after initial acquisition. An almanac can be obtained from any GPS satellite.[[13]]
    * A clock can also be kept operating when the receiver is off or in standby mode, so as to minimize initial acquisition time for the next start-up. [[13]]

* _Ephemeris data_: needed to determine a satellite's position and gives important information about the health of a satellite, current date and time [[5]]
    * ephemeris must be collected from each satellite being tracked on acquisition and at least once every hour. Ephemeris information is normally **valid for 4 hours from the time of transmission**, and a receiver can normally store up to 8 sets of ephemeris data in its memory [[13]]
    * Depending on the NAV msg collection scheme employed in a particular receiver, it can take between 30 seconds and 3 minutes to collect the ephemeris information. [[13]]

* _Start-up Times (Hot, Warm, and Cold)_: Some GPS modules have a super-capacitor or battery backup to save previous satellite data in volatile memory after a power down. This helps decrease the TTFF on subsequent power-ups. Also, a faster start time translates into less overall power draw [[1]]

    * _Cold Start_: If you power down the module for a long period of time and the backup cap dissipates, the data is lost. On the next power up, the GPS will need to download new almanac and ephemeris data. [[1]]

    * _Warm Start_: Depending on how long your backup power lasts, you can have a warm start, which means some of the almanac and ephemeris data is preserved, but it might take a bit extra time to acquire a lock [[1]]

    * _Hot Start_: A hot start means all of the satellites are up to date and are close to the same positions as they were in the previous power on state. With a hot start the GPS can immediately lock. [[1]]


GPS Message Format & Processesing:
---
* NMEA protocol 
    * "Today in the world of GPS, NMEA is a **standard data format** supported by all GPS manufacturers, much like ASCII is the standard for digital computer characters in the computer world."[[11]]
        * This is a common data format that most GPS modules use. NMEA data is displayed in sentences and sent out of the GPS modules serial transmit (TX) pin. The NMEA sentences contain all of the useful data, (position, time, etc.). [[1]]
    * What makes NMEA a bit confusing is that there are quite a few “NMEA” messages, not just one. So, just like there are all kinds of GPS receivers with different capabilities, there are many different types of NMEA messages with different capabilities. [[10]]
    * In the NMEA standard there are no commands to indicate that the gps should do something different. Instead each receiver just sends all of the data and expects much of it to be ignored. There is no way to indicate anything back to the unit as to whether the sentence is being read correctly or to request a re-send of some data you didn't get[[10]]

* [UBX (UBLOX) protocol][UBX-7 Protocol]
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




Terminology: [[1]]
---
* _GNSS_: GNSS stands for Global Navigation Satellite System, and is the standard generic term for satellite navigation systems that provide autonomous geo-spatial positioning with global coverage. This term includes e.g. the GPS, GLONASS, Galileo, Beidou and other regional systems. GNSS is a term used worldwide [[17]

* _Channels_: The number of channels that the GPS module runs will affect your time to first fix (TTFF). Since the module doesn’t know which satellites are in view, the more frequencies/channels it can check at once, the faster a fix will be found. After the module gets a lock or fix, some modules will shut down the extra blocks of channels to save power. If you don’t mind waiting a little longer for a lock, 12 or 14 channels will work just fine for tracking. [[1]]

* _Chipset_: The GPS chipset is responsible for doing everything from performing calculations, to providing the analog circuitry for the antenna, to power control, to the user interface. The differences between chipsets usually falls on a balance between **power consumption, acquisition times, and accessibility of hardware**. [[1]]

* _Gain_: The gain is the efficiency of the antenna in any given orientation [[1]]

* _NMEA_: This is a common data format that most GPS modules use. NMEA data is displayed in sentences and sent out of the GPS modules serial transmit (TX) pin. The NMEA sentences contain all of the useful data, (position, time, etc.). [[1]]

* _Power_: GPS modules are not power hogs, but they do need some juice to number crunch the data from the satellites and to obtain a lock. On average, a common GPS module, with a lock, draws around 30mA at 3.3V. Also, keeping the start-up time low, saves power. [[1]]

* _Update Rate_: The update rate of a GPS module is how often it calculates and reports its position. The standard for most devices is 1Hz (once per second). Higher update rates mean there’s more NMEA sentences flying out of the module.[[1]]

Source [[2]] is quite informative and goes very in-depth reagrding how GPS's work


 
 GPS Modules Available:
 ===

 [GPS Receiver - GP-20U7][GPS Receiver - GP-20U7 Specs]:
 * 56-Channel Receiver (22 Channel All-in-View)
 * Sensitivity : -162dBm
 * 2.5m Positional Accuracy
 * Cold Start : 29s (Open Sky)
 * 40mA @ 3.3V

 [This site][9] contains an extensive list of various GPS reciever and their highlights. A few have been summarized above.
 * 

[NEO-6M GPS Chip][NEO-6M GPS Chip Data Sheet]:
This GPS module continues to appear as an excellent choice for low-power, low-cost projects. A few f the key features are:
* _Power Save Mode(PSM)_: It allows a reduction in system power consumption by selectively switching parts of the receiver ON and OFF. This dramatically reduces power consumption of the module to **just 11mA** making it suitable for power sensitive applications like GPS wristwatch [[4]]

* The breakout board include a battery and EEPROM for memory storage. Utilizing these retains clock and last position, causing **time to first fix (TTFF) to reduce to 1s**.[[14]]
    * Without the battery the GPS always cold-start so the initial GPS lock takes more time [[14]]
    * The battery is automatically charged when power is applied and maintains data for up to **two weeks without power.** [[14]]

* Both modules support UBX and so are compatible with **U-center from u-blox** for evaluation, performance analysis and configuration of u-blox GPS receivers including NEO-6M

* The EEPROM on the Board communicates with the NEO-6M chip using I2C
* To save data (such as Assit Offline) an external memory (Flash) must be connected to the chips SPI header.

![alt text][NEO-6M GPS Board Schematic]

Arduino GPS Libraries:
===
Arduino GPS Libraries handle reading and parsing the data recieved from the attached GPS Module. 

[TinyGPS++][TinyGPS++]:
* Generally accepted as the "go to" library when handling GPS data

[NeoGPS][NeoGPS]:
* This library has been optmized to reduce the amount of RAM and CPU time utilized when parsing the GPS data
* This library has a greater learning-curve associated with it, due to the code being slighlty more complex
* Despite being not as popular as TinyGPS++, this library seems to have a great deal of Readme's and examples to quickly learn and start using this library
* Another feature of this library is that it can save the GPS lattitude and longitude as long values, rather than floats. Arduino float (and duble) types can only precisely store **6-7 digits** [[6]], while a long type can precisely approx. **9-10 digits** [[7]]. The library includes a function that parses and displays the long data to "appear" as though it were a float (i.e. a decimal value) [[8]]

[SparkFun_Ublox_Arduino_Library][SparkFun_Ublox_Arduino_Library]
* Library to control UBX binary protocol and NMEA over I2C on Ublox GPS modules


GPS Power Consumption - In-Depth Look:
===
General Notes:
---
* On average, GPS modlues draw around 30mA at 3.3V. Keep in mind, also, that GPS antennas usually enlist the help of an amplifier that draws extra power. If a unit appears to have super-low power consumption, make sure there's an antenna attached.[[9]]

* The time taken to get a fix is porportional to the number of channels a GPS can search, which in turn is proportional to power consumption

Limitations of GPS (signal pentration, antenna, etc.)
===
Signal penetration:
---
* GPS signals propagate from the satellites to the reciever antenna along the sight and cannnot pentrate water, soil, walls, or other obstacles very well [[12]]
* undwerwater: does not work for the same reason as LoRa; GPS uses RF Signal (i.e high frequency). High-freqency waves can effectively propogate, but not penetrate. 

Antenna:
---
* Additional cuurent drawn, large & heavy




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
[UBX-7 Protocol]: https://www.u-blox.com/sites/default/files/products/documents/u-blox7-V14_ReceiverDescriptionProtocolSpec_%28GPS.G7-SW-12001%29_Public.pdf
[SparkFun_Ublox_Arduino_Library]: https://github.com/sparkfun/SparkFun_Ublox_Arduino_Library
[12]: http://gauss.gge.unb.ca/gpsworld/EarlyInnovationColumns/Innov.1990.03-04.pdf
[13]: https://www.navcen.uscg.gov/pubs/gps/gpsuser/gpsuser.pdf
[14]: https://lastminuteengineers.com/neo6m-gps-arduino-tutorial/
[15]: http://wiki.sunfounder.cc/index.php?title=Ublox_NEO-6M_GPS_Module
[NEO-6M GPS Board Schematic]: http://wiki.sunfounder.cc/images/f/f1/Gsdg.png
[16]: https://www.spirent.com/blogs/positioning/2010/october/2010-10-07_what-is-the-klobuchar-model
[17]: https://www.semiconductorstore.com/blog/2015/What-is-the-Difference-Between-GNSS-and-GPS/1550/