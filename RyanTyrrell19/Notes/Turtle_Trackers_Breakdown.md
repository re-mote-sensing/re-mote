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

# Library Choices

* SoftwareSerial
* AltSoftSerial
* LowPower
* NMEAGPS


## Setup

* Initilize Serial Connections
* 

## Loop

