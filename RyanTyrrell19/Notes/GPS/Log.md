# Read/write UBX commands to/from Arduino

* The GPS reciever can output NMEA, PUBX or the binary UBX protocol
    * NMEA is the standard protocol with basic GPS data
    * PUBX (Proprietary UBX Messages) is essentailly the same as NMEA (in terms of data available). The only difference is how the data is formatetd for quicker transmission
    * Binary UBX protocol relates to all the various commands you can send, and the multitude of data you may recieve. T

* There are limited resources on how to read and write UBX commands using the Arduino. Most sites limit themselves to reading only the GPS data using UBX.

* I2C buffer is limited to 32 bytes (not sure if this value is programmable)

## Potential Libraries

* https://github.com/AuraUAS/UBLOX
    * Was unable to get this library to work. I belive this is because it is only compatible with UBLOX 7 or 8
* https://github.com/sparkfun/SparkFun_Ublox_Arduino_Library
    * This library is tailored to specific Sparkfun GPS modules


# Connecting GPS to Hardware Serial, and still viewing debugging data [[1]] [[2]]

* It is recommended to connect the GPS reciever to the Hardware Serial pins of the Arduino. Unfortunetly, the Arduino Uno, Nano, and Pro Mini only have one, and it is used for viewing debugging data via the Serial Monitor
* To remedy this,
    * The GPS will use the Hardware Serial Pins
    * Using an FTDI-to-USB adapter, connect the Arduino to the computer.
    * AltSoftware Serial is the next best library to Hardware Serial, and so it will be used for recieving debugging infor
        * on the UNO, Nano, and Mini, the RX/TX are Pins D8/D9, respectively
    * Open putty, select the correct COM port, and read the debugging messages
* UPDATE: As of right now, the GPS is connected to the Hardware pins, the debug messages are being output through the standard connector, and no issues are arising
## Conclusion
* You can debug and read GPS data from the Hardware pins, the data may conflict. It is safer to debug using AltSoftware (or Software Serial) and read the debug messages using putty

## Resources
[1]: https://ava.upuaut.net/?p=757  
[2]: https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html

# Does the GPS have an RTC?

* Theory: Yes, it does have an RTC. For it to work, it requires
    * a GPS fix so it knows what to set the time to
    * a constant supply of power to contineu running (i.e. a cell battery)


# If looking into noise again, start here (may be useful for figuring out what antenna to use)

https://ava.upuaut.net/?p=836