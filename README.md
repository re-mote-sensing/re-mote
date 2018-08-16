# arduino-motes

In order to get this project running:

* Install the Arduino IDE if you do not have it.

* Clone the project into the "Arduino" folder that is set up when the Arduino IDE is installed. If there is an empty "libraries" folder, replace it with the libraries present in this repository. If there are libraries you have pre-installed, make sure to copy this project's libraries into the pre-existing folder.

* Upload LoRa3GServer to the arduino with a LoRa shield and 3G shield stacked onto it. This acts as the LoRa server which sends data from all sensors to the Raspberry Pi server.

* If using Atlas Scientific EC and DO sensors, you can upload the ECClient and DOClient sensorval sketches to the Arduinos with those respective sensors attached and a LoRa shield installed. 

* If using the setup with new sensors, you can follow the sketches used to send values from Atlas Scientific sensors as a guideline to create a program for the new sensors.


* If simply testing the setup with no sensors, install the DOClient and ECClient Randnum sketches to the respective Arduinoswith LoRa shields takced on top to send randum floats to your web server. 

All code is commented with details on how to use it, so it is completely adjustable to specific needs.
