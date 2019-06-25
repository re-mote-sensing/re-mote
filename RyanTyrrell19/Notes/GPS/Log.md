# Outline
    * Read/write UBX commands to/from Arduino
    * Connecting GPS to Hardware Serial, and still viewing debugging data
    * Does the GPS have an RTC?
    * Evidence that the Watchdog Timer (used when the Arduino enters LowPower.Sleep() mode) is indeed inaccurate
    *
    * Rather then use GPS, setup up an abundant supply of Gateways and use them to triangulate the location of the Nodes
    * Can LoRa transmit underwater?
    * What approach did the original paper take in optimizing battery life?
    * LoRa is high frequency, which makes it good at propagation, but bad at penetration. Will this be an issue when waterproofing the Tracker
    *
    *


# Read/write UBX commands to/from Arduino

* The GPS receiver can output NMEA, PUBX or the binary UBX protocol
    * NMEA is the standard protocol with basic GPS data
    * PUBX (Proprietary UBX Messages) is essentially the same as NMEA (in terms of data available). The only difference is how the data is formatted for quicker transmission
    * Binary UBX protocol relates to all the various commands you can send, and the multitude of data you may recieve. T

* There are limited resources on how to read and write UBX commands using the Arduino. Most sites limit themselves to reading only the GPS data using UBX.

* I2C buffer is limited to 32 bytes (not sure if this value is programmable)

## Potential Libraries

* https://github.com/AuraUAS/UBLOX
    * Was unable to get this library to work. I belive this is because it is only compatible with UBLOX 7 or 8
* https://github.com/sparkfun/SparkFun_Ublox_Arduino_Library
    * This library is tailored to specific Sparkfun GPS modules


# Connecting GPS to Hardware Serial, and still viewing debugging data [[1]] [[2]]

* It is recommended to connect the GPS receiver to the Hardware Serial pins of the Arduino. Unfortunately, the Arduino Uno, Nano, and Pro Mini only have one, and it is used for viewing debugging data via the Serial Monitor
* To remedy this,
    * The GPS will use the Hardware Serial Pins
    * Using an FTDI-to-USB adapter, connect the Arduino to the computer.
    * AltSoftware Serial is the next best library to Hardware Serial, and so it will be used for receiving debugging info
        * on the UNO, Nano, and Mini, the RX/TX are Pins D8/D9, respectively
    * Open putty, select the correct COM port, and read the debugging messages
* UPDATE: As of right now, the GPS is connected to the Hardware pins, the debug messages are being output through the standard connector, and no issues are arising
## Conclusion
* You can debug and read GPS data from the Hardware pins, the data may conflict. It is safer to debug using AltSoftware (or Software Serial) and read the debug messages using putty

## Resources
[1]: https://ava.upuaut.net/?p=757  
[2]: https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html

# Does the GPS have an RTC?

* Theory: Yes, it does have an RTC. For it to work, it requires
    * a GPS fix so it knows what to set the time to
    * a constant supply of power to continue running (i.e. a cell battery)

* https://www.u-blox.com/sites/default/files/products/documents/LEA-NEO-MAX-6_HIM_%28UBX-14054794%29_1.pdf
    * "In case of a power failure on pin VCC, the real-time clock and backup RAM are supplied through pin V_BCKP.
This enables the u-blox 6 receiver to recover from a power failure with either a Hotstart or a Warmstart
(depending on the duration of VCC outage) and to maintain the configuration settings saved in the backup
RAM. If no backup battery is connected, the receiver performs a Coldstart at power up." 
    * **For the NEO-6M, backup battery current is 22µA (from the datasheet)**

# Evidence that the Watchdog Timer (used when the Arduino enters LowPower.Sleep() mode) is indeed inaccurate
* https://forum.arduino.cc/index.php?topic=425222.0
    * “I'm going to tell you this though, the watchdog timer SUCKS for accuracy. I found that out on a project I did for school. The frequency varies significantly with supply voltage and temperature, so it should not be used for anything requiring any degree of accuracy.”
* https://jeelabs.org/2012/03/27/tracking-time-via-the-watchdog/
    * "**This is only 10% accurate**, because it uses the hardware watchdog which is based on an internally RC-generated 128 KHz frequency."
    * note how 8192 ms ends up being 8255 ms, due to the watchdog timer inaccuracy."
* https://forum.arduino.cc/index.php?topic=49549.0
    * "The watchdog rate changes based on the power passing through the chip.  Going from idle to power down, the watchdog will run at a different rate.  Turn on a few digital outputs; different rate.  The watchdog timer is also sensitive to the ambient temperature and the supply voltage.  Even changing the watchdog timer settings alters the rate.
In addition, the watchdog timer has a lot of jitter.  Under the exact same conditions, the timer will expire at slightly different times"

* Timer used while the Arduino is in Low Power Mode. This timer is know to be very inaccurate, with it fluctuating depending on the input voltage and temperature.


# GPS Module VK2828U7G5LF comes with an RTC with a lifespan of 2 hours. If the GP were to turn on every hour for 10 seconds, would it extend the lifespan of the RTC?
* Unfortunately, no. Can try extending the time the GPS run-time or shorten the interval where it's powered off.
* Theory is that the RTC is powered via a capacitor, and so the capacitor needs time to recharge when connected to a power source. This in not included in the data sheet, so little can be done about proving/testing this.

# If looking into noise again, start here (may be useful for figuring out what antenna to use)
https://ava.upuaut.net/?p=836


# Rather then use GPS, setup up an abundant supply of Gateways and use them to triangulate the location of the Nodes
* Although a viable solution, it does not seem like the best one; especially for tracking a non-static object

* http://orbit.dtu.dk/ws/files/130478296/paper_final_2.pdf
    * This paper, titles “GPS-free Geolocation using LoRa in Low-Power WANs”, pursues the possibility of geolocation with LoRa. This tested used 1 non-moving node and 4 gateways
    * Taken from the paper are the following,
        * “it can be feasible to locate a device in a static spot with an accuracy of around 100 meters. However, for a real-time tracking application it can only be seen as a first approach, and not as a usable one”
        * “Another problem is the multipath of the signal, which causes wrong measurements of the TDOAs. Gateways do not always receive the direct path of the signal due to the reflections with terrestrial objects like buildings, forests or mountains. The ability to resolve this phenomenon depends on the bandwidth of the signal. If the bandwidth is large, the resolution is better and vice versa. The bandwidth employed in LoRa is small (125 kHz), so the recorded times in the gateways can be the time of a multipath signal instead of the direct one.”
* https://www.link-labs.com/blog/lora-localization
    * This website discusses “using 3 or more gateways to make a time difference of arrival (TDOA) calculation on the received LoRa signal and calculating the position.”
    * Taken from the site,
        * “our professional expertise leads us to the conclusion that accurate localization using LoRa (or any low power, narrowband, RF technology- Sigfox, etc.) is extremely difficult or impossible to successfully develop into a usable approach”
        * Visit the site for more info; it is well laid out and effectively concludes each topic. It’s quite informative and descriptive in discussing how feasible (or not feasible) geolocation using LoRa is

## Concluding Thoughts

Yes, it looks as though this is possible. However, no examples exists, only theories. Various techniques are discuses that are fairly complex. To implement this method would in itself be a large project with undetermined possibility of success.



# Can LoRa transmit underwater?
* Why is this a concern in the first place? If underwater, GPS cannot get a fix so LoRa need not bother transmitting)
* LoRa is just high-frequency radio waves. So, the question is, can high-frequency radio waves effectively transmit underwater? In addition, could the properties that makes LoRa different (chirp spread spectrum and such) influence this?
* https://www.thethingsnetwork.org/forum/t/underwater-range-of-lora/599
    * A forum discussing LoRa underwater (saltwater). The gist of it is that it doesn’t seem capable.
* https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4934316/
    * A very technical paper titles “RF Path and Absorption Loss Estimation for Underwater Wireless Sensor Networks in Different Water Environments”
    * Taken from the paper
        * “for absorption losses to be less than 3 dBs at water conductivity of 0.001 S, the radios ranging from 1 × 108 Hz or greater can be used”

## Concluding Thoughts

I believe it to be possible, but it depends. (I believe) as long as the volume of water is not too great, the signal can pass through it. Once free of the water, the signal can travel normally, but it’s power may be significantly reduced.



# What approach did the original paper take in optimizing battery life?

This was not the focus of the paper. Its main concern was the GPS accuracy and ability to log the data. It does not go into detail describing power consumption or acquiring GPS fixes at specific times.

# LoRa is high frequency, which makes it good at propagation, but bad at penetration. Will this be an issue when waterproofing the Tracker
* Initial thoughts, I would say yes. Depending on how the Tracker is waterproofed will most likely reduce the range of the Tracker

