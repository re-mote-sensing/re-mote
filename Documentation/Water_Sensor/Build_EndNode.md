# Build a Sensor Node

## Get all parts ready.

[Specification](./Documentation/Water_Sensor/Specification.md)

## Arduino Assembly

**KEY POINTS:**  

- All modules have a power pin (commonly known as Vcc) and a ground pin (GND).

- Vcc pin of all the modules must be connected to the 5V pin of Arduino. 

  - Before connecting any sensors to the 5V supply from the Arduino, please make sure that the sensors can operate at the 5V input supply. This can be easily confirmed from the product datasheet.

- For all of the modules except the Turbidity sensor, pins such as Rx(Receiving), Tx(Transmitting), and En(Enable) or OFF can be connected to either analog or digital input/output pins of Arduino. This is because Arduino's analog pins can be easily configured and used as digital I/O pins.
  - In this project, the Turbidity sensor is transceiving signals in analog form. Please remember to only connect the turbidity sensors data pin to an analog pin of Arduino.

- After you have finished connecting all the pins as discussed below, please keep in mind that you are required to switch the Rx and Tx pins of all the sensors (except temperature and turbidity sensors) in the software.

- In the software, Arduino's digital I/O pins can be assigned by using their pin numbers, and Arduino Nano has 13 digital I/O pins. In case analog pins are needed to function as digital I/O pins, use the following numbers:

  | Analog pins | Designated Number for the Software |
  | ----------- | ---------------------------------- |
  | A0          | 14                                 |
  | A1          | 15                                 |
  | A2          | 16                                 |
  | A3          | 17                                 |
  | A4          | 18                                 |
  | A5          | 19                                 |

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
Important Note: Turbidity's Data pin needs analog connection to the Arduino

**Temperature Sensor:**

- Connect Temperature-Vcc to Arduino-5V
- Connect Temperature-GND to Arduino-GND
- Connect Temperature-D to Arduino-D49

## Completed Arduino

<img src="Documentation/Pictures/water_arduino/IMG_2225.jpeg">