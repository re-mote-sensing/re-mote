# Overview
Three proposals will be put forth regarding possible setups for the MESH Network. The following setups are:
1. LoRa MESH Modules; modules are powered continuously
2. LoRa MESH Modules; modules synchronized to run every hour for X amount of time
3. LoRa RFM95 Modules


# Proposals 1: MESH Modules; continuously on


# Proposals 2: MESH Modules; synchronized power


# Proposals 3: RFM95 MESH Network

## Setup Outline

The setup consists of three components; a Node, several Relays, and the Gateway. 

Node - Broadcasts a message to the Gateway, block until Acknowledgement is received

Relay - The Relay's will run through the following loop; Sleep the radio, sleep the Arduino for X amount of time, run the Channel Activity Detector (CAD). If CAD returns true, break the loop. Otherwise, repeat the loop.

Gateway - Acts just like a relay, only it will determine that it's own ID is the target ID and process the message accordingly

The following outlines how a message is sent from the node to the Gateway:
1. The node sends X amounts of dummy messages, and than delays for Y amount of time
2. Any relays that receive the broadcast will break the CAD loop, broadcast their own dummy messages, than wait for an actual message for Z amount of time.
	* The node must provide enough of a delay to allow all Relay's to wake up, and the relay's must block for long enough that the actual message can get though, but not so long that power is wasted.
3. The Node will send it's message, and hopefully receive an Acknowledgement from the Gateway


Essentially, the node sends a dummy load to "wake up the network". Once awake, the Node can safely send the actual data. This method does work, it is simply a matter of how much power it draws overall.

## Breakdown

# Node

Send dummy loads based on time or quantity?
	* Quantity is best. This will ensure no delay between loads and optimizes the chance of not being missed by the Relay's

Delay between sending dummy loads and sending actual data?
	* Depends on several factors; time it takes for dummy load to send and relay to receive, and the size of the network

Power consumed - of RFM95 (3.3V)
	* During Dummy load transmission - 
	* Delay between dummy loads and actual data - 
	* Transmission of data and receiving acknowledgement - 

Power consumed - of RFM95 + Arduino Pro Mini (3.3V)
	* During Dummy load transmission - 
	* Delay between dummy loads and actual data - 
	* Transmission of data and receiving acknowledgement - 

## Relay

Time between running CAD
	* Depends; longer time equals less power consumed in the long run, but it requires the Node's to send more dummy loads to ensure one is received when CAD is run
	* 500MS seems like a good choice

Dummy Load send time?
	* Quantity based; identical to the Node


Time spent waiting for the actual data to transmit?
	* Depends

Power consumed - of RFM95 (3.3V)
	* Cycle between sleeping and running CAD - 
	* During Dummy load transmission - 
	* Blocking while waiting for data transmission - 

Power consumed - of RFM95 + Arduino Pro Mini (3.3V)
	* Cycle between sleeping and running CAD - 
	* During Dummy load transmission - 
	* Blocking while waiting for data transmission - 


## Gateway
* Identical to Node, only the data will be processed upon realizing it's ID matches that of the target ID for the data; processes the data accordingly

# Power consumed during Various Modes

## RFM95

Sleep - approx. 1uA  
Receiving - 10 - 12mA (Depends on the settings used)  
Transmitting - 20 - 120mA (Depends on the settings used)  

CAD:  
* Taken from the Datasheet of the RFM95,

LoRa Symbol Rate = BW / 2^SF, where BW = Bandwidth, SF = Spreading Factor

The time taken for the channel activity detection is dependent upon the LoRa modulation settings used. For a given configuration the typical CAD detection time is shown in the graph below, expressed as a multiple of the LoRa symbol period. Of this period the radio is in receiver mode for (2^SF + 32) / BW seconds. For the remainder of the CAD cycle the radio is in a reduced consumption state.

![alt-text][CAD as a Function of SF]

To illustrate this process and the respective consumption in each mode, the CAD process follows the sequence of events outlined below:

![alt-text][Consumption Profile of the LoRa CAD profile]

The receiver is then in full receiver mode for just over half of the activity detection, followed by a reduced consumption processing phase where the consumption varies with the LoRa bandwidth as shown in the table below.

![alt-text][LoRa CAD Consumption Figure]


Example:

Let SF = 7 (128 chips/symbol), BW = 125kHz  
Symbol rate = BW / 2^SF = 125000 / 2^7 = 976.56 symbols/second

With SF = 7, CAD runs for a total time of approx. 1.92 Symbols (according to the chart)

Total CAD time = (1/976.56) * 1.92 = 1.966 milliseconds

Of that time, Receiver mode is running for (2^SF + 32) / BW = (2^7 + 32) / 125000 = 1.28 milliseconds


Remainder of the CAD process = 1.966 - 1.28 = 0.686 milliseconds

At 125 kHz, power consumed during receiver mode is 10.8mA and power for the remainder of CAD is 5.6 mA.

Therefore, total current consumed is (10.8mA * 1.28e-3) + (5.6mA * 0.686e-3)







[CAD as a Function of SF]:https://i.ibb.co/J5spnY6/CAD-as-a-Function-of-SF.png
[Consumption Profile of the LoRa CAD profile]: https://i.ibb.co/3fd2R2v/Consumption-Profile-of-the-Lo-Ra-CAD-profile.png 
[LoRa CAD Consumption Figure]: https://i.ibb.co/FsSK3RP/Lo-Ra-CAD-Consumption-Figure.png