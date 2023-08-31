# InfluxDB format specification

Whenever I write table, I mean an influx measurement.

The database we are using is called MacWater, and contains two tables to save the sensor and node data:

## sensorInfo table

This table has 4 columns: time, id, name, and value, where value represents the coordinates of that node at that time. id, name, and value are all stored as strings.

#### Example: 

|**time**|**id**|**name**|**value**|
|----|--|----|-----|
1455396460000000000| 2 |    West Pond |      43.2697, -79.9318|
1455396485000000000| 2 |    West Pond |      43.2684, -79.9361|
1561742252000000000| 16|    Other     |      43.000000,-79.000000|
1561742334000000000| 16|    Other     |      43.000000,-79.000000|
1563816848000000000| 8192|  Weekend_Gateway| 43.000000,-79.000000|
1564688848000000000| 4112|  Gateway_2|       43.258415,-79.920830|


## sensorMeasurements table

This table has 4 columns: time, id, sensortype, and value, where value is the measurement of that sensortype from that node at that time. id and sensortype are saved as strings, and value is saved as a float.

#### Example:

|**time**|**id**|**sensortype**|**value**|
|----|--|----|-----|
1455592250590000000| 2|     Dissolved_Oxygen|  7.4|
1455652663551000000| 2|     Dissolved_Oxygen|  7.24|
1561742252000000000| 16|    Conductivity    |  487.1|
1561742252000000000| 16|    Dissolved_Oxygen|  7.75|
1561742252000000000| 16|    Turbidity       |  46|
1561742252000000000| 16|    Water_Temperature| 23|
1563816787000000000| 4112|  Air_Temperature|   24.3|
1563816787000000000| 4112|  Humidity       |   57.3|

