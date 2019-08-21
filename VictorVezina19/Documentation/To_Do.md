# List of things to do on this project

This is a list of things that need to be done in this project, in no particular order

* Finish README.md file to be more involved and explanative
* Do a proper deployment test
* Add some form of testing and calibration code to make seting up things easier for others
* Add proper backwards communication (from server to gateway to end node) for changing settings
* Convert to new lower power Arduinos
* Add ability to change settings using bluetooth
* Convert Arduino code to use an event loop
* Add remoteConfig.h configurator to the website to make configuration a lot easier
* Robustify both the Gateway and End Node to ensure proper handling of errors
* Add ability to send error to website from node
* Add ability to change the time period that you're viewing data for to the server
* Add encryption for messages between nodes
* Add encryption to saved data on server
* Add ability to add login page to view data on the website, maybe with different amounts of power (ex: view data vs. change node settings)
* Add verification to ensure that data coming from an end node is legitimate, same for gateway to server
* Document how to change all modules on nodes (ex: data saving, communication method, etc.)
* Combine code from VictorVezina19 and RyanTyrell19 together (combine Turtle tracker project with this one)
	* Namely the EndData/EEPROM, Sleep/LowPower, Config, and GPS/UBX+NeoGPS libraries
* Add data point validation to the server (remove outliers)
* Add server alerts for important data changes (spikes, dips, etc.)
* Add data analysis capabilities to the server
* Fix floating point rounding causing weird values
* Change names in data post to gateway to use actual chars instead of hex representation of chars
* Reorganize entire arduino-motes repository to make more sense and combine similar code
