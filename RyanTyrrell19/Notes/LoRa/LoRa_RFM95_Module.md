# Overview

# Neat Features

* Low Battery Detector
	* A low battery detector is also included allowing the generation of an interrupt signal in response to the supply voltage dropping below a programmable threshold that is adjustable through the register RegLowBat. The interrupt signal can be mapped to any of the DIO pins by programming RegDioMapping.


* Beacon Mode
	* In some short range wireless network topologies a repetitive message, also known as beacon, is transmitted periodically by a transmitter. The Beacon Tx mode allows for the re-transmission of the same packet without having to fill the FIFO multiple times with the same data.

* Built-in Temperature Sensor
	* A stand alone temperature measurement block is used in order to measure the temperature in any mode except Sleep and Standby. 
	* Due to process variations, the absolute accuracy of the result is +/- 10 °C. Higher precision requires a calibration procedure at a known temperature. The figure below shows the influence of just such a calibration process.



# Links with Key Notes Summarized




* [Ultra low power listening mode for battery nodes][Site 3]
	* 
*
* [Small loop antennas @ 433 Mhz][Site 4]
	* Check out later

* [Build LoRa node using Arduino Uno and HopeRF RFM95 LoRa transceiver module][Site 5]


* [Build a copper 868MHz coil antenna][Site 6]


# [RadioHead Library][RadioHead Library]
* Caution: Developing this type of software and using data radios successfully is challenging and requires a substantial knowledge base in software and radio and data transmission technologies and theory. It may not be an appropriate project for beginners. If you are a beginner, you will need to spend some time gaining knowledge in these areas first.


** Check out this site for buying a [Helical Antenna][Helical Antenna Site]**


[Site 1]: https://lowpowerlab.com/forum/low-power-techniques/any-success-with-lora-low-power-listening/
[Site 2]: https://lowpowerlab.com/forum/low-power-techniques/using-listenmode-as-wakeup-timer/15/
[Site 3]: https://lowpowerlab.com/forum/low-power-techniques/ultra-low-power-listening-mode-for-battery-nodes/
[Site 4]: https://lowpowerlab.com/forum/rf-range-antennas-rfm69-library/small-loop-antennas-433-mhz/

[Site 5]: https://www.mobilefish.com/developer/lorawan/lorawan_quickguide_build_lora_node_rfm95_arduino_uno.html

[Site 6]: https://www.mobilefish.com/developer/lorawan/lorawan_quickguide_build_868mhz_coil_antenna.html

[Helical Antenna Site]: https://fr.aliexpress.com/wholesale?SearchText=sw868-th13&switch_new_app=y

[RadioHead Library]: https://www.airspayce.com/mikem/arduino/RadioHead/index.html