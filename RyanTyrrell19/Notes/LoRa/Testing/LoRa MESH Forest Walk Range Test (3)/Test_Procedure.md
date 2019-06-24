# Test 2: Forest Walk  Test
## Test Procedure
* Do the Nodes and Gateways need to use the same antenna??? (same frequency but I don't think need be the same type)

The Gateway will be set up at a static location, preferably at a location higher than where the nodes will be. Two nodes will be taken on a walk In the forest near the track. One will use the DIY Helical antenna, the other will use a more heavy duty antenna (one that a Gateway may use). The antennas to use will be determined from the previous test done (RSSI Test). The nodes will acquire a GPS fix and, every 15 seconds, transmit the coordinates to the Gateway. The nodes will save any coordinates that were sent unsuccessfully on the Arduino's EEPROM. Upon completion, the GPS coordinates received by the Gateway can be compared with that of the GPS coordinates stored to see at what locations the transmission failed.  
The Gateway will save the received data and RSSI values to an SD card. Someone watching over the Gateway can report if data has stopped being received.

## Node Setup
### General

* LoRa MESH Module Connected to Arduino Nano
* Arduino Code -> found in 'Ryan Tyrrell19/Code/MESH_and_Antenna_Test/MESH_and_Antenna_Test_Node.ino'
* LoRa MESH communicates with Arduino via AltSoftSerial
* GPS communicates with Arduino via HardwareSerial
* Debugging is done via SoftwareSerial using Putty (an adapter must be used to power the Arduino rather than the USB cord provided)
* **Node 1 uses the White LoRa Antenna, while Node 2 uses the DIY Helical Wire Antenna (both shown in the RSSi Test)**

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
* Data read/write to SD card via Arduino's dedicated SPI pins

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

# Result & Notes
## Results
* A Preliminary Test was done at ITB to ensure the test was working properly
* Data can be viewed in the Excel file "Forest Walk Data.xlsv"
* The Gateway was moved at one point during the test. This can be seen from where the black lines start/stop.

That data can be view and  examined [here][Map Site]. It has been summarized below in case the link no longer works:  

Dark Purple - Data received  
Orange - Data not received  
![alt-text][Node 1 & Node 1 (Gateway)]

Dark Blue - Data received  
![alt-text][Node 2 (Gateway)]

Data was not saved to the Node's EEPROM to view the GPS coordinates that were not successfully test. As such, another test (shown below) was done using this node

Green - Data received  
Dark Blue - Data not received  
![alt-text][Node 2 & Node 2 (Gateway)]

## Notes

* There appeared to be an issue with receiving data when Node 1 and Node 2 sent data at the same time
* If the Gateway is to be placed on a hill, ensure that the hill drops off immediately. Otherwise, the earth my block the transmission. This is why the Gateway was moved during the test.

[Map Site]: https://mapmakerapp.com?map=5d0d363ed4a9e062847053ff28ae
[Node 1 & Node 1 (Gateway)]: https://i.ibb.co/tDjdfm4/Node-1-Node-1-Gateway.png
[Node 2 (Gateway)]: https://i.ibb.co/ZGHZ3zh/Node-2-Node-2-Gateway.png
[Node 2 & Node 2 (Gateway)]: https://i.ibb.co/k6mWMtR/Node-2.png

