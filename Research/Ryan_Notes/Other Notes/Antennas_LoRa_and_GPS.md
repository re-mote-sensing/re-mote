# GPS Antenna
## Active and Passive Antennas

**Active antennas**: are any antennas with integrated signal amplifiers built right into the unit [[1]]  
**Passive antennas**: are antennas that have no amplification stages [[1]]

* An active antenna is a passive antenna that simply includes an onboard amplifier. There is no difference between the antenna element of an active or passive antenna of the same type; the only difference is whether an amplifier is included. An active antenna provides gain to help overcome coax cable losses and at the same time provides the proper signal level to the GNSS receiver. The drawback is that an active antenna requires an **external power source**. [[1]] [[2]]
* Only practical reason to use an active antenna is to **compensate for cable loss in receive applications** [[1]]
    * All RF signal loses strength (attenuates) as it passes through coaxial cable. The longer the cable run, the larger the loss
* UHF cable runs of less than 50 feet do not require active antennas or in-line amplification to compensate for loss [[1]]

## General Notes (GPS)
* Most GPS antennas are designed with a characteristic impedance of 50 ohms
* If one places an antenna inside an enclosure, its impedance and resonant frequency may change. 
    * A microstrip patch antenna placed in a plastic enclosure, for example, can have its resonant frequency shifted downward by several megahertz, depending on the thickenss of the plastic and its dielectric constant
    * Antenna manufacturers therefore, purposely design their patch antennas to resonate at a higher frequency than the actual operating value
* Orientation matters


# LoRa Antenna
## General Notes
* Need to know frequency of LoRa so wavelength can be computed and the length of the antenna can match it
* Effective Radiated Power (ERP) limits for your location
* using a piece of wire as an antenna is called a "quarter wavelength monopole"
* Want omnidirectional (radiate signals uniformly in one plane in all directions.)
* An antenna cannot put energy into the system, so it is a passive device
* * **receiver sensitivity**: This is the lowest dB level at which the receiver is able to successfully receive the signal
* Don't own any tools to test the various specs, such as VSWR (to check impedance match, and which frequency the impedance has the best match),
* FCC Certified? 


## Antenna Fundamentals [[4]]
* Wavelength is computed as λ = c/f (c = speed of light, f = frequency) The length of the antenna must be the same as the wavelength, because this is when resonance occurs. Since resonance will occur at whole number fractions (½, 1/3, ¼, etc.) of the fundamental frequency, shorter antennas can be used to send and recover the signal
    * **Reducing the antenna’s size will have some impact on the efficiency and impedance of the antenna, which can affect the final performance of the system.**
* Another way to reduce the size of the antenna is to **coil the element**.
    * This has the advantage of making the **overall length shorter, but it will also reduce the antenna’s bandwidth.**

## Antenna Specifications
### Impedance [[4]]
* The impedance of an antenna is the **real resistance and imaginary reactance that appears at the terminals of the antenna** 
* It will be **affected by objects that are nearby**, such as other antennas, the **components on a circuit board**, and even **the user of the device**
* **maximum power transfer will occur when the source and load impedance match**
	* If they are different, called a mismatch, then some of the **power sent to the antenna will be reflected back into the load or lost as heat**. This **will lower the efficiency of the system, lowering range, increasing power requirements, and shortening battery life.**
* Industry convention for RF is an impedance of 50-ohms.

### Voltage Standing Wave Ratio (VSWR) [[4]]
* **VSWR** is a measurement of how well an antenna is matched to a source impedance, typically 50-ohms
* It is calculated by measuring the voltage wave that is headed toward the load versus the voltage wave that is reflected back from the load
* A perfect match will have a VSWR of 1:1.
	* The higher the first number, the worse the match and the more inefficient the system. Since a perfect match cannot ever be obtained, some benchmark for performance needs to be set. (Usually 2:1)
	* As a side note, since the “:1” is always implied, many data sheets will remove it and just display the first number.

![alt-text][VSWR Graph]

* VSWR is usually displayed graphically versus frequency
* The **lowest point on the graph is the antenna’s center frequency**. The VSWR at that point denotes how close to 50-ohms the antenna gets. The **space between the points where the graph crosses the specified VSWR typically defines the antenna’s bandwidth**

### Radiation Pattern & isotropic antenna [[4]]
* True antenna performance can only be determined by measuring the amount of energy that the antenna radiates into free space
* When the radiated power is measured around the antenna, a shape emerges called the **radiation pattern.**
	* This is the most direct measurement of an antenna’s actual performance.
 * For a dipole antenna, the pattern looks like a doughnut. For a monopole antenna on a ground plane, cut that doughnut in half along the edge and set it on the plane with the antenna sticking up through the middle.

![alt-text][Radiation Patterns]

* After the radiated energy surrounding an antenna is measured, the data is often turned into a radiation pattern plot. This plot **graphically presents the way in which the radio frequency energy is distributed or directed by the antenna into free space.**
* An antenna radiation pattern plot is an important tool that allows rapid visual assessment and comparison of antennas. 
	* The antenna’s radiated performance, and thus the corresponding plot, will be **influenced by the test jig or product on which the antenna is mounted**
* An antenna’s radiation pattern and specifications related to it often need a point of comparison or reference. 
* Most frequently, a theoretical antenna called an **isotropic antenna** or isotropic radiator is used for this purpose. Isotropic describes an antenna which radiates electromagnetic energy the same in all directions.
* The isotropic antenna and its perfect spherical pattern are only theoretical and do
not actually exist

![alt-text][isotropic]

### Efficiency, Directivity and Gain [[4]]
* **Efficiency** is a _measurement of how much energy put into the antenna actually gets radiated into free space rather than lost as heat on the antenna’s structure or reflected back into the source._
	* The antenna’s **impedance and VSWR at the center frequency play a big role in this measurement.**
* **Directivity** _measures how much greater an antenna’s peak radiated power density is in a particular direction than for a reference radiator with the same source power_  
	* It is the **ratio of the power density in the pattern maximum to the average power density at a uniform distance from the antenna.**  
		* In short, it is a comparison of the shape of the radiation pattern of the antenna under test to a reference radiation pattern (e.g. the perfect spherical pattern of the isotropic model)
	* The units of this measurement are _decibels relative to isotropic, or dBi._
	* **A dipole has a gain of 2.15dB over isotropic or dBi = dBd + 2.15dB. When comparing gains, it is important to note whether the gain is being expressed in dBd or dBi and convert appropriately.**
* **Gain**, not to be confused with that of an amplifier, _is the directivity of the antenna reduced by the losses on the antenna, such as dielectric, resistance and VSWR._
	* it is the **product of directivity and efficiency**
	* **Gain is the most direct measurement of an antenna’s real performance. As such, it is one of the most important specifications**

![alt-text][Directivity and Gain]

* A simple way to understand directivity or gain is to think of a focusable light source.
	* Assume the light output is constant and focused over a wide area. If the light is refocused to a spot, it appears brighter because all of the light energy is concentrated into a small area. Even though the overall light output has remained constant, the concentrated source will produce an increase in lux at the focus point compared to the wide source
	* In the same way, an antenna that focuses RF energy into a narrow beam can be said to have higher directivity (at the point of focus) than an antenna that radiates equally in all directions. 
		* In other words, **the higher an antenna’s directivity, the narrower the antenna’s pattern and the better its point performance will be.**

![alt-text][Flashlight]

### Summary [[5]]
* If only a certain portion of the electrical power received from the transmitter is actually radiated by the antenna (i.e. less than 100% efficiency), then the directive gain compares the power radiated in a given direction to that reduced power (instead of the total power received), ignoring the inefficiency. 
	* The directivity is therefore the maximum directive gain when taken over all directions, and is always at least 1. 
* On the other hand, the power gain takes into account the poorer efficiency by comparing the radiated power in a given direction to the actual power that the antenna receives from the transmitter, which makes it a more useful figure of merit for the antenna's contribution to the ability of a transmitter in sending a radio wave toward a receiver. 
	* In every direction, the power gain of an isotropic antenna is equal to the efficiency, and hence is always at most 1, though it can and ideally should exceed 1 for a directional antenna

# Antenna Types & Tests
* The go-to DIY antenna used when space constraints exist is the Helical Antenna.
	* [DIY Helical 1][DIY Helical 1]  
	* [DIY Helical 2][DIY Helical 2]
* The following sites discuss test results for variosu antenna types
	* [Site 1][Antenna Test 1]  
	* [Site 2][Antenna Test 2]

## Final Thoughts
* Too difficult to test/verify many of the characteristics of the antenna (radiation pattern, impedance, gain). Requires equipment we don't have
* Best bet is to just reproduce an antenna that others have developed & tested and have been used in applications similar to ours
	* Only thing we're in control of is the wire length (to match the frequency we're using)
* Fairly certain the SMA connector can be removed and a wire antenna soldered on. It looks like this is what was done for one of the DIY Helical antenna Examples

# Testing
## Testing Environment
![alt-text][Testing Environment] [[6]]

* For tests that yield usable data, you should be testing your network at several locations. In addition, the the following metadata should be recorded, [[6]]
	* You should be testing your network at several locations
	* Location (lat, lon)
	* Height (above/below ground)
	* Surrounding (building inside/outside, forest, street etc.)
	* Weather conditions


## Test Values
**RSSI**: indication of the radio power level being received by the device [[6]]
*  In general you can say, the higher the RSSI number, the stronger the signal
	* But the number does not linearly correspond to the distance between transmitter and receiver. 
	* RSSI won’t tell you much about the bigger picture, especially if you only measure some spots and for a short period of time. 
	* In addition, the signal strength (in dBm) is subject to a logarithmic loss dependent on the distance.
* The lower you get (less than -100), the less linear correlation you will have between the distance (in free space) and the RSSI value. The lower the value, the less valuable information you get out of the RSSI value alone.

 
## Test Procedures

[DIY Test using RSSI values][DIY RSSI Test]  
* A simple test to compare the performance of different antennas using the RSSI value

[DIY LoRa Range Test][DIY LoRa Range Test]
* Travel the area with a LoRa device that transmits GPS coordiantes. In addition, track where you've travelled using GPS. Compare what GPS data was recieved with the GPS data that tracked where you;ve travelled to see what was recieved and what was not






## Antenna Types (GPS)

* Coaxial, Helical, dipole, etc
* monpole, dipole, quadrifilar helices (volutes), sprial helices, slots, microstrips
* ceramic patch antenna

# Other Notes

    * Antenna connection
        * In regard to connecting to the LoRa MESH Module, I believe the connector can be desoldered from the board, and an antenna can simply be soldered on. Using a coaxial wire as the antenna, there shouldn’t be an issue with attaching it to the modules since, despite the extra hardware used to connect the original antenna, all it does in the end is connect the coaxial wire to the board.
        * https://en.wikipedia.org/wiki/SMA_connector
            * Maybe not quite so straight forward. Recall the necessary 50ohm resistance.
        * https://www.google.com/search?q=remove+SMA+connector+and+solder+antenna+directly&oq=remove+SMA+connector+and+solder+antenna+directly&aqs=chrome..69i57.16186j0j7&sourceid=chrome&ie=UTF-8#kpvalbx=1
            * Video on removing the connector


[1]: https://www.rfvenue.com/blog/2014/12/15/active-v-passive-anntennas
[2]: https://techship.com/faq/difference-active-and-passive-antenna/
[3]: http://www2.unb.ca/gge/Resources/gpsworld.july98.pdf
[4]: https://www.linxtechnologies.com/wp/wp-content/uploads/an-00501.pdf
[5]: https://en.wikipedia.org/wiki/Antenna_gain
[6]: https://lorawantester.com/2019/01/09/5-simple-rules-to-guarantee-a-successful-lorawan-coverage-test/

[DIY Helical 1]: https://github.com/OpenHR20/OpenHR20/wiki
[DIY Helical 2]: http://e.pavlin.si/2018/06/29/small-normal-mode-antenna-for-868mhz/

[DIY RSSI Test]: http://www.ph2lb.nl/blog/index.php?page=lora-measuring-antennas
[DIY LoRa Range Test]: https://reibot.org/2017/04/23/lora-range-test/

[Antenna Test 1]: https://www.coredump.ch/2017/04/30/lorawan-868mhz-antenna-test-part-2/
[Antenna Test 2]: https://medium.com/home-wireless/testing-lora-antennas-at-915mhz-6d6b41ac8f1d

[VSWR Graph]: https://i.ibb.co/pXx3BPV/1.png
[Directivity and Gain]: https://i.ibb.co/mH7xMhh/2.png
[Flashlight]: https://i.ibb.co/mqGQbyN/3.png
[Radiation Patterns]: https://i.ibb.co/rsFg3w0/4.png
[isotropic]: https://i.ibb.co/nRF8TKr/5.png
[Testing Environment]: https://i.ibb.co/VVbH171/6.png