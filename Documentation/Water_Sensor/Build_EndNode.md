# Build a Sensor Node

## Get all parts ready

[Specification](./Documentation/Hardware/Specification.md)

## Arduino Assembly

Using [IO Sensor Shield For Arduino Mega](https://www.dfrobot.com/product-560.html) can make cable connection easier.

<img src="Documentation/Pictures/water_arduino/Water_Sensor_End_Node_2023_bb.png">

**pH Sensor:**

- Connect PH-Vcc to Arduino-5V
- Connect PH-GND to Arduino-GND
- Connect PH-OFF to Arduino-A7
- Connect PH-Tx to Arduino-A10
- Connect PH-Rx to Arduino-A11

**Electrical Conductivity (EC) Sensor:**

- Connect EC-Vcc to Arduino-5V
- Connect EC-GND to Arduino-GND
- Connect EC-OFF to Arduino-A9
- Connect EC-Rx to Arduino-A15
- Connect EC-Tx to Arduino-A14

**Dissolved Oxygen (DO) Sensor:**

- Connect DO-Vcc to Arduino-5V
- Connect DO-GND to Arduino-GND
- Connect DO-OFF to Arduino-A8
- Connect DO-Tx to Arduino-D12
- Connect DO-Rx to Arduino-D13

**Turbidity Sensor:**

- Connect Turbidity-PWR to Arduino-A4
- Connect Turbidity-GND to Arduino-GND
- Connect Turbidity-D to Arduino-A5
Important Note: Turbidity's Data pin needs Analog connection to the Arduino.

**Temperature Sensor:**

- Connect Temperature-Vcc to Arduino-5V
- Connect Temperature-GND to Arduino-GND
- Connect Temperature-D to Arduino-D49

## Completed Arduino

<img src="Documentation/Pictures/water_arduino/IMG_2225.jpeg">