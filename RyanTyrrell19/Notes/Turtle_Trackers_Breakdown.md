# Outline

This outline provides a condensed overview of all the research that has been done on these trackers. It includes:  

* Summary of each component, what it does, and any additional, in-depth info that is relevant to this project
* The requirements of each component. A sort of checklist to use when deciding what to purchase

* Keep in mind the following constraints
	* Weight
	* Size
	* Power consumption


# Plan to Complete this Project
1. Decide on what Hardware to use
	* Which GPS module?
	* Which LoRa module?
	* Which Microcontroller?
2. Change the code to reflect the hardware being used
	* LoRa Mesh Protocol
	* GPS features (UBLOX commands, RTC, etc.)
4. With everything set in stone, calculations can be made, and parameters altered to optimize transmission and power consumption
3. Design a waterproof casing that encompasses the Tracker while maintaining a small and streamline shape

5. Alter/fine tune the Encosure
6. Iron out any other bugs in the code
7. Run tests to verfy all aspects of the code work (especially LoRa)
8. Write up a report describing all aspects of the Trackers, including specs (lifespan, range, etc.)

	
# Hardware Breakdown
## GPS
## Description

* Summarize what was discussed in "GPS_Notes"

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
* Most modules come with a ceramic patch antenna already attached, so more often than not, you do not get to choose the GPS Antenna to use

## LoRa
### LoRa MESH Modules

### LoRa RFM95 Modules

### Antenna

### LoRa settings

## Other

* Battery


# Software Breakdown

## Code Operation Scenarios
Depending on the Hardware available, the code will be altered. The following will determine what code is used:  
* RTC always available
	* Will alter sleep interval of Arduino
* Ability to always perform a warm start (i.e. has V Backup)
	* Will alter time the GPS is running for
* EN pin available (otherwise a transistor will need to be used)
	* Will alter Hardware setup and powering of digital I/O pin
* GPS Protocol used
	* Will alter how GPS is configured, and how the data is read from it

## Code Overview
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
* NeoGPS
	* will maintan GPS accuracy by saving data in long type rather than float type


## Setup


## Loop


## RTC and Low Power

Assuming the Tracker has an RTC, and the RTC is **guaranteed** to always be running, the Tracker can run and sleep during specific time intervals, for specific periods of time.  
For example, the Tracker can be programmed to wake at 10am, run every 30min, and sleep from 4pm to 10am.  
It would even be possible to program 3 or more different running time intervals

The following variables must be defined:  
* Time A  
* Time B  
* # of fixes per day  
* Long Interval Time  
* 
The following explains how this may be implemented:

* The Arduino sleeps using the LowPower Library and uses the (inaccurate) Watchdog Timer
* The RTC time is updated by briefly powering on and reading from the GPS
* Before sleeping, compute time Arduino will sleep for, subtract 15% from it
	* The Watchdog timer is said to be 10% off. Subtract 15% to ensure the target wake up time is not overshot
* If the RTC time is +-5min of target time, start short interval sleep sequence


# GPS Fix Time

With the ability to store aiding data such as almanac, ephemeris, position, RTC, etc., the TTFF can be reduced significantly. However, knowing how long to keep the GPS running to collect this data while also not wasting power is difficult.  
The following is a possible implementation to optimize TTFF and current consumption:

* The GPS originally runs for 12.5 minutes to collect all almanac data (this is assuming this data can be stored and **guaranteed** to not be deleted. 

* The allowable fix time of the GPS can be computed 3 different ways:
	* Knowing the probability of when TTFF is likely to be acquired, average download time of ephemeris data, and computation time of Assist Autonomous feature; a rough idea of the turnaround time can be computed and used to determine the allowable run time of the GPS.
	* Alternatively, data can be read from the GPS that can indicate the status of certain components. Based on what is read, a time may be predicted for when the necessary data will be downloaded
	* Lastly, a good chunk of the time the Tracker is deployed will be spent underwater where the GPS cannot get a fix. For every time this occur, increment a counter. This counter can be used to alter the allowable fix time, at no extra cost to power consumed

* Additionally, certain things can be monitored to guage the progress of gettng a GPS fix (such as # of satellites in view). This can be used to provide more/less time to get a fix  
**Final Thoughts:**
* It takes 30sec to download ephemeris data. If the GPS module is advertised to get a TTFF from warm start in 30sec, downlaoding ephemeris data is not worth it
* As long as a warm start is possible (i.e. RTC and position data is saved), TTFF should be fine (30sec or less)

# Reading/Writing UBX Messages

* Useful data to read from GPS
	* TTFF
	* Real Time Clock Status
	* DOP (Dilution of Precision)
	* Position Fix Type
	* Power Saving Mode State
	*

* Configuring will be much more relavent for Gateways (since thye'll most likley use PSM)