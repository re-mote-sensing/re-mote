# Overview
* Changeable LoRa Parameters and their influence on transmission
* RFM95 Features
	* CAD Mode
* Radiohead Library
* Additional Notes
* Additional Resources


* Unless otherwise specified, this information came from the [RFM95 Datasheet][1]

# Changeable parameters and how they relate to one another
## Overview

* Frequency Range
* Spreading Factor
* Bandwidth
* Effective Bitrate (unimportant; kinda)
* Est. Sensitivity
* Coding Rate
* Transmission Power

* "So that it is possible to optimize the LoRa modulation for a given application, access is given to the designer to three critical design parameters. Each one permitting a trade off between link budget, immunity to interference, spectral occupancy and nominal data rate. These parameters are spreading factor, modulation bandwidth and error coding rate."

## Parameters
### Spreading Factor
* The spread spectrum LoRa modulation is performed by representing each bit of payload information by multiple chips of information. The rate at which the spread information is sent is referred to as the symbol rate (Rs), the ratio between the
nominal symbol rate and chip rate is the spreading factor and represents the number of symbols sent per bit of information. The range of values accessible with the LoRa modem are shown in the following table. [[1]]

![alt-text][Spreading Factor Table]

* Note that the spreading factor, SpreadingFactor, must be known in advance on both transmit and receive sides of the link as different spreading factors are orthogonal to each other. Note also the resulting signal to noise ratio (SNR) required at the receiver input. It is the capability to receive signals with negative SNR that increases the sensitivity, so link budget and range, of the LoRa receiver. [[1]]

* SF is the ratio between the symbol rate and chip rate. **A higher spreading factor increases the Signal to Noise Ratio (SNR), and thus sensitivity and range, but also increases the airtime of the packet.** The number of chips per symbol is calculated as 2^SF. For example, with an SF of 12 (SF12) 4096 chips/symbol are used. **Each increase in SF halves the transmission rate and, hence, doubles transmission duration and ultimately energy consumption.** **Spreading factor can be selected from 6 to 12.** As we have shown in previous work, radio communications with different SF are orthogonal to each other and network separation using different SF is possible [[2]]


### Coding rate

* To further improve the robustness of the link the LoRa modem employs cyclic error coding to perform forward error detection and correction. Such error coding incurs a transmission overhead - the resultant additional data overhead per transmission is shown in the table below.[[1]]

![alt-text][Coding Rate Table]

* Forward error correction is particularly efficient in improving the reliability of the link in the presence of interference. So that the coding rate (and so robustness to interference) can be changed in response to channel conditions - the coding rate can optionally be included in the packet header for use by the receiver. Please consult Section 4.1.1.6 for more information on the LoRa packet and header.[[1]]

* CR is the FEC rate used by the LoRa modem that offers protection against bursts of
interference, and **can be set to either 4/5, 4/6, 4/7 or 4/8.** **A higher CR offers more protection, but increases time on air.** Radios with different CR (and same CF, SF and BW), can still communicate with each other if they use an explicit header, as the CR of the payload is stored in the header of the packet, which is always encoded at CR 4/8.[[2]]

### Signal Bandwidth

* An **increase in signal bandwidth permits the use of a higher effective data rate, thus reducing transmission time at the expense of reduced sensitivity improvement.** There are of course regulatory constraints in most countries on the permissible occupied bandwidth. Contrary to the FSK modem which is described in terms of the single sideband bandwidth, the LoRa modem bandwidth refers to the double sideband bandwidth (or total channel bandwidth). The range of bandwidths relevant to most regulatory situations is given in the LoRa modem specifications table (see Section 2.4.5).[[1]]

![alt-text][Bandwidth Table]

* BW is the width of frequencies in the transmission band. **Higher BW gives a higher data rate (thus shorter time on air), but a lower sensitivity (because of integration of additional noise).** **A lower BW gives a higher sensitivity, but a lower data rate.** Lower BW also requires more accurate crystals (less ppm). Data is send out at a chip rate equal to the bandwidth; a bandwidth of 125 kHz corresponds to a chip rate of 125 kcps. Although the bandwidth can be selected in a range of **7.8 kHz to 500 kHz, a typical LoRa network operates at either 500 kHz, 250 kHz or 125 kHz (resp.BW500, BW250 and BW125).**[[2]]


### Carrier Frequency

* CF is the centre frequency that can be programmed in steps of 61 Hz between 137MHz to 1020 MHz. Depending on the particular LoRa chip, this range may be limited to 860MHz to 1020 MHz. [[2]]

### Transmission Power

* TP on a LoRa radio **can be adjusted from −4 dBm to 20 dBm, in 1 dB steps,** but because of hardware implementation limits, the range is often limited to 2 dBm to 20 dBm. In addition, because of hardware limitations, power levels higher than 17 dBm can only be used on a 1 duty cycle. [[2]]
* Transmission power can be any of +5 to +23 (for modules that use **PA_BOOST**)[[3]]

## Relation to one another
### LoRa Symbol Rate

With a knowledge of the key parameters that can be controlled by the user we define the LoRa symbol rate as:

![alt-text][LoRa Symbol Rate]

where BW is the programmed bandwidth and SF is the spreading factor. The transmitted signal is a constant envelope signal. Equivalently, one chip is sent per second per Hz of bandwidth.

## Time-on-air

For a given combination of spreading factor (SF), coding rate (CR) and signal bandwidth (BW) the total on-the-air transmission time of a LoRa packet can be calculated as follows. From the definition of the symbol rate it is convenient to define the symbol rate:


![alt-text][Time on air 1]

The LoRa packet duration is the sum of the duration of the preamble and the transmitted packet. The preamble length is calculated as follows:

![alt-text][Time on air 2]

where n_preamble is the programmed preamble length, PreambleLength.The payload duration depends upon the header mode that is enabled. The following formulae give the payload duration in implicit (headerless) and explicit (with header) modes.

![alt-text][Time on air 3]

Addition of these two durations gives the total packet on-air time.

![alt-text][Time on air 4]

# RFM95 Features

## Channel Activity Detection (CAD)
### Principal Operation
"The channel activity detection mode is designed to detect a LoRa preamble on the radio channel with the best possible power efficiency. Once in CAD mode, the RFM95/96/97/98(W)will perform a very quick scan of the band to detect a LoRa packet preamble."

### Duration, Power consumption, and Example

The time taken for the channel activity detection is dependent upon the LoRa modulation settings used. For a given configuration the typical CAD detection time is shown in the graph below, expressed as a multiple of the LoRa symbol period. Of this period the radio is in receiver mode for (2^SF + 32) / BW seconds. For the remainder of the CAD cycle the radio is in a reduced consumption state.

![alt-text][CAD as a Function of SF]

To illustrate this process and the respective consumption in each mode, the CAD process follows the sequence of events outlined below:

![alt-text][Consumption Profile of the LoRa CAD profile]

The receiver is then in full receiver mode for just over half of the activity detection, followed by a reduced consumption processing phase where the consumption varies with the LoRa bandwidth as shown in the table below.

![alt-text][LoRa CAD Consumption Figure]


**Example:**

Let SF = 7 (128 chips/symbol), BW = 125kHz  
Symbol rate = BW / 2^SF = 125000 / 2^7 = 976.56 symbols/second

With SF = 7, CAD runs for a total time of approx. 1.92 Symbols (according to the chart)

Total CAD time = (1/976.56) * 1.92 = 1.966 milliseconds

Of that time, Receiver mode is running for (2^SF + 32) / BW = (2^7 + 32) / 125000 = 1.28 milliseconds


Remainder of the CAD process = 1.966 - 1.28 = 0.686 milliseconds

At 125 kHz, power consumed during receiver mode is 10.8mA and power for the remainder of CAD is 5.6 mA.

Average current consumption is ((10.8mA * 1.28e-3) + (5.6mA * 0.686e-3)) / (1.28e-3 + 0.686e-3) = 8.98 mA

**Therefore, CAD runs for a total time of 1.966 milliseconds drawing an average of 8.98mA.**


## Other Neat Features (only in FSK/OOK Mode)
* Low Battery Detector
	* A low battery detector is also included allowing the generation of an interrupt signal in response to the supply voltage dropping below a programmable threshold that is adjustable through the register RegLowBat. The interrupt signal can be mapped to any of the DIO pins by programming RegDioMapping.

* Beacon Mode (only in FSK/OOK Mode)
	* In some short range wireless network topologies a repetitive message, also known as beacon, is transmitted periodically by a transmitter. The Beacon Tx mode allows for the re-transmission of the same packet without having to fill the FIFO multiple times with the same data.

* Built-in Temperature Sensor
	* A stand alone temperature measurement block is used in order to measure the temperature in any mode except Sleep and Standby. 
	* Due to process variations, the absolute accuracy of the result is +/- 10 °C. Higher precision requires a calibration procedure at a known temperature. The figure below shows the influence of just such a calibration process.


# [RadioHead Library][RadioHead Library]
* Caution: Developing this type of software and using data radios successfully is challenging and requires a substantial knowledge base in software and radio and data transmission technologies and theory. It may not be an appropriate project for beginners. If you are a beginner, you will need to spend some time gaining knowledge in these areas first.

** Check out this site for buying a [Helical Antenna][Helical Antenna Site]**

## RadioHead MESH Network

See [this site][RFM95 MESH Netwrok Example] for an excellent (albeit complicated) example on a MESH network setup. This example has each node display it's routing table. Could be useful.

* **Recall the RFM95 Modules DO NOT go to sleep when powered via USB**

# Additional Notes
## Acronyms 
* OOK - On-Off Keying Modulation
* (G)FSK - (Gaussian) Frequency-Shift Keying modulation
* ASK - Amplitude-shift keying modulation
* (G)MSK - (Gaussian) Minimum-shift keying modulation
* FHSS - Frequency Hopping Spread Spectrum
* PLL - Phase-locked Loop Oscillator

* AGC - Automatic Gain Control
* AFC - automatic frequency correction
* RSSI - received signal strength indicator
* TLS - top level sequencer

* LO - Local Oscillator 
* LNA - Low-noise Amplifier

## Receiver Enabled and Receiver Active States
In the receiver operating mode two states of functionality are defined. Upon initial transition to receiver operating mode the receiver is in the ‘receiver-enabled’ state. In this state the receiver awaits for either the **user defined valid preamble or RSSI detection** criterion to be fulfilled. Once met the receiver enters ‘receiver-active’ state. In this second state the received signal is processed by the packet engine and top level sequencer. For a complete description of the digital functions of the RFM95/96/97/98(W) receiver please see Section 4 of the datasheet.

# Additional Resources

* [Ultra low power listening mode for battery nodes][Site 3]
* [Small loop antennas @ 433 Mhz][Site 4]
	* Check out later
* [Build LoRa node using Arduino Uno and HopeRF RFM95 LoRa transceiver module][Site 5]
* [Build a copper 868MHz coil antenna][Site 6]



## RFMx series (Differentiating between the different modules)

* RFM95 & RFM96 - Only difference is the frequency they are attuned for (The hardware related to the frequency setting)
	* RFM96 is for 433MHz
	* RFM95 is for 868/915 MHz
	* Both have RF96 on the black chip - ignore it

* RFM69 do not support LoRa. They only support OOK & FSK Modes. RFM95 supports LoRa

![alt-text][Transceiver Compatibility Chart]

[source][4]



[Site 1]: https://lowpowerlab.com/forum/low-power-techniques/any-success-with-lora-low-power-listening/
[Site 2]: https://lowpowerlab.com/forum/low-power-techniques/using-listenmode-as-wakeup-timer/15/
[Site 3]: https://lowpowerlab.com/forum/low-power-techniques/ultra-low-power-listening-mode-for-battery-nodes/
[Site 4]: https://lowpowerlab.com/forum/rf-range-antennas-rfm69-library/small-loop-antennas-433-mhz/

[Site 5]: https://www.mobilefish.com/developer/lorawan/lorawan_quickguide_build_lora_node_rfm95_arduino_uno.html

[Site 6]: https://www.mobilefish.com/developer/lorawan/lorawan_quickguide_build_868mhz_coil_antenna.html

[Helical Antenna Site]: https://fr.aliexpress.com/wholesale?SearchText=sw868-th13&switch_new_app=y

[RadioHead Library]: https://www.airspayce.com/mikem/arduino/RadioHead/index.html

[Spreading Factor Table]: https://i.ibb.co/0tKX4pk/Spreading-Factor-Table.png
[Coding Rate Table]: https://i.ibb.co/R4w0sbt/Coding-Rate-Table.png
[Bandwidth Table]: https://i.ibb.co/kgBt5qh/Bandwidth-Table.png
[LoRa Symbol Rate]: https://i.ibb.co/zRzPv2j/Lo-Ra-Symbol-Rate.png
[Receiver Startup Time]: https://i.ibb.co/6BGvtqy/Receiver-Startup-Time.png

[Transceiver Compatibility Chart]: https://i.ibb.co/cF91MJZ/Transceiver-Compatibility-Chart.png

[Time on air 1]: https://i.ibb.co/TTP9VyK/Time-on-air-1.png
[Time on air 2]: https://i.ibb.co/M9PMMGf/Time-on-air-2.png
[Time on air 3]: https://i.ibb.co/jHSM57Q/Time-on-air-3.png
[Time on air 4]: https://i.ibb.co/prbXcJm/Time-on-air-4.png

[1]: https://cdn.sparkfun.com/assets/learn_tutorials/8/0/4/RFM95_96_97_98W.pdf
[2]: https://eprints.lancs.ac.uk/id/eprint/85515/4/lora_tps_r1342.pdf
[3]: https://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html#a6f4fef2a1f40e704055bff09799f08cf
[4]: https://lowpowerlab.com/guide/moteino/transceivers/

[RFM95 MESH Netwrok Example]: https://nootropicdesign.com/projectlab/2018/10/20/lora-mesh-networking/


[CAD as a Function of SF]:https://i.ibb.co/J5spnY6/CAD-as-a-Function-of-SF.png
[Consumption Profile of the LoRa CAD profile]: https://i.ibb.co/3fd2R2v/Consumption-Profile-of-the-Lo-Ra-CAD-profile.png 
[LoRa CAD Consumption Figure]: https://i.ibb.co/FsSK3RP/Lo-Ra-CAD-Consumption-Figure.png