# Message Formats for Arduino End Node and Gateway node

This file contains some specification on the different LoRa message types that are exchanged between the Arduino end nodes and gateway nodes.

## Registration

First message sent from an end node to the gateway node, sent only once and used to register the name and sensor types of an end node with a gateway node. The format is as follows: 

| Description   | Message type and number of sensors                                                                      | Name Length             | Name                                                | Length of sensor type     | Sensor type                                      |
|---------------|---------------------------------------------------------------------------------------------------------|-------------------------|-----------------------------------------------------|---------------------------|--------------------------------------------------|
| Size in bytes | 1                                                                                                       | 1                       | Name length                                         | 1                         | Length of sensor type                            |
| Contents      | First 4 bits are 0000, as 0 is the message type, next 4 bits is the number of sensors for this end node | Length of end node name | The name of the end node, where each byte is a char | Length of the sensor type | The name of the sensor type, each byte is a char |

The last two columns are repeated for each sensor on the end node.

The gateway node sends an acknowledgement containing one byte of data, the codes are as follows:

| Ack  | Description                                                                       |
|------|-----------------------------------------------------------------------------------|
| 0x00 | Successful registration; new node registered                                      |
| 0x01 | Successful registration; the sent data is the same as saved data for this node id |
| 0x02 | Failed registration; saved data for this node id differs from what was sent       |
| 0x03 | Failed registration; unknown gateway error                                        |


## Sensor Data

The main message sent from an end node to the gateway, contains read sensor data, time information, and gps location. The format is as follows: 

| Description   | Message type and number of sensors                                                                      | Time                                                                    | Latitude                                                                | Longitude                                                                | Sensor Data                               |
|---------------|---------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------|-------------------------------------------------------------------------|--------------------------------------------------------------------------|-------------------------------------------|
| Size in bytes | 1                                                                                                       | 4                                                                       | 4                                                                       | 4                                                                        | 4                                         |
| Contents      | First 4 bits are 0001, as 1 is the message type, next 4 bits is the number of sensors for this end node | Unsigned long representing the Unix time that the data was collected at | Float representing the latitude of the end node when the data was taken | Float representing the longitude of the end node when the data was taken | Float representing the data of the sensor |

The last column is repeated for every sensor on this end node, and the last 4 columns are repeated for how ever many data points are being sent.

The gateway node sends an acknowledgement containing one byte of data, the codes are as follows:

| Ack  | Description                                                                      |
|------|----------------------------------------------------------------------------------|
| 0x00 | Success; the data was logged                                                     |
| 0x01 | Fail; the data sent doesn't correspond to the registration data for this node id |
| 0x02 | Fail; unknown gateway error                                                      |



