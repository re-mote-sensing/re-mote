# Saved Data Formats for Arduino End Node and Gateway node, and Raspberry Pi Server

This file contains some specification on the different formats in which data is saved on the Arduino end nodes and gateway nodes, and on the Raspberry Pi server.

## Arduino End Node

The format that the Arduino end node uses to save the time, location, and sensor data in its EEPROM. If the EEPROM on the Arduino that you are using has been used before, please make sure to set the first 4 bytes according to the default values of Last Saved Data Address and Valid Data From Address. The format of the data is as follows: 

| **Description**   | Last Saved Data Address | Valid Data From Address | Data Points |
|-------------------|-------------------------|-------------------------|-------------|
| **Size in Bytes** | 2                       | 2                       | (1 + 4 * (Number of sensors + 1)) * Number of data points + 8 * Number of locations |
| **Contents**      | The EEPROM address directly after the end of the last saved data point. Default value is 4 | The EEPORM address where valid data starts. Default value is 3 | The actual saved data |

The last column is described below:

| **Description**   | Has Location | Time | Location       | Sensor Data           |
|-------------------|--------------|------|----------------|-----------------------|
| **Size in bytes** | 1            | 4    | 8 if it exists | 4 * Number of sensors |
| **Contents**      | Byte representing whether or not this data point contains location data | An unsigned integer (32 bit) representing the Unix time that this data was collected at | Two 4 byte floats, the first represents latitude, the second longitude. If this data point doesn't contain location data, this is 0 bytes long | A float representing the data collected from a sensor. This column is repeated for every sensor on the end node |

A data point will only include a location if the location of the end node has changed by more than 0.00005 degrees [(~5.57 m)](https://en.wikipedia.org/wiki/Decimal_degrees#Precision) in either latitude or longitude (due to inaccuracies in GPS modules). The data will loop around the EEPROM if it's necessary, overwriting the oldest data with new data. How many data points the EEPROM will be able to hold depends on the Arduino being used, the number of sensors, and how frequently the location is changing, but can be calculated using the following formula, where the things in the square brackets must be replaced: 
(\[EEPROM size\] - 4) / (1 + (\[Number of sensors\] + 1) * 4 + \[Fraction of data points that contain a location\] * 8)

<br/>

## Arduino Gateway Node

The format that the Arduino gateway node uses to save the time, location, and sensor data from multiple nodes in its SD card. Please supply the gateway node with a completely empty SD card (or microSD), formatted to either Fat16 or Fat32. Each node registered with the gateway node has its own .csv file on the SD card named nodeX.csv (where X is the node's id). The format for that .csv is as follows, where things in brackets are replaced by actual data: 

(Name of node)
Time, Latitude, Longitude, (Sensor 1 Name), (Sensor 2 Name) ...
(Data 1 Time), (Data 1 Latitude), (Data 1 Longitude), (Data 1 Sensor 1), (Data 1 Sensor 2) ...
(Data 2)
...

The node can have any number of sensors (although the LoRa message formats limit this to 15), it can have any number of data points, and all of the names can be of any size (but cannot contain commas or new lines). Time is the string representation of an unsigned integer (32 bit) of the unix time that the data point was collected at, Latitude and Longitude are the string representations of floats of the latitude and longitude of the node, and the sensor data is the string representation of a float of the data collected for that sensor. Please note: not every data point in the file will have a latitude and longitude, they are only inserted if the node's location changes

There is then an additional .csv on the SD card named ToSend.csv, which contains data on what node data still needs to be posted to the server. Its format is as follows, again where things in brackets are replaced by actual data:

Node ID, Data Points, Position
(Node 1 ID), (Number of data points node 1 has to send), (Position in the node 1 file where the data to be sent starts)
(Node 2)
...

Both the ID and position in the file are printed as whatever length, but the number of data points is always printed as a 3 digit number (using leading zeros if needed). Please note: Only nodes that have data to send to the server will have data in this file, and the maximum amount of data points that need to be sent for any individual node is 255.

<br/>

## Raspberry Pi Sever

The format that the Raspberry Pi server uses to save the time, location, and sensor data from multiple nodes in its InfluxDB database (default name is MacWater). The database has one measurement (table) named sensorData, which contains the following columns:

| **time** | **id** | **name** | **latitude** | **longitude** | **sensortype** | **value** |
|----------|--------|----------|--------------|---------------|----------------|-----------|

Each entry in the database represents a sensor reading, and must contains at least a time, id, sensortype, and value. The other columns (name, latitude, and longitude) can be blank, but must be present for each individual node id at least once. The id, name, latitude, longitude, and sensortype columns are all represented as strings, whereas the value column is represented as a float.



