## Comparison

### Software
1. Django(2.1)
2. Flask(1.02)
3. InfluxDB(0.10.0)

### Hardware
Raspberry Pi 3b+ ARMv7 Processor rev 4 (v7l) 4 cores ubuntu 16-4

### Tests

1. Django + InfluxDB
2. Flask + InfluxDB

### Insert Data
The program creates a connection for each insertion, and one HTTP request contains three parameters: type, source and value. 
This program aims to compare the performance of these two web servers for HTTP requests on Raspberry Pi.

```python
	try:
		client = influxdb.InfluxDBClient(host='localhost', port='8086',database='sensors') 
		data="%s,sensorID=%s value=%f "%(type,source,float(value))
		client.write(data, params={'db': client._database}, protocol='line')
	except:
		return HttpResponse("Failed!")
	return HttpResponse("Inserted!")
```

### Results
1. [Django](./djangoinflux.txt)  16 requests/s
2. [Flask](./flaskwithinfluxdb.txt)  20 requests/s

