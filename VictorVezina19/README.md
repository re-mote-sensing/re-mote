# Setting Up This Project

This file describes how to get this setup working properly, from the hardware to the software.

# Setting up the server

To setup a custom server on a [Raspberry Pi](https://www.buyapi.ca/product/raspberry-pi-4-model-b-1gb/), follow the instructions found [here](https://gitlab.cas.mcmaster.ca/re-mote/pi-server/blob/master/Documentation/RaspberryPi_Instructions.md).

For instruction on using a cloud service, look [here]().

# What You Need

## The Hardware You Need

What you'll need will of course depend on what kind of setup you're trying to achieve.

### Gateway

For each Gateway, you'll need:

* One Arduino compatible micro controller. At least 32 kB of flash memory and 2kB of ram is highly recomended, although more is prefered. The [Arduino Uno](https://store.arduino.cc/usa/arduino-uno-rev3) seems to work well
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

* One Arduino compatible micro controller. At least 30kB of flash memory and 1.5kB of ram is highly recomended. The [Arduino Nano](https://store.arduino.cc/usa/arduino-nano) seems to work well
* A [LoRa mesh module](https://www.dfrobot.com/product-1670.html)
* A GPS module of some sort. The [GP-735](https://www.sparkfun.com/products/13670) is very good, although expensive
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

* When using any two (or more) Atlas Scientific sensors together, any Conductivity and pH sensors should use a [voltage isolator](https://www.atlas-scientific.com/product_pages/circuits/basic-ezo.html). A caveat to the caveat: when using a pH and Conductivity sensor together, only the pH sensor needs to be isolated. Read the EZO data sheets provided by [Atlas Scientific](https://www.atlas-scientific.com/) for more information.
* The software is currently setup to where if you are using pH or Conductivity sensors, multiple sensors of the same type must either all be isolated or all not isolated (this should be respected through the first caveat anyway)
* Using multiple sensors on the Gateway may cause flash issues. If you plan on using lots of sensors on the Gateway, get a micro controller with enough flash (64kB+)
* The DHT22 sensor is not weather proof, so you must make sure that your enclosure accounts for this

You can also use your own sensors with this system, instructions on how to do this can be found [here](Documentation/Custom_Sensors.md).

### Power options

Details on power consumption of [End-Node](https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/blob/master/HarneetSingh19/Power%20Consumption%20Analysis%20of%20Nodes/End-Node_POWER_CONSUMPTION.md) and [Gateway-Node](https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/blob/master/HarneetSingh19/Power%20Consumption%20Analysis%20of%20Nodes/Gateway_Node_POWER_CONSUMPTION.md).

### Enclosures

Check out the details of [solar panel](https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/tree/master/HarneetSingh19/Enclosure%20-%203D%20Printing%20Model/Solar%20Power%20Enclosure), [end-node](https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/tree/master/HarneetSingh19/Enclosure%20-%203D%20Printing%20Model/End%20Node) and [gateway-node](https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/tree/master/HarneetSingh19/Enclosure%20-%203D%20Printing%20Model/Gateway%20Node) housing units. Download .stl files [here](https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/tree/master/HarneetSingh19/Enclosure%20-%203D%20Printing%20Model).


## The Software You Need

### Arduino IDE

First, you'll need the [Arduino IDE](https://www.arduino.cc/en/main/software). This will be used to upload the code from your computer to the Arduino compatible micro controller. If you want, you could also use the Arduino web editor, or any other IDE that supports uploading .ino files to Arduinos.

### Program file

Next, you'll need to download [remoteNode.ino](remoteNode/remoteNode.ino) from this repository. This is the main program that runs on the Arduino.

### Libraries

You'll then need to download the libraries from this repository, and add some others from the IDE. First, copy all files found under [Libraries](https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/tree/master/VictorVezina19/Libraries) into your [Arduino library folder](https://www.arduino.cc/en/guide/libraries), replacing what's already there (if anything). You'll then need to install these other libraries using the IDE's built-in library manager:

* DHT_sensor_library
* NeoGPS
* OneWire
* SdFat

If you wish to run the program in "debug mode", you'll need to download the [MemoryFree library](https://github.com/maniacbug/MemoryFree).

<br>  

# Assembling the Hardware

## Gateway


## End node

<br>  

# Configuring and Uploading the Software

## Gateway and End Node

The main configuration file, config.h, can be found under the re-moteConfig library file. This is the only file you need to change to configure the Arduinos (assuming you aren't adding custom support). You need to comment and uncomment out lines 15 and 16 depending on if you're uploading to a Gateway or an End node. You then need to change the rest of the settings in that file to accomodate for your particular setup. The settings in the file all have clear comments that explain what that setting does.

<br>  

# Testing

It's important to test your setup at every stage of this document to make sure everything is functioning correctly. It is also highly recommended that once everything is finished, you run a full test over the course of a few days. This will ensure that everything is functioning as expected. If at some stage in the setup proccess something does not work, please refer to the [Troubleshooting section](#troubleshooting).

<br>  

# Deploying

Once everything is completely ready to be deployed, it is important to deploy them correctly. The first step is to choose your deployment location carefully. The LoRa mesh modules used allow for your nodes to be set up in a mesh configuration, which means that as long as a node (node 1) is within range of any other node (node2) that is connected to a gateway, the first node (node 1) will be able to connect to the gateway. This allows for a long string of end nodes connected to each other and only one gateway. The tested range of these LoRa mesh modules is about 200m through heavy obstruction (hill, buildings, etc.), about 500m through light obstruction (trees, etc.), and 650m through very light obstruction (chain link fences, nothing, etc.). This range can be improved by lifting the antenna, but by how much depends heavily on your circumstances (tested up to about +100m). It is also important to consider the legality of your deployment locations; you must ensure that you are not breaking any laws by deploying your nodes in the locations.

The next thing to consider when deploying your nodes is hardware safety. It is important to make sure that your nodes will not be dislodged or washed array by heavy rains, strong winds, or snow (depending on the weather where you're deploying). Depending on your deployment locations, it may also be important to consider hiding/securing your nodes in some way to prevent vandalism. This could be done by burying your nodes (although this may affect GPS and LoRa connectivity), painting them to blend in, or covering them with something (could be leaves, or even trash, etc.).

Once the nodes have been secured, it is important to deploy the sensors so that they are secure. For us, this involved putting a large steel pole into the water, and securing the sensor probes to it using water proof tape. The steps to do this will depend on the location and sensor probes you are using.

<br>  

# Using the website

To use your website, either enter the IP address of your personal server into a web browser (for your own server), or go to the website that hosts your website (for a cloud service). You may need to add a /# after the IP/URL. Once there, you should be able to see a map with the locations of all the nodes you have setup. To select a node, click on its pin on the map or select it from the Location drop-down menu. You can then select the measurement you wish to see the data for in the Sensor Type drop-down menu. Sensor Types in black are available at that node, Sensor Types in gray are not.

To see a list of nodes that contain a certain measurement, first unselect a location from the Location drop-down menu, then select the measurement you wish to use. When you expand the Location drop-down menu, the locations in black contain that measurement, and the locations in gray do not.   You may also notice that when you have a location selected, it will create a path with circles on the map. Each circle represents GPS co-ordinates where that node was located.

Once you have both a Location and Sensor Type selected, a table containing the date, time, and value of every data point will appear under the map. Underneath the table you will see a graph representing the sensor data through time.

<br>  

# Troubleshooting

The first step to troubleshooting is to make sure you've configured and connected everything correctly. If that didn't help, the next step is to identify the problem. This is most easily done by isolating parts of the Arduino and seeing which ones function correctly and which ones do not. If you've been testing after every step of this document, this should be quite easy. Once you've isolated the problem (let's say your DO sensor isnt initialising), it is good practice to re-check the configuration for this module to make sure that isn't the cause of the problem. If that wasn't the problem, you then need to determine if the problem is with the module or the Arduino. This is most easily done if you have another Arduino or sensor to swap in. If not, you can try to change the pins the sensor is connected to (ideally to pins you know work). If the problem is with the sensor, you may have gotten a defective one and will have to contact the seller. If the issue is with the Arduino, you can try changing the pins the sensor is connected to to see if some of the pins are broken. If that doesn't help, try testing the Arduino with a simple LED blink sketch. If the module and Arduino work seperately, but not together, you will have to do proper software bug finding and fixing, for which it is recommended that you have some form of experience with software development, preferrably in Arduino or C/C++.
