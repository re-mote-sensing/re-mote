# Go Influxdb API Documentation

Wherever {Homepage} is written it means the main URL that your server is running on (ex: [http://www.cas.mcmaster.ca/](http://www.cas.mcmaster.ca/#)).

## Getting node information

### Getting current information of nodes

**Type:** GET  
**URL:** {Homepage}/sensor/info  
**Description:** Gets the current information of all sensors, including id, name, last known coordinates, and time of those coordinates  
**Example:** {Homepage}/sensor/info  
Could return:

		{  
			"locations": [  
				[  
					"1",  
					"itb",  
					"43.259009, -79.920048",  
					"1557168840530000000"  
				],  
				[  
					"2",  
					"McMaster_Innovation_Park",  
					"43.256857, -79.900731",  
					"1557238174630000000"  
				],  
				[  
					"3",  
					"GPS",  
					"43.261032, -79.907221",  
					"1557238177230000000"  
				]  
			]  
		}  

### Getting detailed information of a specific node

**Type:** GET  
**URL:** {Homepage}/sensor/info?id={id}  
**Description:** Gets the entire location information of the node with id {id}  
**Example:** {Homepage}/sensor/info?id=3  
Could return:

		{  
			"name": "GPS",  
			"locations": [  
				[  
					"1557238176630000000",  
					"43.260948, -79.907204"  
				],  
				[  
					"1557238177230000000",  
					"43.261032, -79.907221"  
				]  
			]  
		}  

### Getting current information of all nodes with a certain sensor type

**Type:** GET  
**URL:** {Homepage}/sensor/info?type={sensor type}  
**Description:** Gets the current information of all sensors that have data for {sensor type}, including id, name, last known coordinates, and time of those coordinates  
**Example:** {Homepage}/sensor/info?type=Temperature  
Could return:

		{  
			"locations": [  
				[  
					"1",  
					"itb",  
					"43.259009, -79.920048",  
					"1557168840530000000"  
				],  
				[  
					"2",  
					"McMaster_Innovation_Park",  
					"43.256857, -79.900731",  
					"1557238174630000000"  
				]  
			]  
		}  

<br/>

## Getting sensor type information

### Getting sensor types

**Type:** GET  
**URL:** {Homepage}/sensor/type  
**Description:** Gets the full list of sensor types available  
**Example:** {Homepage}/sensor/type  
Could return:

		{  
			"types": [  
				"Temperature",  
				"Turbidity",  
				"Conductivity"
			]  
		}  

### Getting the sensor types available to a specific node

**Type:** GET  
**URL:** {Homepage}/sensor/type?id={id}  
**Description:** Gets the full list of sensor types available for the node with id {id}  
**Example:** {Homepage}/sensor/type?id=1  
Could return:

		{  
			"types": [  
				"Temperature",  
				"Turbidity"  
			]  
		}  

<br/>

## Posting sensor information

**Type:** POST  
**URL:** {Homepage}/sensor/info?id={id}&name={name}&coords={coordinates as a string}&time={unix time in ns}  
**Description:** Adds information data for a node to the database with the above values  
**Example:** {Homepage}/sensor/info?id=1&name=itb&coords=43.259015,-79.920087&time=1562178587000000000  
This request doesn't return any data  

<br/>

## Getting sensor data

### Getting data for a specific node and sensor type

**Type:** GET  
**URL:** {Homepage}/sensor/data?id={id}&type={sensor type}  
**Description:** Gets the data of type {sensor type} from the node with id {id}  
**Example:** {Homepage}/sensor/data?id=1&type=Temperature  
Could return:

		{  
			"id": "1",  
			"type": "Temperature",  
			"data": [  
				[  
					"1557169056986000000",  
					"20"  
				],  
				[  
					"1557169061851000000",  
					"21"  
				],  
				[  
					"1557169065664000000",  
					"18"  
				]  
			]  
		}  

### Getting data for a specific node and sensor type and time interval

**Type:** GET  
**URL:** {Homepage}/sensor/data?id={id}&type={sensor type}&start={unix time in ns}&end={unix time in ns}  
**Description:** Gets the data of type {sensor type} from the node with id {id} where the time is between the start and end times  
**Example:** {Homepage}/sensor/data?id=1&type=Temperature&start=1557169056986000000&end=1557169065543000000  
Could return:

		{  
			"id": "1",  
			"type": "Temperature",  
			"data": [  
				[  
					"1557169061851000000",  
					"21"  
				]  
			]  
		}  

<br/>

## Posting sensor data

### Posting sensor data

**Type:** POST  
**URL:** {Homepage}/sensor/data?id={id}&type={sensor type}&value={value}&time={unix time in s}  
**Description:** Adds sensor data to the database with the above values  
**Example:** {Homepage}/sensor/data?id=1&type=Temperature&value=17.5&time=1562178587  
This request doesn't return any data  


### Posting data and information from multiple nodes

**Type:** POST  
**URL:** {Homepage}/sensor/data?data={data}  
**Description:** Adds sensor and information data from multiple nodes to the database all at once. This is the request that the Arduino gateway nodes use to interact with the web server. Specifications and examples on how to format {data} can be found in the [Arduino message format specification file](Documentation/Water_Sensor/Message_Formats.md#data-post-to-server).  



