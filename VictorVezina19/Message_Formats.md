# Message Formats for Arduino End Node and Gateway node

This file contains some specification on the different LoRa message types that are exchanged between the Arduino end nodes and gateway nodes. These specifications are only for the payload portion of the [LoRa mesh modules](https://www.dfrobot.com/product-1670.html) message (the only part we can control). For the specifications of the other parts of the messages, see [Spencer Park's write-up](https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/blob/master/LoRa/LoRa.ipynb). This document shows that the maximum payload size for the LoRa mesh modules is 111 bytes (127 byte message size - overhead), which gives us very limited space per LoRa message, and considering the current draw of the modules can spike up to 100 mA, keeping the number of messages to a minimum is ideal.

**NOTE:** Arduinos are little endian, so the multi-byte data types may seem "backwards"

## Registration

First message sent from an end node to the gateway node, sent only once and used to register the name and sensor types of an end node with a gateway node. The format is as follows: 

|**Description**  |Message type and number of sensors|Name Length|Name| Length of sensor type|Sensor type|
|-----------------|----------------------------------|-----------|----|----------------------|-|
|**Size in bytes**| 1 | 1 | Name length | 1 | Length of sensor type |
|**Contents**     | First 4 bits are 0000, as 0 is the message type, next 4 bits is the number of sensors for this end node | Length of end node name | The name of the end node, where each byte is a char | Length of the sensor type | The name of the sensor type, each byte is a char |

The last two columns are repeated for each sensor on the end node.

<br />

The gateway node sends an acknowledgement containing one byte of data, the codes are as follows:

| **Ack** | **Description**                                                                       |
|------|-----------------------------------------------------------------------------------|
| 0x00 | Successful registration; new node registered                                      |
| 0x01 | Successful registration; the sent data is the same as saved data for this node id |
| 0x02 | Failed registration; saved data for this node id differs from what was sent       |
| 0x03 | Failed registration; unknown gateway error                                        |

<br/>

**Example:** (in hex)  
Sending a registration with a name of "ITB_TEST", and two sensor types; "Turbidity", and "Temperature"

| 02 | 08 | 49 | 54 | 42 | 5f | 54 | 45 | 53 | 54 | 09 | 54 | 75 | 72 | 62 | 69 | 64 | 69 | 74 | 79 | 0b | 54 | 65 | 6d | 70 | 65 | 72 | 61 | 74 | 75 | 72 | 65 |
|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|

<br/>

**Rationale:**  
Sending a registration message allows the node to not have to send non changing information (like name and sensor types) multiple times, saving message space on subsequent LoRa messages. It also allows for better bookkeeping on the side of the gateway node, as it can easily keep track of which nodes are sending data to it.

<br/>

## Sensor Data

The main message sent from an end node to the gateway, contains read sensor data, time information, and gps location. The format is as follows: 

|**Description**|Message type and number of sensors|Number of Locations|Locations|Data Points|
|-|-|-|-|-|
| **Size in bytes** | 1 | 1 | 1 x Number of locations | 4 x (Number of sensors + 1) x Number of data points + 8 x Number of locations |
| **Contents** | First 4 bits are 0001, as 1 is the message type, next 4 bits is the number of sensors for this end node | Byte representing how many locations are being sent with this data | A series of bytes, each one represents the number of a data point that has an associated location | The data collected by the end node's sensors and GPS |

**NOTE:** Location data is only sent when either the latitude or longitude of the end node changes by more than 0.00005 degrees [(~5.57 m)](https://en.wikipedia.org/wiki/Decimal_degrees#Precision) due to inaccuracies possible in the gps module.

The last column is described below:

### Data points:

| **Description**   | Time | Latitude | Longitude | Sensor Data           |
|-------------------|------|----------|-----------|-----------------------|
| **Size in bytes** | 4    | 4 if it exists | 4 if it exists | 4 x Number of sensors |
| **Contents**      | An unsigned integer (32 bits) representing the unix time at which this data point was collected | A float representing the end node's latitude, nothing if this data point doesn't have a location | A float representing the end node's longitude, nothing if this data point doesn't have a location | A series of floats representing the sensor data collected for each sensor |

<br />

The gateway node sends an acknowledgement containing one byte of data, the codes are as follows:

| **Ack**  | **Description**                                                                      |
|------|----------------------------------------------------------------------------------|
| 0x00 | Success; the data was logged                                                     |
| 0x01 | Fail; this node hasn't been registered                                           |
| 0x02 | Fail; the data sent doesn't correspond to the registration data for this node id |
| 0x03 | Fail; unknown gateway error                                                      |

<br/>

**Examples:** (in hex)  
Sending one location (43.258552, -79.920548) associated with data point 1, and two data points, each with two sensor data values ((1561408970, 8.01, 17.3), (1561409031, 7.87, 17.8)):

| 12 | 01 | 01 | ca | 35 | 11 | 5d | c2 | 08 | 2d | 42 | 52 | d7 | 9f | c2 | f6 | 28 | 00 | 41 | 66 | 66 | 8a | 41 | 07 | 36 | 11 | 5D | 0a | d7 | fb | 40 | 66 | 66 | 8e | 41 |
|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|

Sending no locations and one data point with four sensor values(1561468548, 7.98, 18.2, 14, 164.8):

| 14 | 00 | 84 | 1e | 12 | 5D | 29 | 5c | ff | 40 | 9a | 99 | 91 | 41 | 00 | 00 | 60 | 41 | cd | cc | 24 | 43
|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|

<br/>

**Rationale:**  
The entire rationale behind this message format is to reduce overall message size on average. There is really only 2 bytes of overhead, plus one extra byte per location. The first byte of overhead can't be avoided; the message type has to be transmitted. The other bytes of overhead allow for a reduced message size, as it allows us to not send a location with every data point if it hasn't changed. This is more efficient than sending a location with every data point as long as less than 7/8 data points have a location. 


