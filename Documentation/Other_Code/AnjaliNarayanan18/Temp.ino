/*Original author © Miles Burton
Last update from source: October 25, 2018 
Original code can be found under "Examples" in the DallasTemperature Library
Code truncated by Anjali Narayanan
*/

//Include libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_PIN 2 // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_PIN);// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void setup(void)
{
  Serial.begin(9600); //Begin serial communication
  Serial.println("Temperature Reading from DS18B20 Sensor"); //Print a message
  sensors.begin();
}

void loop(void)
{ 
  // Send the command to get temperatures
  sensors.requestTemperatures();  
  Serial.print("Temperature is: ");
  Serial.println(sensors.getTempCByIndex(0)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
 Serial.print("C");
  //Update value every 1 sec.
  delay(1000);
}
