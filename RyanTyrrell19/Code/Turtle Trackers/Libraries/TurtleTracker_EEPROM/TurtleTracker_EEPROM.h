/*
 Name:		TurtleTracker_EEPROM.h
 Created:	8/4/2019 12:13:41 PM
 Author:	tyrre
 Editor:	http://www.visualmicro.com
*/

#ifndef _TurtleTracker_EEPROM_h
#define _TurtleTracker_EEPROM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <TurtleTracker_Config.h>
#include <EEPROM.h>

// starts at 1 since nodeID is reserved for address 0
#define ADDRESS_STARTPOINT 1 

class TurtleTracker_EEPROM {

public:
	/*
	*	Default Constructor
	*/
	TurtleTracker_EEPROM();

	/*
	*	Saves the data to the microcontrollers EEPROM
	*
	*	data: the data to be saved
	*/
	void savePayload(char *data);

	/*
	*	Retreive saved data from the microcontrollers EEPROM
	*
	*	count: the address within the EEPROM to get the data from
	*	payload: pointer that will refernce the retrieved data
	*/
	void getSavedPayload(uint8_t count, char *payload);

	/*
	* Setter/Getter function to set/get the global varaible, _savedPayloadsCounter
	*/
	void setSavedPayloadsCounter(uint8_t count);
	uint8_t getSavedPayloadsCounter();

private:

	/*
	*	Used to keep track of how much data has been saved to the EEPROM
	*/
	uint8_t _savedPayloadsCounter;
};



#endif

