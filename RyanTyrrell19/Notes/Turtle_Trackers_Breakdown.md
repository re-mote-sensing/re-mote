# Outline

This outline provides a condensed overview of all the research that has been done on these trackers. It includes:  

* Summary of each component, what it does, and any additional, in-depth info that is relevant to this project
*  The requirements of each component. A sort of checklist to use when deciding what to purchase

* Keep in mind the following constraints
	* Weight
	* Size
	* Power consumption
	
# Hardware Breakdown
## GPS
## Description


### General
* Cheap - less than $50
* Low Current consumption - max of 50mA in Acquisition mode. 40mA is ideal
* Quick fix - 30 sec. is ideal (i.e. a Warm start)


### Specifics
* Supports a V_BACK pin, that can be connected to the Arduino
	* Could use a cell battery, but Arduino is more reliable
* Capable of **reliably** backing up RTC and aided data
	* Doing so allows the GPS to perform a warm start, cutting down on the TTFF
* Supports an EN pin
	* EN pin, when set to low, powers the GPS off. Effectively the same as using a transistor, without the transistor 
* Supports various modes of operation for altering power consumption
* Configurable - Using UBLOX or some other protocol that supports configurations
* Connector to attach external antenna
	* Allows us to choose which antenna to use

### Antenna
* This component is by far the heaviest. Choosing a smaller size will reduce weight, but may increase TTFF, thereby reducing battery life
* Orientation matters

## LoRa
### LoRa MESH Modules

### Antenna

### LoRa settings

## Other

* Battery


# Software Breakdown

# Code Overview

* Run Setup
	* Serial Setup
	* NeoGPS Setup
	* Configure GPS
	* Configure LoRa
* Loop
	* Sleep
		* If RTC, have Tracker run 30min intervals from Time A to Time B, sleeping for the rest
		* No RTC, run at 1 hour intervals
	* Power on
	* Send LoRa transmission with dummy load
		* No ACK, underwater or out of range - Power back down
		* ACK - continue operation
	* Power GPS on and let it get a fix (algoithm to determine time to get a fix?)
		* If RTC - GPS will Warm Start, should take 30sec
		* No RTC - Always cold start, should take 1min
	* Self-correct time using RTC (if available)
	

# Library Choices
 
* SoftwareSerial
* AltSoftSerial
* LowPower
* NMEAGPS


## Setup

* Initilize Serial Connections
* 

## Loop


## RTC and Low Power

Assuming the Tracker has an RTC, and the RTC is **guaranteed** to always be running, the Tracker can run and sleep during specific time intervals, for specific periods of time.  
For example, the Tracker can be programmed to wake at 10am, run every 30min, and sleep from 4pm to 10am.  
It would even be possible to program 3 or more different running time intervals

The following variables must be defined:  
* Time A  
* Time B  
* Short Interval Time  
* Long Interval Time  
* 
The following explains how this may be implemented:

* The Arduino sleeps using the LowPower Library and uses the (inaccurate) Watchdog Timer
* Every so often (every 30min?), the Arduino wakes up, checks the RTC (and converts it to the correct time zone), 
	* Keep in mind the GPS must be turned on briefly to read the RTC
	* It's been said that the Watchdog timer is +-10% off. A time could be computed and 10% of that time b


# GPS Fix Time

With the ability to store aiding data such as almanac, ephemeris, position, RTC, etc. the TTFF can be reduced significantly. However,knowing how long to keep the GPS running to collect this data while also not wasting power is difficult.  
The following is a possible implementation to optimize TTFF and current consumption:

* The GPS originally runs for 12.5 minutes to collect all almanac data (this is assuming this data can be stored and **guaranteed** to not be deleted. 

* The allowable fix time of the GPS can be computed 3 different ways:
	* Knowing the probability of when TTFF is likely to be acquired, average download time of ephemeris data, and computation time of Assist Autonomous feature; a rough idea of the turnaround time can be computed and used to determine the allowable run time of the GPS.
	* Alternatively, data can be read from the GPS that can indicate the status of certain components. Based on what is read, a time may be predicted for when the necessary data will be downloaded
	* Lastly, a good chunk of the time the Tracker is deployed will be spent underwater where the GPS cannot get a fix. For every time this occurs, increment a counter. This counter can be used to alter the allowable fix time, at no extra cost to power consumed

Final Thoughts:
* It takes 30sec to download ephemeris data. If the GPS module is advertised to get a TTFF from warm start in 30sec, downlaoding ephemeris data is not worth it
* As long as a warm start is possible (i.e. RTC and position data is saved), TTFF should be fine (30sec or less)
