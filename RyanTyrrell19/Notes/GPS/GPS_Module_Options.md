* Price range?
* Probably better off buying a pre-made board rather than design our own PCB. A lot of things that need to be considered when designing your own board
 
 
 
 [GPS Receiver - GP-20U7][GPS Receiver - GP-20U7 Specs]:
 * 56-Channel Receiver (22 Channel All-in-View)
 * Sensitivity : -162dBm
 * 2.5m Positional Accuracy
 * Cold Start : 29s (Open Sky)
 * 40mA @ 3.3V

 [This site][9] contains an extensive list of various GPS reciever and their highlights. A few have been summarized above.

[NEO-6M GPS Chip][NEO-6M GPS Chip Data Sheet]:
This GPS module continues to appear as an excellent choice for low-power, low-cost projects. A few f the key features are:
* _Power Save Mode(PSM)_: It allows a reduction in system power consumption by selectively switching parts of the receiver ON and OFF. This dramatically reduces power consumption of the module to **just 11mA** making it suitable for power sensitive applications like GPS wristwatch [[4]]

* The breakout board include a battery and EEPROM for memory storage. Utilizing these retains clock and last position, causing **time to first fix (TTFF) to reduce to 1s**.[[14]]
    * Without the battery the GPS always cold-start so the initial GPS lock takes more time [[14]]
    * The battery is automatically charged when power is applied and maintains data for up to **two weeks without power.** [[14]]

* Both modules support UBX and so are compatible with **U-center from u-blox** for evaluation, performance analysis and configuration of u-blox GPS receivers including NEO-6M

* The EEPROM on the Board communicates with the NEO-6M chip using I2C
* To save data (such as Assit Offline) an external memory (Flash) must be connected to the chips SPI header.

![alt text][NEO-6M GPS Board Schematic]