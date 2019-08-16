# Setting Up This Project

This file describes how to get this setup working properly, from the hardware to the software.

# What You Need

## The Hardware You Need

What you'll need will of course depend on what kind of setup you're trying to achieve.

### Server

Leaving this blank cause maybe cloud?

### Gateway

For each Gateway, you'll need:

* One Arduino compatible micro controller. At least 32 kB of flash memory and 2kB of ram is highly recomended, although more is prefered. The [bla]() is a good option
* A 3G module of some sort. The current two supported are the [Tinysine 3G shield](https://www.tinyosshop.com/3g-gprs-gsm-shield-for-arduino-sim5320a) and the [Adafruit FONA 3G](https://www.adafruit.com/product/3147)
* A sim card with a data plan on it. Where to get this sim card will heavily depend on where you are
* A [LoRa mesh module](https://www.dfrobot.com/product-1670.html)
* A [microSD card module](https://www.amazon.ca/Senmod-Adapter-Reader-Module-Arduino/dp/B07GXBP672)
* A [microSD card](https://www.amazon.ca/SanDisk-Ultra-microSDXC-Adapter-SDSQUAR-128G-GN6MA/dp/B073K14CVB). It's recomended that it is at least 1GB, a breakdown of size to data points can be found in the [Data Storage Options file](https://gitlab.cas.mcmaster.ca/re-mote/publications/blob/master/VictorVezina19MeasurementDataStorage/Data_Storage_Options.md)
* Any sensors you want to connect to it, see the [sensor options section](#sensor-options) for more information
* An appropriate power option, see the [power options section](#power-options) for more information
* Some form of [eclosure](#enclosures)

As with everything in the setup, these parts could be changed out for others (ex: changing data storage, using a different 3G module, etc), the listed options are simply what is currently supported.

### End node

For each End node, you'll need:

* One Arduino compatible micro controller. At least 30kB of flash memory and 1.5kB of ram is highly recomended. The [bla]() is a good option
* A [LoRa mesh module](https://www.dfrobot.com/product-1670.html)
* A GPS module of some sort. The [bla]() is recomended
* Any sensors you want to connect to it, see the [sensor options section](#sensor-options) for more information
* An appropriate power option, see the [power options section](#power-options) for more information
* Some form of [eclosure](#enclosures)

Once again, these parts could be changed out for others, the listed options are simply what is currently supported.


### Sensor options

There are quite a few currently supported sensors, including some for water and air quality. The list is as follows:

* [Atlas Scientific Dissolved Oxygen Sensor](https://www.atlas-scientific.com/product_pages/kits/do_kit.html)
* [Atlas Scientific Conductivity Sensor](https://www.atlas-scientific.com/product_pages/kits/ec_k1_0_kit.html)
* [Atlas Scientific pH Sensor](https://www.atlas-scientific.com/product_pages/kits/ph-kit.html)
* [DFRobot Gravity Analog Turbidity Sensor](https://www.dfrobot.com/product-1394.html)
* [DFRobot Gravity Waterproof DS18B20 Sensor (Temperature)](https://www.dfrobot.com/product-1354.html)
* [DHT22 sensor (Air Temperature and Humidity)](https://www.amazon.ca/gp/product/B07CM2VLBK/)

A couple caveats: 

* When using any two (or more) Atlas Scientific sensors together, any Conductivity and pH sensors should use a [voltage isolator](https://www.atlas-scientific.com/product_pages/circuits/basic-ezo.html). A caveat to the caveat: when using a pH and Conductivity sensor together, only the pH sensor needs to be isolated. Read the EZO data sheets provided by Atlas Scientific for more information.
* The software is currently setup to where if you are using pH or Conductivity sensors, multiple sensors of the same type must either all be isolated or all not isolated (this should be respected through the first caveat anyway)
* Using multiple sensors on the Gateway may cause flash issues. If you plan on using lots of sensors on the Gateway, get a micro controller with enough flash (64kB+)
* The DHT22 sensor is not weather proof, so you must make sure that your enclosure accounts for this

You can also use your own sensors with this system, instructions on how to do this can be found [here](./Custom_Sensors.md).

### Power options

battery vs solar vs pluged in

### Enclosures

What kinds, needs, how, etc.


## The Software You Need

### Server stuff

Leaving this blank cause maybe cloud?

### Arduino IDE

First, you'll need the [Arduino IDE](https://www.arduino.cc/en/main/software). This will be used to upload the code from your computer to the Arduino compatible micro controller. If you want, you could also use the Arduino web editor, or any other IDE that supports uploading .ino files to Arduinos.

### Program file

Next, you'll need to download [bla.ino]() from this repository. This is the main program that runs on the Arduino.

### Libraries

You'll then need to download the libraries from this repository, and add some others from the IDE. First, copy all files found under [Libraries](https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/tree/master/VictorVezina19/Libraries) into your [Arduino library folder](https://www.arduino.cc/en/guide/libraries), replacing what's already there (if anything). You'll then need to install these other libraries using the IDE's built-in library manager:

* DHT_sensor_library
* NeoGPS
* OneWire
* SdFat

If you wish to run the program in "debug mode", you'll need to download the [MemoryFree library](https://github.com/maniacbug/MemoryFree).

<br>  

# Assembling the Hardware

## Server

Leaving this blank cause maybe cloud?


## Gateway


## End node

<br>  

# Configuring and Uploading the Software

## Server

Leaving this blank cause maybe cloud?


## Arduinos

The main configuration file, config.h, can be found under the re-moteConfig library file. This is the only file you need to change to configure the Arduinos (assuming you aren't adding custom support). You need to comment and uncomment out lines 15 and 16 depending on if you're uploading to a Gateway or an End node. You then need to change the rest of the settings in that file to accomodate for your particular setup. The settings in the file all have clear comments that explain what that setting does.

<br>  

# Testing

<br>  

# Deploying

<br>  

# Using the website

<br>  

# Troubleshooting?
