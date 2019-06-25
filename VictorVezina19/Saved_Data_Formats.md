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

A data point will only include a location if the location of the end node has changed by more than 0.00005 degrees [(~5.57 m)](https://en.wikipedia.org/wiki/Decimal_degrees#Precision) in either latitude or longitude (due to inaccuracies in GPS modules). The data will loop around the EEPROM if it's necessary, overwriting the oldest data with new data.

<br/>

**Storage size:**  
The number of data points the EEPROM will be able to hold depends on the Arduino being used, the number of sensors, and how frequently the location is changing, but can be calculated using the following formula, where the things in the square brackets must be replaced:  
(\[EEPROM size\] - 4) / (1 + (\[Number of sensors\] + 1) * 4 + \[Fraction of data points that contain a location\] * 8)  
As an example, if we used an Arduino nano (1KB EEPROM) with 4 sensors, reading the sensors every hour, and assume the location is going to change once every 48 sensor reads (2 days), we get:  
(1024 - 4) / (1 + (4 + 1) * 4 + 1/48 * 8) = 48.19 data points, or 2 days of internal data storage

<br/>

**Rationale:**  
The reason behind most of these design decisions is storage space availability; Arduinos generally have very little EEPROM (aswell as SRAM and Flash) memory, thus storage space conservation is very important. With the EEPROM, we encounter another problem; that is that the EEPROM can only be written to quite a low amount of times (100000) before it could start to fail. Taking these two factors into account, I designed the overhead space usage to be quite low (only 4 bytes), and tried to minimize the space required by each data point on average. Using a "has location" byte reduces the amount of memory each data point uses on average (as long less than 7/8 of data points have a location), while still being able to easily add or remove data points. The use of two addresses for "bookkeeping" allows easily writing/reading the important data and looping around the EEPROM, without excessive EEPROM writes.

<br/>

## Arduino Gateway Node

The format that the Arduino gateway node uses to save the time, location, and sensor data from multiple nodes in its SD card. Please supply the gateway node with a completely empty SD card (or microSD), formatted to either Fat16 or Fat32.  
Each node registered with the gateway node has its own .csv file on the SD card named nodeX.csv (where X is the node's id). The format for that .csv is as follows, where things in brackets are replaced by actual data: 

(Name of node)  
Time, Latitude, Longitude, (Sensor 1 Name), (Sensor 2 Name) ...  
(Data 1 Time), (Data 1 Latitude), (Data 1 Longitude), (Data 1 Sensor 1), (Data 1 Sensor 2) ...  
(Data 2)  
...

**Example:**  
ITB\_TEST  
Time, Latitude, Longitude, Temperature, Dissolved\_Oxygen, Turbidity, Conductivity  
1560443510000000000,43.259397,-79.920138,17.4,7.89,13,52.25  
1560443556000000000,,,16.8,8.07,4,0  
1560690552000000000,,,17.3,8.11,7,104.3  

The node can have any number of sensors (although the LoRa message formats limit this to 15), it can have any number of data points, and all of the names can be of any size (but cannot contain commas or new lines). Time is the string representation of an unsigned integer (32 bit) of the unix time that the data point was collected at, Latitude and Longitude are the string representations of floats of the latitude and longitude of the node, and the sensor data is the string representation of a float of the data collected for that sensor. Please note: not every data point in the file will have a latitude and longitude, they are only inserted if the node's location changes.

<br/>

There is then an additional .csv on the SD card named ToSend.csv, which contains data on what node data still needs to be posted to the server. Its format is as follows, again where things in brackets are replaced by actual data:

Node ID, Data Points, Position  
(Node 1 ID), (Number of data points node 1 has to send), (Position in the node 1 file where the data to be sent starts)  
(Node 2)  
...

**Example:**  
Node ID, Data Points, Position  
2,001,48  
4,003,104  
3,001,763

Both the ID and position in the file are printed as whatever length, but the number of data points is always printed as a 3 digit number (using leading zeros if needed). Please note: Only nodes that have data to send to the server will have data in this file, and a node cannot have more than 255 data points waiting to be sent to the server.

<br/>

**Storage size:**  
The amount of data the SD card will be able to hold depends on a lot of different factors, especially how often the location is updated and which sensors are being used, as this can influence how many characters are required for each data point (7.1 vs 982.3465; both are floats but take up very different amount of memory when printed as a string)  
As a quick example, let's assume 5 end nodes with 4 sensors each sending data every hour, and their locations changing every 48 hours. Let's also assume names (node and sensor) of 15 characters, each sensor reading takes 5 characters (ex: 54.89), time takes 10 characters (ex: 1561405722), and each of latitude and longitude take 10 characters (ex: -79.920551). For a 2GB microSD card (ignoring flash overhead space loss), we get:  
(2*10^9) - 5 * (16 + 34 + 15 * 4) = 1999999450 bytes of memory available for data storage (ignoring ToSend.csv, as it takes very minimal space)  
We then get that each data point takes:  
13 + 10 + 4 * 5 + (1/48) * 20 = ~43.42 bytes  
This gives us:  
1999999450 / 43.42 = ~46061710 data points, or ~9212342 data points per node, or ~383847.6 days of data storage.  

<br/>

**Rationale:**  
As can be seen, saving space is much less important when dealing with the SD card on the gateway node. That is why I was able to take some liberties to make data reading, writing, and analizing easier. This includes using the .csv format, which can take extra overhead space with the commas, but allows for easy importing into excel, which could be very useful for daa analysis. It also allowed using strings for saving the data, making it much easier for humans to read it. It also allowed saving a bit of extra data in ToSend.csv (number of data points), which isn't strictly necessary, but allows the gateway Arduino to quickly get the size required for the data post to the server before constructing the post, which stops [heap fragmentation](https://cpp4arduino.com/2018/11/06/what-is-heap-fragmentation.html).

<br/>

## Raspberry Pi Sever

The format that the Raspberry Pi server uses to save the time, location, and sensor data from multiple nodes in its InfluxDB database (default name is MacWater). The database has two measurements (tables) named sensorInfo and sensorMeasurements. sensorInfo looks like: 

| **time** | **id** | **name** | **value** |
|----------|--------|----------|-----------|

In this case, value is a string representing the latitude and longitude of the node (ex: "43.258506,-79.920547"). This table is used to keep tack of the nodes' names and locations, their "info". Every entry in the table represents a location change, and must contain a time, id, and value. The name column only needs to be filled once per node id. Every column in this table is represented as a string

<br/>

sensorMeasurements looks like: 

| **time** | **id** | **sensortype** | **value** |
|----------|--------|----------------|-----------|

This table is used to log the actual sensor data from the nodes. Each entry in this table represents a sensor reading, and must contains a time, id, sensortype, and value. The id and sensortype columns are represented as strings, whereas value is a float.

<br/>

**Rationale:**  
By using two tables, we are able to separate node information data that will probably be updated infrequently and the sensor data that will be updated very frequently. The data from the two tables can still be linked together quite easily using the node's id.



