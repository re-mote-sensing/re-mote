Gateway Node consists of the following:

1. Arduino UNO
2. FONA 3G by Adafruit
3. SD Card
4. DHT Sensor
5. LoRA Mesh



**Key Points to Remember:**

- Current measurements taken at the positive terminal of the 3.7V battery (to measure current - connect the multimeter in series with the circuit, next to the +ve terminal of the battery)
- Although Adafruit recommends to use a separate battery for FONA 3G, these readings are taken without attaching a battery with FONA3G (a separate battery might be required to make calls, LTE connections, etc.)
- nA = nano-Amperes, uA = micro-Amperes, mA = milli-Amperes



**Observations:**

- Initial current drawn ranges between 100-210mA (with sudden spikes of 300mA) for 8-10 seconds
- Reading the sensor phase consumes about 245mA
- Posting the data can reach upto 370mA
- When waiting for LoRA messages, current is 108mA (at this stage, FONA is essentially idle)



Since, the gateway node is always ON and the current draw is rapidly changing and too much (in comparison to End-Node), **<u>it is strongly recommended to use a high capacity battery to help solar module in supplying the power to the gateway node.</u>** 



------

**Gateway Node Test of each individual component:** For pin-to-pin current flow, read the following tables:

| FONA 3G                                                      |                                                              |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| <u>**Run-time (takes about 40-43 seconds to finish this process):**</u> |                                                              |
| 5V Arduino to 5V FONA                                        | range of 65-180mA during posting and getting a fix for the GPS (current spikes upto 196mA for moments once or twice during each post/fix) |
| 5V Arduino to Vio Pin                                        | range of 3.5-6.6mA during posting and getting a fix for the GPS |
| Digital I/O Pin to Rx                                        | 17nA (spike of 5uA when reading sensors for a split second)  |
| Digital I/O Pin to Tx                                        | 2nA                                                          |
| Digital I/O Pin to Key                                       | 7.5uA (value when reading and posting the data, bounces between 14nA to 7.5uA all the time in this phase) |
| Ground to Arduino GND                                        | range of 55-190mA during posts and fixes                     |
| **<u>Idle (Period when waiting and receiving for LoRA messages):</u>** |                                                              |
| 5V Arduino to 5V FONA                                        | 3mA                                                          |
| 5V Arduino to Vio Pin                                        | 0mA                                                          |
| Digital I/O Pin to Rx                                        | 17nA                                                         |
| Digital I/O Pin to Tx                                        | 2nA                                                          |
| Digital I/O Pin to Key                                       | 18nA                                                         |
| Ground to Arduino GND                                        | 3mA                                                          |



| SD CARD                 |                                                              |
| ----------------------- | ------------------------------------------------------------ |
| 5V Arduino to Vcc       | always consuming 3.26mA (range of 4.5-5.6mA during initialization stage ~ about 2-3 seconds) |
| Digital I/O Pin to MISO | 2nA                                                          |
| Digital I/O Pin to MOSI | 2nA                                                          |
| Digital I/O Pin to SCK  | 0A                                                           |
| Digital I/O Pin to CS   | 2nA                                                          |
| Ground to Arduino GND   | 3.28mA                                                       |



| DHT SENSOR              |                                                              |
| ----------------------- | ------------------------------------------------------------ |
| 5V Arduino to Vcc       | always 11.5uA (spike of 80uA (momentary rise) when reading sensor) |
| Digital I/O Pin to Data | range of 0.1-0.3uA (reading and posting the data)            |
| Ground tp Arduino GND   | about 11.5uA (bounces around 11uA)                           |



| LoRA Mesh             |                                                              |
| --------------------- | ------------------------------------------------------------ |
| 5V Arduino to Vcc     | 11.2mA when idle (spike of 19mA when receiving LoRA message - for less than a second) |
| Digital I/O Pin to Rx | range of 4.3-7mA ~ mostly at 7mA                             |
| Digital I/O Pin to Tx | 0A                                                           |
| Ground to Arduino GND | around 18mA (although can go down to 16mA for a second)      |



| Arduino UNO                                                  |
| ------------------------------------------------------------ |
| With no load code, Uno can draw current of 45-50mA. Our code draws 78-79mA of current (in the initialization mode) |



| SOLAR POWER MODULE                                           |
| ------------------------------------------------------------ |
| Draws about 5mA of current when not exposed to sunlight      |
| After removing all the LEDs (5) from the module, solar module only consumes 173micro-Amps |
