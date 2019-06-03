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

Additional Resources:
===
Site to purchuse NEO-6M and via various datasheets
https://www.u-blox.com/en/product/neo-6-series#tab-product-selection



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