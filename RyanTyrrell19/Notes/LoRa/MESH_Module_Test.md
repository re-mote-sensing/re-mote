# Test Outline

1) Four tests will be conducted, each in a different environmental setting. 
2) For each setting, tests will be conducted over several distances. 
3) For each distance, elevation and the antenna used will be changed

* Distances will be measured out using a GPS watch (Forerunner 235)

# Changeable Parameters (Of the Node)

* Distance
    * Increase distance over set intervals
    * When no longer recieving, slowly move back until it does again
* Environment
    * Line-of-sight (no obstructions)
    * Rural (Tress & hills)
    * Urban (City)

* Antenna
    * Powerful, large antenna
    * no antenna
    * Any other antenna I can find?
* Elevation
    * On the ground
    * Waist height
    * Head height

* LoRa Settings?

* Gateway elevation?

# Record

* If data was recieved
* Signal strength

# Test Set-Up

## Node
### General

* LoRa MESH Module Connected to Arduino Nano
* Arduino Code -> found in 'Ryan Tyrrell19/Code/LoRa/LoRa_MESH_Test_Node.ino'
* LoRa MESH communicates with Arduino via SoftwareSerial

### Pinout

LoRa       Arduino
GND    ->   GND
VCC    ->   5V
RX     ->   D2
TX     ->   D3


## Gateway
### General

* LoRa MESH Module Connected to Arduino Uno
* Arduino Code -> found in 'Ryan Tyrrell19/Code/LoRa/LoRa_MESH_Test_Gateway.ino'
* LoRa MESH communicates with Arduino via SoftwareSerial

### Pinout

LoRa       Arduino
GND    ->   GND
VCC    ->   5V
RX     ->   D2
TX     ->   D3

# Test 1: Line-of-Sight

General Location:
Gatway Location:
Node Locations: 


