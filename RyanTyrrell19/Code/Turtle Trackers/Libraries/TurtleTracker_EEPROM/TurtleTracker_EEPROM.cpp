/*
 Name:		TurtleTracker_EEPROM.cpp
 Created:	8/4/2019 12:13:41 PM
 Author:	tyrre
 Editor:	http://www.visualmicro.com
*/

#include "TurtleTracker_EEPROM.h"

TurtleTracker_EEPROM::TurtleTracker_EEPROM() {

	setSavedPayloadsCounter(0);
}

void TurtleTracker_EEPROM::savePayload(char *payload) {

	// Compute the starting point within the EEPROM to get the desired data.
	uint16_t address = ADDRESS_STARTPOINT + (RH_PAYLOAD_LEN * getSavedPayloadsCounter());

	for (int i = 0; i < RH_PAYLOAD_LEN; i++)
		EEPROM.write(address + i, payload[i]);

	setSavedPayloadsCounter(getSavedPayloadsCounter() + 1);

}

void TurtleTracker_EEPROM::getSavedPayload(uint8_t count, char *payload) {
	// Compute the starting point within the EEPROM to get the desired data.
	uint16_t address = ADDRESS_STARTPOINT + (RH_PAYLOAD_LEN * count);

	for (int i = 0; i < RH_PAYLOAD_LEN; i++)
		payload[i] = EEPROM.read(address + i);
}

void TurtleTracker_EEPROM::setSavedPayloadsCounter(uint8_t count) {
	_savedPayloadsCounter = count;
}
uint8_t TurtleTracker_EEPROM::getSavedPayloadsCounter() {
	return _savedPayloadsCounter;
}

