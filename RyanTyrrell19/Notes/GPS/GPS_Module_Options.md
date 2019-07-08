* Price range?
* Probably better off buying a pre-made board rather than design our own PCB. A lot of things that need to be considered when designing your own board
 * Include key, important feature relevant to the project, and provide a link to the datasheets

 # What we're looking for
* Cheap
    * < $50
* Capable of backing up RTC and aided data
* Various mode of operation for altering power consumption
* Configurable
* Low Current consumption
    * < 50mA
* Quick fix
    * < 30 sec

 # [GPS Receiver - GP-20U7][GPS Receiver - GP-20U7 Specs]:

* Dimensions: 18.4mm x 18.4mm x 4mm
* Weight:  


 * 56-Channel Receiver (22 Channel All-in-View)
 * Sensitivity : -162dBm
 * 2.5m Positional Accuracy
 * Cold Start : 29s (Open Sky)
 * 40mA @ 3.3V


* Does contain a RX header so that commands can be sent to it (although it's been covered by tape and a ire would need to be soldered to it)
* Despite having a battery, it does not appear as though config or aided data is saved.

# NEO-6M GPS Chip:

* Dimensions: 28mm x 28mm x 8.6mm
* Weight:
    * Chip: 
    * Board:

* [NEO-6M GPS Chip][NEO-6M GPS Chip]
* [NEO-6M GPS Board][NEO-6M GPS Board]

![alt text][NEO-6M GPS Board Schematic]


This GPS module continues to appear as an excellent choice for low-power, low-cost projects. A few of the key features are:

* _Power Save Mode(PSM)_: It allows a reduction in system power consumption by selectively switching parts of the receiver ON and OFF. This dramatically reduces power consumption of the module to **just 11mA** making it suitable for power sensitive applications like GPS wristwatch [[4]]

* The breakout board include a battery and EEPROM for memory storage. Utilizing these retains clock and last position, causing **time to first fix (TTFF) to reduce to 1s**.[[3]]
    * Without the battery the GPS always cold-start so the initial GPS lock takes more time [[3]]
    * The battery is automatically charged when power is applied and maintains data for up to **two weeks without power.** [[3]]
        * **Battery is not a reliable source. Too many unknowns, such as lifespan, recharge time, etc.**

* Both modules support UBX and so are compatible with **U-center from u-blox** for evaluation, performance analysis and configuration of u-blox GPS receivers including NEO-6M

* The EEPROM on the Board communicates with the NEO-6M chip using I2C
* To save data (such as Assit Offline) an external memory (Flash) must be connected to the chips SPI header.


# VK2828U7G5LF GPS

* Dimensions: 28mm x 28mm x 8.6mm
* Weight:
    * Chip: 
    * Board:

* [VK2828U7G5LF GPS Chip][VK2828U7G5LF GPS Chip]
* [VK2828U7G5LF GPS Board][VK2828U7G5LF GPS Board]

* This GPS contains an EN pin
* Supports UBLOX (So it is configurable)
* An RTC is built into the module, with an endurance of 2 hours

# [Adafruit Ultimate GPS][Adafruit Ultimate GPS]

* [Adafruit Ultimate Pinout][Adafruit Ultimate Pinout]
* [Adafruit Ultimate Battery Backup][Adafruit Ultimate Battery Backup]

* Consumption current(@3.3V):
    * Acquisition: 25mA Typical
    * Tracking: 20mA Typical
* RTC battery-compatible
* Built-in **datalogging**
* Internal patch antenna + u.FL connector for external active antenna
* Contains:
    * ultra-low dropout 3.3V regulator so you can power it with 3.3-5VDC in, 5V level safe inputs
    * ENABLE pin so you can turn off the module using any microcontroller pin or switch
    * footprint for optional CR1220 coin cell to keep the RTC running and allow warm starts

* Supports **PMTK command packet** 
    * Custom commands created by Adafruit (I believe)
    * Although it does not support UBLOX, it does still support many of the features that UBLOX provides (just given a different name)

* [Commands][Adafruit Ultimate Commands]
    * Looks like custom commands (similar to UBLOX) for configuring
    * Seems to be capable of storing aided data, such as position, almanac data, etc
    * Supports:
        * Hot, Warm, & Cold Forced Resets
        * SBAS & DGPS Correction
        * Standby mode for power saving
        *  PMTK_SET_AL_DEE_CFG
            * "It means the module needs to extend the time for ephemeris data receiving under what situation."
            * Coulod be used to monitor emphemeris download status?
        * PMTK_CMD_PERIODIC_MODE 
            * Enter Standby or Backup mode for power saving
            *  operation mode
                * ‘0’ = go back to normal mode
                 * ‘1’ = Periodic backup mode
                 * ‘2’ = Periodic standby mode
                 * ‘4’ = Perpetual backup mode
                 * ‘8’ = AlwaysLocateTM standby mode
                 * ‘9’ = AlwaysLocateTM backup mode 
* Looks like you can force it into Backup mode, alonmg with some other power saving options
* Supports a variation of Assit Autonomous to extrapolat ephemeris data
* VBACKUP - Backup Power Input for RTC & Navigation Data Retention


# Notes about to GPS Units in General

* **EN Pin** - When this pin is connected to GND, the GPS is powerd down (even if the VCC pin is connected to power.) This method can allow us to connect the GPS directly to a power line, avoiding the need for a transistor. Simply connect the EN pin to a digital pin, and set the output accordingly (LOW Poer mode may automatically set all pins to LOW, so it may not even be necessary to set the output of the pin)



[GPS Receiver - GP-20U7 Specs]: https://cdn.sparkfun.com/datasheets/GPS/GP-20U7.pdf

[NEO-6M GPS Chip]: https://www.u-blox.com/en/product/neo-6-series#tab-documentation-resources
[NEO-6M GPS Board]: https://lastminuteengineers.com/neo6m-gps-arduino-tutorial/
[NEO-6M GPS Board Schematic]: http://wiki.sunfounder.cc/images/f/f1/Gsdg.png

[VK2828U7G5LF GPS Chip]: https://www.u-blox.com/en/product/ubx-m8030-series#tab-document-resources
[VK2828U7G5LF GPS Board]: https://abra-electronics.com/wireless/gps/modules/vk2828u7g5lf-ttl-ublox-gps-module-with-antenna.html

[Adafruit Ultimate GPS]: https://learn.adafruit.com/adafruit-ultimate-gps/overview
[Adafruit Ultimate Pinout]: https://learn.adafruit.com/adafruit-ultimate-gps/pinouts
[Adafruit Ultimate Battery Backup]: https://learn.adafruit.com/adafruit-ultimate-gps/battery-backup
[Adafruit Ultimate Commands]: https://cdn-shop.adafruit.com/datasheets/PMTK+command+packet-Complete-C39-A01.pdf


[1]: https://lastminuteengineers.com/neo6m-gps-arduino-tutorial/
[2]: https://www.sparkfun.com/pages/GPS_Guide
[3]: https://lastminuteengineers.com/neo6m-gps-arduino-tutorial/
[4]: https://learn.sparkfun.com/tutorials/gps-basics/all

