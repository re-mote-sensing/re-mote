# Test Outline

1. Construct a Helical Antenna using a guide below
    * Will need to remove SMA connector of MESH modules to connect the antenna
2. Configure MESH Module to match antenna attached to it
	* Big one here is frequency
2. Test to determine which antenna to use for Trackers.
    * Gateway antenna does not matter too much. Use one with good receiver sensitivity
    * Do RSSI test to see how Tracker antenna performs
        * Set Modules and antenna up at set distance. Send data, compare RSSi values. Do this for different orientations and distances 
            * Must be specific with set-up. Distance, elevation, weather conditions, etc.
        * NOTE: to try DIY Helic Antenna, SMA connector of MESH module must be removed 
2. Determine which antenna will be used
    * NOTE: Gateways are not limited to the weight or space constraint the Trackers are, so a different antenna may be used. The antenna used by the Gateway cannot improve the distance the turtles can travel, but it can improve the separation distance between other Gateways. Only requirement is good receiver sensitivity
3. Take Tracker for a walk
	* gateway is static at a known location
	* Trackers will continuously transmit GPS data, while also saving the GPS data to file
	* The data successfully transmitted can be compared against all the points taken by the Tracker

# Testing Info
## Testing Environment
![alt-text][Testing Environment] [[6]]

* For tests that yield usable data, you should be testing your network at several locations. In addition, the the following metadata should be recorded, [[6]]
	* You should be testing your network at several locations
	* Location (lat, lon)
	* Height (above/below ground)
	* Surrounding (building inside/outside, forest, street etc.)
	* Weather conditions


## Test Values
**RSSI**: indication of the radio power level being received by the device [[6]]

*  In general you can say, the higher the RSSI number, the stronger the signal
	* But the number does not linearly correspond to the distance between transmitter and receiver. 
	* RSSI won’t tell you much about the bigger picture, especially if you only measure some spots and for a short period of time. 
	* In addition the signal strength (in dBm) is subject to a logarithmic loss dependent on the distance.
* The lower you get (less than -100), the less linear correlation you will have between the distance (in free space) and the RSSI value. The lower the value, the less valuable information you get out of the RSSI value alone.
 
## Test Procedures Available

[DIY Test using RSSI values][DIY RSSI Test]  
* A simple test to compare the performance of different antennas using the RSSI value

[DIY LoRa Range Test][DIY LoRa Range Test]
* Travel the area with a LoRa device that transmits GPS coordiantes. In addition, track where you've travelled using GPS. Compare what GPS data was recieved with the GPS data that tracked where you've travelled to see what was recieved and what was not

# Test 1: RSSI Test
## Test Outline

Two nodes will be placed 50m apart, at waist height. One node (host) will transmit dummy data to the other node (client). The client node will then send back the recorded RSSi value associated with that dummy load. The host node will receive this data (the first RSSI value), along with the associated RSSI value. This will de done 10 times, the two RSSI values will be averaged and displayed. This will be done with various antennas to get a general idea of their performance.

An additional test will be done where the node is enclosed in a casing similar to what will be used to protect/waterproof the tracker. THis will enable us to see how much the enclosure limits the range of Trackers LoRa.

The Client will be the Node with the same Antenna for all Tests

## Host Setup
### General

* LoRa MESH Module Connected to Arduino Nano
* Arduino Code -> found in 'Ryan Tyrrell19/Code/Antenna_RSSI_Test/LoRa_RSSI_Test_Host.ino'
* LoRa MESH communicates with Arduino via SoftwareSerial

### Pinout

LoRa   |   Arduino  
GND    ->   GND  
VCC    ->   5V  
RX     ->   D2  
TX     ->   D3  


## CLient Setup
### General
* LoRa MESH Module Connected to Arduino Uno
* Arduino Code -> found in 'Ryan Tyrrell19/Code/Antenna_RSSI_Test/LoRa_RSSI_Test_Client.ino'
* LoRa MESH communicates with Arduino via SoftwareSerial

### Pinout
LoRa   |   Arduino  
GND    ->   GND  
VCC    ->   5V  
RX     ->   D2  
TX     ->   D3  


## Test Procedure & Results

The following antenna will be used for the Client:

![alt-text][Gateway Antenna]

The following antennas will be tested with the Host:

![alt-text][Test Antennas]

The antennas will be tested from left to right, with the last one representing no antenna.

The following location is where the test will be conducted:

![alt-text][RSSI Test Area]


# Test 2: Cootes Walk-around Test
## Test Procedure
* Do the Nodes and Gateways need to use the same antenna??? (same frequnecy but I don't think need be the same type)

The Gateway will be set up at a static location, preferably at a location higher than where the nodes will be. Two nodes will be taken on a walk around cootes. One will use the DIY Helical antenna, the other will use a more heavy duty antenna (one that a Gateway may use). The antennas to use will be determined from the previous test done (RSSI Test). The nodes will aquire a GPS fix and, every 15 seconds, transmit the coordinates to the Gateway. The nodes will save any coordinates that were sent unsuccessfully on the Arduino's EEPROM. Upon completion, the GPS coordiante recieved by the Gateway can be compared with that of the GPS coordinates stored to see at what locations the transmission failed.  
The Gateway will save the received data and RSSI values to the EEPROM. Someone watching over the Gateway can report if data has stopped being received.

## Node Setup
### General

* LoRa MESH Module Connected to Arduino Nano
* Arduino Code -> found in 'Ryan Tyrrell19/Code/MESH_and_Antenna_Test/MESH_and_Antenna_Test_Node.ino'
* LoRa MESH communicates with Arduino via AltSoftSerial
* GPS communicates with Arduino via HardwareSerial
* Debugging is done via SoftwareSerial using Putty (an adapter must be used to power the Arduino rather than the USB cord provided)

### Pinout

LoRa   |   Arduino  
GND    ->   GND  
VCC    ->   5V  
RX     ->   D8  
TX     ->   D8  

GPS   |   Arduino  
GND    ->   GND  
VCC    ->   5V  
RX     ->   TX  
TX     ->   RX

PC    |   Arduino  
GND    ->   GND  
VCC    ->   5V  
RX     ->   D3  
TX     ->   D2


## Gateway Setup
### General

* LoRa MESH Module Connected to Arduino Uno
* Arduino Code -> found in 'Ryan Tyrrell19/Code/MESH_and_Antenna_Test/MESH_and_Antenna_Test_Gateway.ino'
* LoRa MESH communicates with Arduino via AltSoftSerial
* GPS communicates with Arduino via HardwareSerial
* Debugging is done via SoftwareSerial using Putty (an adapter must be used to power the Arduino rather than the USB cord provided)

### Pinout

LoRa   |   Arduino  
GND    ->   GND  
VCC    ->   5V  
RX     ->   D8  
TX     ->   D8  

GPS   |   Arduino  
GND    ->   GND  
VCC    ->   5V  
RX     ->   TX  
TX     ->   RX

PC    |   Arduino  
GND    ->   GND  
VCC    ->   5V  
RX     ->   D3  
TX     ->   D2


[6]: https://lorawantester.com/2019/01/09/5-simple-rules-to-guarantee-a-successful-lorawan-coverage-test/

[DIY RSSI Test]: http://www.ph2lb.nl/blog/index.php?page=lora-measuring-antennas
[DIY LoRa Range Test]: https://reibot.org/2017/04/23/lora-range-test/

[Antenna Test 1]: https://www.coredump.ch/2017/04/30/lorawan-868mhz-antenna-test-part-2/
[Antenna Test 2]: https://medium.com/home-wireless/testing-lora-antennas-at-915mhz-6d6b41ac8f1d
[Testing Environment]: https://i.ibb.co/VVbH171/6.png

[Gateway Antenna]: https://i.ibb.co/x2M60vc/Gateway-Antennas.jpg
[Test Antennas]: https://i.ibb.co/VBMqhqq/Test-Antennas.jpg

[RSSI Test Area]: https://i.ibb.co/bQqkWgL/7.png
