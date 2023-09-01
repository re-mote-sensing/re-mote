# Build a Gateway

## Get all parts ready.

[Specification](/Documentation/Water_Sensor/Specification.md)

## Breakdown of all connections:

**SD Card:**

This module requires SPI communication with the Arduino. Thus, it is imperative to read about the SPI pins of the specific Arduino board you will be using. This information can be found in the 'Input and Output' section of your Arduino Board. Arduino Uno in the specification page is supported as following pins.

- Connect SD-Card-CS to Arduino D10
- Connect SD-Card-SCK to Arduino D13
- Connect SD-Card-MOSI to Arduino D11
- Connect SD-Card-MISO to Arduino D12
- Connect SD-Card-Vcc to Arduino 5V
- Connect SD-Card-GND to Arduino GND

**DHT22 Sensor:**

- Connect DHT-Vcc to Arduino 5V
- Connect DHT-GND to Arduino GND
- Connect DHT-D to Arduino D5

**FONA 3G by Adafruit:**

- Connect FONA-Vcc to Arduino 5V
- Connect FONA-GND to Arduino GND
- Connect FONA-Rx to Arduino D4
- Connect FONA-Tx to Arduino D3
- Connect FONA-Key to Arduino D2

**LoRA:**

- Total of 7 pins on the chip: Used Tx, Rx, Vcc, and GND (IGNORE SET, AUX, and EN pins)
- Connect the Vcc of LoRA to the 5V of Arduino Nano
- Connect GND of LoRA to GND of Arduino Nano
- Connect LoRA Tx to Arduino D6
- Connect LoRA Rx to Arduino D7

------

#### Optimize battery life (Optional and require soldering skill):

- Remove unnecessary LEDs and voltage regulator from the Arduino or any other module. 

  <u>Need to know how?</u> Click on [Link#1](http://www.home-automation-community.com/arduino-low-power-how-to-run-atmega328p-for-a-year-on-coin-cell-battery/), [Link#2](https://www.youtube.com/watch?v=2nNz0faMti0), [Link#3](https://www.youtube.com/watch?v=2xVOg7nYH-E), [Link#4](https://www.youtube.com/watch?v=7qujkC72dYs)