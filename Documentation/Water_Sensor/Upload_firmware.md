## The Software You Need

### Arduino IDE

First, download and install the [Arduino IDE](https://www.arduino.cc/en/main/software) available on Windows, Linux and MacOS. This software allows you to upload code from your computer to the Arduino-compatible microcontroller. Alternatively, you can use the Arduino web editor or any other IDE that supports uploading .ino files to Arduino devices.

### Program file

Next, download the [Node_Code.ino](/Water_Sensor/Node_Code/Node_Code.ino) file from the provided repository. This file contains the primary program that will execute on the Arduino.

### Libraries

Next, you'll need to incorporate the required libraries. Start by downloading all the library files from the [Libraries](/Water_Sensor/Libraries/) directory of this repository. Once downloaded, copy all the files into your [Arduino library folder](https://www.arduino.cc/en/guide/libraries).

- Windows: /Users/{username}/Documents/Arduino/libraries/
- Linux: /home/{username}/Arduino/libraries/
- Mac: ~/Documents/Arduino/libraries/

Replacing what is already there (if anything).

### Select the development board

#### Using Arduino Mega

Select `Tools -> Board -> Arduino AVR Boards -> Arduino Mega` from the top bar.

#### Using Moteino Mega

Install the `Moteino AVR Boards` package in Arduino IDE: [Programming & Libraries](https://lowpowerlab.com/guide/moteino/programming-libraries/).

Select `Tools -> Board -> Moteino AVR Boards -> Moteino Mega` from the top bar.

---

Connect the arduino to the computer by USB cable or FTDI cable.

Open `Node_Code.ino` in Arduino App.

## Upload

Upload with Arduino IDE by clicking the upload (->) button.