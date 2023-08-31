from django.shortcuts import render
from test1.models import SensorData
from django.shortcuts import render_to_response
from django.http import Http404, HttpResponse
import influxdb

# Create your views here.

def insert_data(request, type, value, source):
	try:
		data = SensorData(type = type, value=value, source =source)
		data.save()
	except:
		return HttpResponse("failed!")
	return HttpResponse("Inserted!")
	
def insert_data_influxdb(request, type, value, source):
	try:
		client = influxdb.InfluxDBClient(host='localhost', port='8086',database='sensors') #change according to your database
		data="%s,sensorID=%s value=%f "%(type,source,float(value))
		client.write(data, params={'db': client._database}, protocol='line')
	except:
		return HttpResponse("failed!")
	return HttpResponse("Inserted!")

