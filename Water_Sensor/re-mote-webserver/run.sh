echo "re:mote Server\n"

chmod -R 777 /var/lib/influxdb/
service influxdb start

echo "Waiting for influxdb ..."
sleep 5
echo "Create Database: MacWater"
curl -XPOST http://localhost:8086/query --data-urlencode "q=CREATE DATABASE MacWater"

echo "\n\n*********\n > Run the following commands to add sample data to the website"
echo "curl --location --request POST 'http://localhost:8080/sensor/info?k=remote&id=10000&name=Example&coords=43.2585,-79.9201'"
echo "curl --location --request POST 'http://localhost:8080/sensor/data?k=remote&id=10000&type=Water_Temperature&value=25&time=1670270196'"
echo "*********\n"

cd backend
./main