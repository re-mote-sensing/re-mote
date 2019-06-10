# Outline
* keep in mind the following constraints
	* weight
	* Size
	* Power consumption
	
# Hardware Breakdown
## GPS
### General
* Cheap
    * < $50
* Low Current consumption
    * < 50mA
* Quick fix
    * < 30 sec
### Specific
* Supports a V_BACK pin, that can be connected to the Arduino
	* Could use a cell battery, but Arduino is more reliable
* Capable of **reliably** backing up RTC and aided data
* Supports an EN pin
	* EN pin, when set to low, powers the GPS off. Effectively the same as using a transistor, without the transistor 
* Supports various modes of operation for altering power consumption
* Configurable
* Connector to attach external antenna
	* Allows us to choose which antenna to use

### Antenna
## LoRa


# Software Breakdown