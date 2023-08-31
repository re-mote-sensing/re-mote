The following measurements were obtained with the following modules attached to the Arduino Nano:

1. Atlas Scientific Electrical Conductivity (EC)
2. Atlas Scientific Dissolved Oxygen (DO)

3. Turbidity Sensor
4. Temperature Sensor
5. GPS Module
6. LoRA Module

To order the correct products, please read the article [here](../../Setup/Water_Quality_Setup.md) and read the 'The Hardware You Need' section.


**Key Points to Remember:**

- Power is supplied to this end node using 3.7V battery, and a multimeter measures the current flowing out of the positive terminal of the battery

- Sensor Node activates every hour for the duration of program execution, and sleeps for the rest of the time

- Program Execution Duration - please note that these numbers can vary according to micro-controllers performance

  | Stage                                     | Time [seconds] |
  | ----------------------------------------- | -------------- |
  | Initialization                            | 2              |
  | Reading Sensor & GPS Fix                  | 30-40          |
  | Saving data to EEPROM & LoRA Transmission | 5-10           |

  Since the program will be flashed (program loaded using USB) onto the micro-controller before deploying it to the field, the initialization stage will not impact the battery life. Thus, the following calculations will not consider the effects of peak currents in the initialization process.

- As you can see, total time to complete the process is 37-52 seconds (addition of time in different stages). However, in the following calculations, we will be using 2 minutes as an upper limit of wake-time, this is done to account for any glitches/anomalies.

- Assumption, as discussed in the previous point, wake mode is 2 minutes every hour:

  - 2 minutes = 0.0333 hour
  - system wakes up for 2 minutes every hours,and 24 hours per day equals to (24 * 0.0333hours) = 0.8hours wake time in a day
  - Remaining time in a day is (24 hours - 0.8 hours) = 23.3 hours, this is the sleep time per day 



###### Sensor End Node (Nano Unmodified):

| Mode          | Current Reading [milli-Ampere (mA)] |
| ------------- | ----------------------------------- |
| Run-time/wake | 150                                 |
| Sleep Mode    | 62                                  |

Based on these readings, we can predict the battery life of a 3.7V - 4400mAh battery:

| Total current drawn from the battery in a day | ( 0.15[A] \* 0.8[hour] ) + ( 0.062[A] * 23.2[hour] ) = 1.5584Ah (Amp-hour) |
| --------------------------------------------- | ------------------------------------------------------------ |
| Average Current per day, Iavg                 | 1.5584[Ah] / 24[h] = 0.0649A = 64.9mA                        |
| Battery Lifetime (Efficiency Factor - 95%)    | ( 4400[mAh] * 0.95 ) / 64.9[mA] = 64.40 hours = 2.68 days    |

------



In order to increase the battery life, we need to make following modifications on the Arduino board:

- Remove Power LED, Built-in LED and Voltage Regulator



After implementing these changes, the following results were obtained:

| Mode          | Current Reading [milli-Ampere (mA)] |
| ------------- | ----------------------------------- |
| Run-time/wake | 137-138                             |
| Sleep Mode    | 50                                  |



Battery-Life Calculations:

| Total current drawn from the battery in a day | ( 0.138[A] \* 0.8[hour] ) + ( 0.050[A] * 23.2[hour]) = 1.2704Ah |
| --------------------------------------------- | ------------------------------------------------------------ |
| Average Current per day, Iavg                 | 1.2704[mAh] / 24[h] = 52.9mA                                 |
| Battery Lifetime (Efficiency Factor - 95%)    | ( 4400[mAh] * 0.95 ) / 52.9[mA]             =          79.02 hours  = 3.29 days |

------

During Sleep-Mode, power consumption of each module is listed below:

| Module                       | Current          |
| ---------------------------- | ---------------- |
| Solar Module* - Unmodified** | 5 milli-Amps     |
| Solar Module - Modified***   | 173 micro-Amps   |
| LoRA Mesh                    | 10-12 milli-Amps |
| Sensors                      | 3-5 milli-Amps   |

\* Solar module consumes power when it is not exposed to the sunlight (during night/shady days). This module contains a voltage regulator, power-boost, solar panel and battery. 

** No alterations were made on the circuitry supplied by Adafruit

*** Removed all the LEDs from both of the solar module chips (in total, 5 LEDs on the boards)

------

To replicate this test on your End-Node, re-create the entire setup as described in the setup file, except connect the positive terminal of the battery with the positive lead of the multimeter and the negative terminal of the multimeter must be connected to the wire which was required to be connected with the positive terminal of the battery (please make sure that your multimeter is in series connection with the circuit before moving forward with the test).