# Test 2: Forest Walk  Test
## Test Procedure
* Do the Nodes and Gateways need to use the same antenna??? (same frequnecy but I don't think need be the same type)

The Gateway will be set up at a static location, preferably at a location higher than where the nodes will be. Two nodes will be taken on a walk around cootes. One will use the DIY Helical antenna, the other will use a more heavy duty antenna (one that a Gateway may use). The antennas to use will be determined from the previous test done (RSSI Test). The nodes will aquire a GPS fix and, every 15 seconds, transmit the coordinates to the Gateway. The nodes will save any coordinates that were sent unsuccessfully on the Arduino's EEPROM. Upon completion, the GPS coordiante recieved by the Gateway can be compared with that of the GPS coordinates stored to see at what locations the transmission failed.  
The Gateway will save the received data and RSSI values to an SD card. Someone watching over the Gateway can report if data has stopped being received.

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
* Data read/write to SD card via Arduinos dedicated SPI pins

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