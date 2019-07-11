# Overview
* Arduino Pro Mini
* Moteino


# Arduino Pro Mini



# [Moteino Micro-controllers][Moteino]

## Features
* Essentially an Arduino that has been modified specifically to draw minimal power. 
* designed to be a compact, highly customizable and affordable development platform, suitable for IoT, home automation and long range wireless projects
* Several different boards exist based on the processors used by the Arduino models
* Can be programmed using the Arduino IDE. May need to first install the Moteino Core. This defines the pin map and other important core definitions (required by the MEGA)

* true ultra low power: 
	* with just ~2uA achievable in deep sleep mode, AVR Moteinos allow battery friendly projects like wireless motion/environment sensors to run for years. 
	* Watchdog sleep mode is at ~6uA (periodic wake). 
	* The new 8Mhz Moteino allows the lowest possible 100nA sleep mode. 
	* The new SAMD21 Cortex M0+ based MoteinoM0 sleeps at just ~8uA!
* **wirelessly (aka OTA) programmable:**
	* be able to re-flash it without wires, when deployed in difficult locations **(supported with RFM69 radios only)**
	* The wireless programming protocol is generic and supported by all Moteinos with DualOptiboot and FLASH Memory option, but the **example implementation was only done for RFM69 transceivers**
* affordable
	* zero-cost wire monopole antennas yield excellent RF range for all-around wireless projects. 
	* With the u.FL/IPEX and SMA connector pads you can attach any other external antennas with higher gain

* Sketches must be uploaded through an FTDI Adapte


## Moteino (basic)

![alt-text][Moteino Board]  

![alt-text][Moteino Specs]


## Moteino M0

![alt-text][Moteino M0]

## MoteinoMEGA

![alt-text][MoteinoMEGA Board]

* uses the Atmega1284p microcontroller
* runs at 3.3V/16Mhz
* has 128KB of internal flash (vs 32KB on Atmega328p)
* **16KB of RAM** (vs 2KB)
* 4KB EEPROM (vs 1KB)
* 2x hardware serial ports
* 8x PWM pins and bunch more GPIO pins
* Still a small board at just 1.0×2.0″ and breadboard friendly.


![alt-text][MoteinoMEGA Specs]



## Moteino with LoRa

All Moteinos now accept these types of transceivers:
![alt-text][Transceivers Supported]

* two special Moteino variants with integrated PCB trace antennas, these are available for Moteino and MoteinoMEGA. 
	* The PCB antennas are tuned for the 868-915mhz bands and have an excellent performance given their compact size. 
	* Other than the PCB antenna, they are identical and work the same as regular Moteinos.
	* Small size – 1.0 x 2.6 inch

![alt-text][Moteino Board with Trace Antenna]
![alt-text][Moteino & MoteinoMEGA Board with Trace Antenna]


## M0 Sensor Shields
There are now a few compact low-power sensor breakouts available that can flat-mount straight on the M0 PCB (bottom side).

* BME280 Breakout
	* This breakout includes a [BME280 sensor][BME280 sensor] (Humidity Pressure Sensor)
* LSM9DS1 9DoF IMU Breakout
	*  includes 3 sensors in 1 chip – 3-axis accelerometer, 3-axis gyroscope and 3-axis magnetometer!
* SD-card Logger Breakout
	* mini SD-card breakout which includes a power switch to put your SD-Card to sleep




[Moteino]: https://lowpowerlab.com/guide/moteino/
[Moteino Board]: https://i.ibb.co/vdd2MXs/Moteino-Board.png
[Moteino Specs]:https://i.ibb.co/6Jp4CgZ/Moteino-Specs.png
[MoteinoMEGA Board]: https://i.ibb.co/9bF4cCT/Moteino-MEGA-Board.png
[MoteinoMEGA Specs]: https://i.ibb.co/Ss6jM74/Moteino-MEGA-Specs.png
[Moteino M0]: https://i.ibb.co/dr53PPF/Moteino-M0-Board.png
[BME280 sensor]: https://www.adafruit.com/product/2652
[Transceivers Supported]: https://i.ibb.co/c8Ms2W1/Transceivers-Supported.png
[Moteino Board with Trace Antenna]: https://i.ibb.co/sg7LxYL/Moteino-Board-with-Trace-Antenna.png
[Moteino & MoteinoMEGA Board with Trace Antenna]: https://i.ibb.co/WpCgy6B/Moteino-Moteino-MEGA-Board-with-Trace-Antenna.png