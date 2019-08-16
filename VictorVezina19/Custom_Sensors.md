# How to Add a Custom Sensor to re:mote

This document details how to add a custom sensor to your re:mote nodes. This document assumes some Arduino (or at least C/C++) experience.

## Choose a sensor

The first step is to, of course, choose a sensor you want to add to your nodes. The sensor you choose would ideally be advertised as working with Arduino, but as long as it can be interfaced with using what's available on your micro-controller it should work.

## Interface with the sensor

Technically this step could be skipped, but it will make adding a sensor much easier. First learn how to interface with the sensor you're planning to add, then write a function that will allow the Arduino to interface with it, and test to make sure it works correctly. This is done easiest within it's own small Arduino program, as it will allow you to easily debug any issues that arise.

## Integrate the custom function with the library

To integrate your sensor with the remoteSensors library, you will need to do a few things. 

The first thing is to add a custom macro definition for your sensor. Choose a name that describes your sensor well.

The second thing, although it isn't necessary, is to add an initialisation function to the initialise() function in remoteSensors.cpp. To do this, you must add a new conditional compilation block, similar to the ones already there. Make this block check the current sensor type string against whatever string you want to name your sensor, something ery similar to the macro you defined is ideal. Within this check, call your initialisation function. Your initialisation function should do whatever needs to be done once for your sensor to work (ex: setting pin modes, configuring sensor settings, etc.).

The third, and final, thing you will need to do is add a sensor read function into the library's read() function. You will first need to add a conditional compilation block to the function, same as you did in step 2. This will block will check if the current sensor type string is the same as whatever you wish to name your sensors, once again the same as you did in step 2. Within this if statement, you will want to add the value returned by your sensor read function to the curr variable. What this does is increment the index of the data array by how much data you've added. This means that your sensor read function will have to have the following API:

* It must take as input a uint8_t representing the index of the sensor in the sensorTypes array.
* It will also take as input the pointer to a position in a uint8_t array representing the place to add the read sensor data.
* It will return a uint8_t representing the number of bytes that was written to the passed in array.

To work properly with the Gateway and server, your sensor should read and save the read values into the data array as 32 bit floats. Your sensor may read as many values as you want (ex: DHT22 reads Air_Temperature and Humidity), just be sure to add a unique string to sensorNames for every float your sensor reads. If you wish to compensate for temperature or salinity, you can add code to check for the Temperature\_Comp and Salinity\_Comp macros, and use the class variables lastTemp and lastSal as the values to compensate with. It is also important to note that if you plan on using serial communication to interface with the sensor, you must use NeoSWSerial, which only supports baud rates of 9600, 19200, 31250, and 38400.
