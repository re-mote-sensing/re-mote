# 3G Shield Baud Adjustment

Do this after the hardware is assembled

## Install Library

To begin reading sensor data, you will need to Install Library as follows:

Download [Arduino Library](https://www.tinyosshop.com/download/Adafruit_FONA.zip) and unzip it.
Copy the Adafruit_FONA folder to the Arduino libraries folder. Typically in your (home folder)/Documents/Arduino/Libraries folder.
Re-start the Arduino IDE if it is currently running.

## 3G Baud Adjustment

Open the `./FONA3G_setBaud` example under this folder, this program only needs to run once, what it does is set the baudrate from 4800 to 9600.

After the serial print out the IMEI number, the 3g shield is now ready to use.

## Finished

You can delete the library if you want.