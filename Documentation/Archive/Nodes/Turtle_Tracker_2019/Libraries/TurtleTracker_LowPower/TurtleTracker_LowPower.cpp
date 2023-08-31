/*
 Name:		TurtleTracker_LowPower.cpp
 Created:	7/29/2019 11:57:51 AM
 Author:	tyrre
 Editor:	http://www.visualmicro.com
*/

#include "TurtleTracker_LowPower.h"

TurtleTracker_LowPower::TurtleTracker_LowPower(HardwareSerial &debugPort) {
	
	_debugPort = &debugPort;
	setFixCounter(0);
	setSleepType(LONG_SLEEP);
}


uint32_t TurtleTracker_LowPower::computeSleepTime(uint8_t wakeHour, uint8_t wakeMin, uint8_t currentHour, uint8_t currentMin, uint8_t currentSec) {

	// Compute difference between target time and real time (in minutes)
	uint8_t hourDiff = 0;
	uint8_t minDiff = 0;
	uint8_t secDiff = 0;

	/*	DEBUG_PORT.println();
	_debugPort->println(fix.dateTime.hours);
	_debugPort->println(fix.dateTime.minutes);
	_debugPort->println(wakeHour);
	_debugPort->println(wakeMin);
	_debugPort->println();
	*/

	// Hour resets to 0 after 23. Minutes resets to 0 after 59. Below code checks for cases when this reset occurs, and changes the 
	// sleep time accordingly. In addition, the scenario where the current time is just ahead of the target wake-up time is tested for
	// (in which case an entire days should elapse versus just an hour)


	wakeHour = (currentHour > wakeHour) ? (wakeHour + 24) : wakeHour;

	if (currentHour == wakeHour && wakeMin < currentMin)
		hourDiff = 23;
	else
		hourDiff = wakeHour - currentHour;

	if (wakeMin > currentMin) {
		minDiff = wakeMin - currentMin;
	}
	else {
		minDiff = (wakeMin + 60) - currentMin;
		hourDiff--;
	}
	secDiff = 60 - currentSec;
	minDiff--;

	uint32_t totalDiff = (hourDiff * 60L * 60L) + (minDiff * 60L) + secDiff;

	_debugPort->print(F("Target Sleep Time:   "));
	_debugPort->println(totalDiff);

	// Watchdog timer can be off by aprrox. 10%. To be safe, subtract 15% of time difference off of itself to ensure next
	// wakeup does not occur past target wakeup time
	uint32_t sleepTime = totalDiff - (totalDiff * WATCHDOG_CORRECTION);

	_debugPort->print(F("Corrected Sleep Time:   "));
	_debugPort->println(sleepTime);

	return sleepTime;
}

/*
Determines which sleep interval to use (Short, Long, or Constant)
*/
bool TurtleTracker_LowPower::enterLowPowerMode(uint8_t currentHour, uint8_t currentMin, uint8_t currentSec, bool isRtcValid) {

	// default sleepTime is 1 hour (for cases where RTC is not available)
	uint32_t sleepTime = (CONSTANT_SLEEP * 60);
	uint8_t sleepAgain = getSleepType();
	setSleepType(NO_SLEEP);

	if (isRtcValid) {
		// The Tracker is currently in the Long Sleep Interval Period
		if (getFixCounter() == NUM_FIXES_PER_DAY || sleepAgain == LONG_SLEEP) {
			setFixCounter(0);
			sleepTime = computeSleepTime(WAKE_TIME_HOUR, WAKE_TIME_MIN, currentHour, currentMin, currentSec);

			if (sleepTime > SLEEP_THRESHOLD)
				setSleepType(LONG_SLEEP);

		}
		// The Logger is in the Short Sleep Interval Period
		else {

			uint8_t timeSinceWake = getFixCounter() * SHORT_SLEEP_TIME;
			uint8_t wakeHour = WAKE_TIME_HOUR + (timeSinceWake / 60);		// intermediate calculations are not subject to rounding
			uint8_t wakeMin = WAKE_TIME_MIN + (timeSinceWake % 60);

			// Below accounts for when switching over to new hour or start of new day
			if (wakeMin >= 60) {
				wakeHour += 1;
				wakeMin -= 60;
			}
			if (wakeHour >= 24) {
				wakeHour -= 24;
			}

			sleepTime = computeSleepTime(wakeHour, wakeMin, currentHour, currentMin, currentSec);

			if (sleepTime > SLEEP_THRESHOLD) {
				setSleepType(SHORT_SLEEP);
			}
		}
	}

	_debugPort->println("Going to Sleep\n");
	delay(500);
	for (int i = 0; i < sleepTime; i++) {
		LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
	}
	delay(500);
	_debugPort->println("Waking Up\n");

	if (getSleepType() == NO_SLEEP) {
		setFixCounter(getFixCounter() + 1);
		return true;
	}
	else
		return false;
}

void TurtleTracker_LowPower::setFixCounter(uint16_t count) {
	_fixCounter = count;
}

uint8_t TurtleTracker_LowPower::getFixCounter() {
	return _fixCounter;
}


void TurtleTracker_LowPower::setSleepType(uint8_t type) {
	_sleepType = type;
}
uint8_t TurtleTracker_LowPower::getSleepType() {
	return _sleepType;
}
