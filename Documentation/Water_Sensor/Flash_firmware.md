## The Software You Need

### Arduino IDE

First, you'll need the [Arduino IDE](https://www.arduino.cc/en/main/software). This will be used to upload the code from your computer to the Arduino compatible micro controller. If you want, you could also use the Arduino web editor, or any other IDE that supports uploading .ino files to Arduinos.

### Program file

Next, you'll need to download [Node_Code.ino](Water_Sensor/Node_Code/Node_Code.ino) from this repository. This is the main program that runs on the Arduino.

### Libraries

You'll then need to download the libraries from this repository, and add some others from the IDE. First, copy all files found under [Libraries](Water_Sensor/Libraries/) into your [Arduino library folder](https://www.arduino.cc/en/guide/libraries), 

- Mac: ~/Documents/Arduino/libraries/
- Windows: /Users/{username}/Documents/Arduino/libraries/

replacing what's already there (if anything).

### Select dev board

#### Using Arduino Mega

Select `Tools -> Board -> Arduino AVR Boards -> Arduino Mega` from the top bar

#### Using Moteino Mega

Install the `Moteino AVR Boards` package in Arduino IDE: [Programming & Libraries](https://lowpowerlab.com/guide/moteino/programming-libraries/)

Select `Tools -> Board -> Moteino AVR Boards -> Moteino Mega` from the top bar

---

Connect arduino to computer by usb cable or FTDI cable.

Open `Node_Code.ino` in Arduino App

## Flash

Flash with Arduino IDE by click the flash (->) button