# Message Formats for Arduino End Node and Gateway node

This file contains some specification on the different LoRa message types that are exchanged between the Arduino end nodes and gateway nodes.

## Registration

First message sent from an end node to the gateway node, sent only once and used to register the name and sensor types of an end node with a gateway node. The format is as follows: 

| Description   | Message type and number of sensors                                                                      | Name Length             | Name                                                | Length of sensor type     | Sensor type                                      |
|---------------|---------------------------------------------------------------------------------------------------------|-------------------------|-----------------------------------------------------|---------------------------|--------------------------------------------------|
| Size in bytes | 1                                                                                                       | 1                       | Name length                                         | 1                         | Length of sensor type                            |
| Contents      | First 4 bits are 0000, as 0 is the message type, next 4 bits is the number of sensors for this end node | Length of end node name | The name of the end node, where each byte is a char | Length of the sensor type | The name of the sensor type, each byte is a char |

The last two columns are repeated for each sensor on the end node.

<br />

The gateway node sends an acknowledgement containing one byte of data, the codes are as follows:

| Ack  | Description                                                                       |
|------|-----------------------------------------------------------------------------------|
| 0x00 | Successful registration; new node registered                                      |
| 0x01 | Successful registration; the sent data is the same as saved data for this node id |
| 0x02 | Failed registration; saved data for this node id differs from what was sent       |
| 0x03 | Failed registration; unknown gateway error                                        |

<br/>

## Sensor Data

The main message sent from an end node to the gateway, contains read sensor data, time information, and gps location. The format is as follows: 

|Description|Message type and number of sensors|Number of Locations|Locations|Data Points|
|-|-|-|-|-|
| Size in bytes | 1 | 1 | 9 x Number of locations | 4 x (Number of sensors + 1) x Number of data points |
| Contents | First 4 bits are 0001, as 1 is the message type, next 4 bits is the number of sensors for this end node | Byte representing how many locations are being sent with this data | The location data collected by the end node's gps | The data points collected by the end node's sensors |

The last two columns are described below:

### Locations:

| Description   | Associated data point | Latitude | Longitude |
|---------------|-----------------------|----------|-----------|
| Size in bytes | 1                     | 4        | 4         |
| Contents      | The data point that this location is associated with, essentially the time at which this location data was gathered | A float representing the end node's latitude | A float representing the end node's latitude |

**NOTE:** Location data is only sent when either the latitude or longitude of the end node changes by more than 0.00005 degrees [(~5.57 m)](https://en.wikipedia.org/wiki/Decimal_degrees#Precision) due to inaccuracies possible in the gps module.

### Data points:

| Description   | Time | Sensor Data           |
|---------------|------|-----------------------|
| Size in bytes | 4    | 4 x Number of sensors |
| Contents      | An unsigned integer (32 bits) representing the unix time at which this data point was collected | A series of floats representing the sensor data collected for each sensor |

<br />

The gateway node sends an acknowledgement containing one byte of data, the codes are as follows:

| Ack  | Description                                                                      |
|------|----------------------------------------------------------------------------------|
| 0x00 | Success; the data was logged                                                     |
| 0x01 | Fail; this node hasn't been registered                                           |
| 0x02 | Fail; the data sent doesn't correspond to the registration data for this node id |
| 0x03 | Fail; unknown gateway error                                                      |



