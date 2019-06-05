# What I've done/tried

## Read/write UBX commands to/from Arduino

* The GPS reciever can output  NMEA, PUBX or the binary UBX protocol
    * NMEA is the standard protocol with basic GPS data
    * PUBX (Proprietary UBX Messages) is essentailly the same as NMEA (in terms of data available). The only difference is how the data is formatetd for quicker transmission
    * Binary UBX protocol relates to all the various commands you can send, and the multitude of data you may recieve. T

* The UBLOX protocol outputs the UBX commands 



* There are limited resources on how to read and write UBX commands using the Arduino. Most sites limit themselves to reading only the GPS data using UBX. Nothing on

* I2C buffer is limited to 32 bytes (not sure if this value is programmable)

# Connecting GPS to Hardware Serial, and still viewing debugging data

* It is recommended to connect the GPS reciever to the Hardware Serial pins of the Arduino. Unfortunetly, the Arduino Uno, Nano, and Pro Mini only have one, and it is used for viewing debugging data via the Serial Monitor
* To remedy this,
    * The GPS will use the Hardware Serial Pins
    * Using an FTDI-to-USB adapter, connect the Arduino to the computer.
    * AltSoftware Serial is the next best library to Hardware Serial, and so it will be used for recieving debugging infor
        * on the UNO, Nano, and Mini, the RX/TX are Pins D8/D9, respectively


## Resources
[1]: https://ava.upuaut.net/?p=757  
[2]: https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html
