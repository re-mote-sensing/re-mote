# Overview
Three proposals will be put forth regarding possible setups for the MESH Network. The following setups are:
1. LoRa MESH Modules; modules are powered continuously
2. LoRa MESH Modules; modules synchronized to run every hour for X amount of time
3. LoRa RFM95 Modules


# Proposals 1 & 2: MESH Modules

[LoRa MESH Modules]:  
* Includes a separate processor allowing them to continue running and relaying messages while the Arduino sleeps
* Processor when Idle draws ~10mA
* Data sheets available are in a different language and fairly cryptic and there are few forums out here discussing these modules
* Modules are limited on what Parameters can be changed (SF, BW, etc) either because it is not fully understood how to, or it is not supported
* The Modules come with an SMA connector (for the antenna) already attached. This can be good for the Relayer's (it enables us to attach a more heavy duty antenna) but is an inconvenience for the Turtles Trackers (since they have to be removed; not an easy task)
* The modules may be an issue to use with the Turtle Trackers in terms of weight and size 
* The Modules handle the data relaying, so little must be done software wise other than transmitting the data

## Proposals 1: MESH Modules; continuously on
### Setup
The setup consists of three components; End-Node, Relay-Node, and a Gateway. 

End-Node - To conserve power, the MESH Modules are powered off when not used. When ready to transmit data, the modules are powered on, and data is transmitted to the desired Gateway.

Relay-Node - Due to the Modules having their own processor, the Relayer's do not require an Arduino to run. After an initial configuration, these modules simply need an antenna and battery to receive and forward data. The main issue is the large power drawn when idle. (However, the Relayer's are not limited in weight and/or size, so a larger battery and/or solar panels may be employed)

Gateway - acts just like a Relay-Node, only it will determine that it's own ID is the target ID and process the message accordingly

### Power

When idle - ~10mA
When Transmitting - ~100mA (very brief)


## Proposals 2: MESH Modules; synchronized power
### Setup

Similar to the previous setup, however, all devices are equipped with an RTC and powered off for the majority of the same. Every predetermined interval, for a predetermined window of time (both measured using the RTC), all devices will power on the MESH Modules and transmit their data. A schedule must be created so that all data is not transmitted simultaneously)

End-Node - The Tracker will power on during the predetermined time, collect, and transmit its data

Relay-Node - The Relay-Nodes require an Arduino to use the RTC with the MESH Module. The additional hardware required is compensated for with a significant decrease in power drawn from the MESH Module (since it will be powered off for the majority of the time). During the allocated on-period, the MESH module will be powered and capable of relaying any data.

Gateway - acts just like a Relay-Node, only it will determine that it's own ID is the target ID and process the message accordingly


# Proposals 3: RFM95 MESH Network
## RFM95 Background Info
See LoRa_RFM95_Module.md for Background Information

## Setup 1 - RFM95 Mesh
### Setup Outline

The setup consists of three components; End-Node, Relay-Node, and a Gateway. 

End-Node - Broadcasts a message to the Gateway, waits until an Acknowledgement is received

Relay-Node - The Relay-Node's will run through the following loop; Sleep the RFM95 & Arduino for X amount of time, run the Channel Activity Detector (CAD). If CAD returns true, break the loop. Otherwise, repeat the loop.

Gateway - acts just like a Relay-Node, only it will determine that it's own ID is the target ID and process the message accordingly

Requirements:
* The message preamble **must be greater than** the sleep time during the CAD cycle (the preamble is what the CAD looks for to see if a message is available)
* The timeout values for finding a valid route & waiting for message Acknowledgement must be changed to reflect the air time of the message and the number of nodes in the network

1. End-Node requests to send a message. A valid route must be determined. This is essentially done by broadcasting to all Relay-Nodes, and having the Relay-Nodes re-broadcast until the Gateway is found. The route to the Gateway is send back to the End-Node
2. The initial broadcast to find a route causes the Relay-Nodes to break the CAD cycle and have them wait in Receiver mode.
3. Once a route has been found, the End-Node transmits the data to the Gateway.
4. The Gateway receives the data and sends back an Acknowledgement
5. All device's go back to sleep


### Power Consumption Details

Arduino power consumption when powered -> approx. 4.5mA  
Arduino power consumption when sleeping -> approx. 72.3uA  
RFM95 power consumption when sleeping - approx. 1uA  
RFM95 power consumption when Receiving - 10 - 12mA (Depends on the settings used)  
RFM95 power consumption when Transmitting - 20 - 120mA (Depends on the settings used)  


| Mode  | Power Drawn (3V3, Arduino Pro Mini & RFM95)  | Duration (if applicable) |
|:-----:|:--------------------------------------------:|:------------------------:|
| Driver + Arduino Sleep | 72.8uA | NA |
| Arduino Awake + CAD running | Depends on Parameter values (≈5-10mA) | Depends on Parameter values (≈6ms) |
| Receiver Mode | 14.7mA | NA |
|Transmission Mode | 117mA (max value displayed) | Almost Negligible |

Note: Majority of these values depend on the what the Parameters are set to

#### Current setup value and calculations

Parameter settings and associated air time values calculated using the [LoRa Calculator][LoRa Calculator]  

Parameter Settings:  
![alt-text][Parameter Settings]

Transmission Duration Breakdown:  
![alt-text][Setup Runtime]

Duration and power consumed during Transmission breakdown:  
![alt-text][Phase, duration, and power consumed]

Average Current consumption for given power consumed & Duration:  
![alt-text][Current Calculations]

**End-Node:**  
Time waiting for an Acknowledgement
* Greater than the time to send a message to the Gateway and send a message back

**Relay-Node:**  
Time between running CAD
* Depends; longer time equals less power consumed in the long run, but it requires the message preamble to be longer; causing all messages to take longer to send
* 500MS seems like a good choice
* Don't try to have Arduino sleep while CAD runs. Interrupts are handled internally by the RadioHead Library and trying to use the interrupt messes it up (plus CAD runs quick enough that having the Arduino sleep isn't really needed)


Time spent waiting for the actual data to transmit?
* Must be long enough so that a route may be found, the data transmitted, and an Acknowledgement received


**Gateway:**  
Identical to Node, only the data will be processed upon realizing it's ID matches that of the target ID for the data; processes the data accordingly

## Setup 2 Outline - RFM95 Flooding
### Setup Outline

The End-Node broadcasts its message, and all Relayer's within range rebroadcast the message. Eventually, the message will be received by the Gateway.
The Gateway then sends an ACK back. Precautions are taken to avoid broadcast storms and discard stale messages.

Message Format - [ACK]|[Lifespan]|[Message ID]|[Message Data]  
ACK - 0 or 1 ( 0 = not an ACK, 1 = is an ACK)  
Lifespan - Exceeding a threshold will result in the message being terminated  
Message ID - Uniquely determined using the Node ID, # of nodes in the network, and the # of messages the node has sent  
Message Data - Reason for the message transmission. This data could be GPS coordinates, commands, etc.  


End-Node - Constructs and broadcast the message. Enters the CAD loop and, when a message is detected, checks if it matched the assigned message ID of the message it sent, and whether it is the acknowledgement or not

Relay-Node - Stays in the CAD loop until a message is detected. When a message is received, the below condition table outlines the steps taken to determine whether to relay the data or not. Afterwards, the Relayer re-enters the CAD loop  

![alt-text][Flooding Relay Condition Table]

* If seen before, should not relay!

Gateway -  Stays in the CAD loop until a message is detected. When a message is received, it checks whether it has already seen the message. If not, it saves the message ID, resets the Lifespan count, Sets the ACK TO 1, and broadcasts the message


* Main issue is message collisions. [This][LoRa Scalability: A Simulation Model Based on Interference Measurements] paper has some interesting notes regarding message collision between LoRa Modules
## Other Potential Setup Ideas (some are repeats of above)


* Currently have the Relay-Nodes and Gateway break the CAD cycle and run in receiver mode for a set amount of time. Instead, have them break the CAD cycle, process the message, then re-enter the CAD cycle. Whether it's a broadcast to find a route, the actual data being sent, or the Acknowledgement; as long as the preamble is set long enough, the Relay-Nodes & Gateway should have enough time to exit the loop and receive the message.

* Ignore the MESH Network Protocol completely. Instead, just have the End-Node Broadcast its message to all Relay-Nodes. They will then rebroadcast the message until it makes it to the Gateway. The Gateway will then do the same thing for the Acknowledgement. This is known as **[Flooding][Flooding]**
	* Ensure each node only relays a message once. Perhaps create a circular array that stores an ID for the message. Circular so that the oldest message (that has most likely expired long a go), is removed
	* Each time a message is relayed, increment a counter (that is a part of the message) by 1. Once that message exceeds a value, do not re-transmit it
	* If a node has already received the ACK for a message it is suppose to re-transmit, do not re-transmit the message
	* Potential issue with multiple Gateways receiving the same message	

* End-Nodes broadcast the message, but Relayer’s have a specific, predetermined route (similar to what Emil mentioned in his email)
	* Issue of a node fails (cuts off connection for that "branch")

* **Basic MESH Netwrok setup, but collect data for the day and send in one go (data dump).**

* MESH network flow
	* Sends broadcast to find route, broadcast is received and forwarded until it reaches the Gateway. The final route is forwarded back. (I think) Each broadcast should have the same air time and preamble length (data sent by gateway may be longer since it sends the routing table)



[LoRa MESH Modules]: https://www.dfrobot.com/product-1670.html


[LoRa Calculator]:https://www.loratools.nl/#/airtime


[Phase, duration, and power consumed]:https://i.ibb.co/7tL8Mkz/Phase-duration-and-power-consumed.png
[Setup Runtime]:https://i.ibb.co/XSy0sHY/Setup-Runtime.png
[Current Calculations]: https://i.ibb.co/hMd9f9M/Current-Calculations.png
[Parameter Settings]: https://i.ibb.co/hHT9zHZ/Parameter-Settings.png

[Flooding]:https://en.wikipedia.org/wiki/Flooding_(computer_networking)
[Flooding Relay Condition Table]:https://i.ibb.co/QDZJ7xx/Flooding-Relay-Condition-Table.png

[LoRa Scalability: A Simulation Model Based on Interference Measurements]: https://www.mdpi.com/1424-8220/17/6/1193