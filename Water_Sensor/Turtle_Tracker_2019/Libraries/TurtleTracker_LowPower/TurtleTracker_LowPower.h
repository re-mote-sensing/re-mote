/*
 Name:		TurtleTracker_LowPower.h
 Created:	7/29/2019 11:57:51 AM
 Author:	tyrre
 Editor:	http://www.visualmicro.com
*/

#ifndef _TurtleTracker_LowPower_h
#define _TurtleTracker_LowPower_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <LowPower.h>      
#include <TurtleTracker_Config.h>


/*------------------------------------------------------------------------------------*/


class TurtleTracker_LowPower {
	public:
		/*
		*	Default Constructor
		*
		*	debugPort:	Serial Port used for providing helpful output on the state of the program
		*/
		TurtleTracker_LowPower(HardwareSerial &debugPort);

		/*
		*	Used to initiate the process of putting the Arduin to sleep
		*
		*	currentHour:	Current hour of the day
		*	currentMin:		Current minute of the day
		*	currentSec:		Current second of the day
		*
		*	returns: whether the Arduino should call this function again or not
		*/
		bool enterLowPowerMode(uint8_t currentHour, uint8_t currentMin, uint8_t currentSec, bool isRtcValid);

	private:
		/*
		*	Used to determine, in seconds, how long the Arduino should sleep for. Takes into account the error associated with the Watcdog Timer
		*
		*	wakeHour:		Desired hour to wakeup
		*	wakeMin:		Desired minute to wake up
		*	currentHour:	Current hour of the day
		*	currentMin:		Current minute of the day
		*	currentSec:		Current second of the day
		* 
		*	return: Time to sleep, in seconds
		*/
		uint32_t computeSleepTime(uint8_t wakeHour, uint8_t wakeMin, uint8_t currentHour, uint8_t currentMin, uint8_t currentSec);

		/*
		* Setter/Getter function to set/get the global varaible, _fixCounter
		*/
		void setFixCounter(uint16_t count);
		uint8_t getFixCounter();

		/*
		* Setter/Getter function to set/get the global varaible, _sleepType
		*/
		void setSleepType(uint8_t type);
		uint8_t getSleepType();

		// Counter for the number of fixes aquired so far. Accessed via getter & setter
		uint8_t _fixCounter;							

		// Keeps track of whether the next sleep should be SHORT, LONG, or NONE. Accessed via getter & setter
		uint8_t _sleepType;	
								
		// Pointer to the Serial object defined in the main program and passed via the constructor
		// Used for providing helpful output on the state of the program
		HardwareSerial *_debugPort;
};

#endif

