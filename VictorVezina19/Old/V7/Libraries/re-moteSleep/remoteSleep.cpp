/*
Library for sleeping, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated July 25, 2019
Released into the public domain
*/

#include "remoteSleep.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <remoteConfig.h>

remoteSleep::remoteSleep() {
}

//Sleep for a certain amount of time, accounting for the time since start
long remoteSleep::sleep(unsigned long time, unsigned long start) {
    long sleepTime; //The amount of time to sleep
    
    //Get how long to sleep, accounts for overflow
    if (millis() < start) {
        sleepTime = time - (millis() + (4294967295-start));
    } else {
        sleepTime = time - (millis() - start);
    }
    
    #ifdef DEBUG
    Serial.print(F("Sleeping for "));
    Serial.print(sleepTime);
    Serial.println(F(" milliseconds"));
    delay(250);
    #endif
    
    long ans = sleepTime; //Amount of time we will sleep
    
    //Sleep setup
    byte prevADCSRA = ADCSRA;
    ADCSRA = 0;
    set_sleep_mode(Sleep_Type);
    sleep_enable();
    
    //Sleep for the amount of time given
    
    //Sleep in bursts of 8 seconds until less than 8 seconds left
    while (sleepTime >= 8000) {
            sleep8000();
            sleepTime -= 8000;
    }
    
    //Sleep the rest of the time left
    if (sleepTime >= 4000) {
        sleep4000();
        sleepTime -= 4000;
    }
    
    if (sleepTime >= 2000) {
        sleep2000();
        sleepTime -= 2000;
    }
    
    if (sleepTime >= 1000) {
        sleep1000();
        sleepTime -= 1000;
    }
    
    if (sleepTime >= 500) {
        sleep500();
        sleepTime -= 500;
    }
    
    if (sleepTime >= 250) {
        sleep250();
        sleepTime -= 250;
    }
    
    if (sleepTime >= 125) {
        sleep125();
        sleepTime -= 125;
    }
    
    if (sleepTime >= 64) {
        sleep64();
        sleepTime -= 64;
    } 
    
    if (sleepTime >= 32) {
        sleep32();
        sleepTime -= 32;
    }
    
    if (sleepTime >= 16) {
        sleep16();
        sleepTime -= 16;
    }
    
    //Go back to normal
    sleep_disable();
    ADCSRA = prevADCSRA;
    
    #ifdef DEBUG
    delay(250);
    Serial.print(F("Slept for "));
    Serial.print(ans - sleepTime);
    Serial.println(F(" milliseconds"));
    #endif
    
    //Return the actual amount of time slept
    return ans - sleepTime;
}

// When WatchDog timer causes Arduino to wake it comes here
ISR (WDT_vect) {
  wdt_disable(); //Turn off watchdog
}

/*-------------------------------------------------------------*/
//The following functions are all the same, just varying in time
//For some reason having a simple if statement setting the time bits doesn't work, so here we are
//I will only comment the first one

//Sleep for 8000 ms
void remoteSleep::sleep8000() {
    //Turn off Brown Out Detection (low voltage)
    sleep_bod_disable();

    //Ensures we can wake up again by first disabling interrupts
    noInterrupts();

    //Clear various "reset" flags
    MCUSR = 0;

    //Set the sleep time bits
    WDTCSR = bit (WDCE) | bit(WDE);
    WDTCSR = bit (WDIE) | bit (WDP3) | bit(WDP0);

    wdt_reset();
    interrupts(); //Allow interrupts now
    sleep_cpu(); //Go to sleep
}

void remoteSleep::sleep4000() {
    sleep_bod_disable();
    noInterrupts();
    MCUSR = 0;
    WDTCSR = bit (WDCE) | bit(WDE);
    WDTCSR = bit (WDIE) | bit (WDP3);
    wdt_reset();
    interrupts();
    sleep_cpu();
}

void remoteSleep::sleep2000() {
    sleep_bod_disable();
    noInterrupts();
    MCUSR = 0;
    WDTCSR = bit (WDCE) | bit(WDE);
    WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1) | bit (WDP0);
    wdt_reset();
    interrupts();
    sleep_cpu();
}

void remoteSleep::sleep1000() {
    sleep_bod_disable();
    noInterrupts();
    MCUSR = 0;
    WDTCSR = bit (WDCE) | bit(WDE);
    WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1);
    wdt_reset();
    interrupts();
    sleep_cpu();
}

void remoteSleep::sleep500() {
    sleep_bod_disable();
    noInterrupts();
    MCUSR = 0;
    WDTCSR = bit (WDCE) | bit(WDE);
    WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP0);
    wdt_reset();
    interrupts();
    sleep_cpu();
}

void remoteSleep::sleep250() {
    sleep_bod_disable();
    noInterrupts();
    MCUSR = 0;
    WDTCSR = bit (WDCE) | bit(WDE);
    WDTCSR = bit (WDIE) | bit (WDP2);
    wdt_reset();
    interrupts();
    sleep_cpu();
}

void remoteSleep::sleep125() {
    sleep_bod_disable();
    noInterrupts();
    MCUSR = 0;
    WDTCSR = bit (WDCE) | bit(WDE);
    WDTCSR = bit (WDIE) | bit (WDP1) | bit (WDP0);
    wdt_reset();
    interrupts();
    sleep_cpu();
}

void remoteSleep::sleep64() {
    sleep_bod_disable();
    noInterrupts();
    MCUSR = 0;
    WDTCSR = bit (WDCE) | bit(WDE);
    WDTCSR = bit (WDIE) | bit (WDP1);
    wdt_reset();
    interrupts();
    sleep_cpu();
}

void remoteSleep::sleep32() {
    sleep_bod_disable();
    noInterrupts();
    MCUSR = 0;
    WDTCSR = bit (WDCE) | bit(WDE);
    WDTCSR = bit (WDIE) | bit (WDP0);
    wdt_reset();
    interrupts();
    sleep_cpu();
}

void remoteSleep::sleep16() {
    sleep_bod_disable();
    noInterrupts();
    MCUSR = 0;
    WDTCSR = bit (WDCE) | bit(WDE);
    WDTCSR = bit (WDIE);
    wdt_reset();
    interrupts();
    sleep_cpu();
}