# Active and Passive Antennas

**Active antennas**: are any antennas with integrated signal amplifiers built right into the unit [[1]]
**Passive antennas**: are antennas that have no amplification stages [[1]]

* An active antenna is a passive antenna that simply includes an onboard amplifier. There is no difference between the antenna element of an active or passive antenna of the same type; the only difference is whether an amplifier is included. An active antenna provides gain to help overcome coax cable losses and at the same time provides the proper signal level to the GNSS receiver. The drawback is that an active antenna requires an **external power source**. [[1]] [[2]]
* Only practical reason to use an active antenna is to **compensate for cable loss in receive applications** [[1]]
    * All RF signal loses strength (attenuates) as it passes through coaxial cable. The longer the cable run, the larger the loss

* UHF cable runs of less than 50 feet do not require active antennas or in-line amplification to compensate for loss [[1]]

# General Notes
* Most GPS antennas are designed with a characteristic impedance of 50 ohms
* If one places an antenna inside an enclosure, its impedance and resonant frequency may change. 
    * A microstrip patch antenna placed in a plastic enclosure, for example, can have its resonant frequency shifted downward by several megahertz, depending on the thickenss of the plastic and its dielectric constant
    * Antenna manufacturers therefore, purposely design thei patch antennas to resonate at a higher frequency than the actual operating value

# Antenna Types

* Coaxial, Helical, dipole, etc
* monpole, dipole, quadrifilar helices (volutes), sprial helices, slots, microstrips
* ceramic patch antenn

# GPS Antenna

# LoRa Antenna

# Other Notes

* Design and optimize the antenna 
    * Antenna shape, size, length
        * https://github.com/OpenHR20/OpenHR20/wiki/2.1)--433-MHz-and-868-MHz--Antenna-Design-Examples
        * http://e.pavlin.si/2018/06/29/small-normal-mode-antenna-for-868mhz/
    * Antenna connection
        * In regard to connecting to the LoRa MESH Module, I believe the connector can be desoldered from the board, and an antenna can simply be soldered on. Using a coaxial wire as the antenna, there shouldn’t be an issue with attaching it to the modules since, despite the extra hardware used to connect the original antenna, all it does in the end is connect the coaxial wire to the board.
        * https://en.wikipedia.org/wiki/SMA_connector
            * Maybe not quite so straight forward. Recall the necessary 50ohm resistance.
        * https://www.google.com/search?q=remove+SMA+connector+and+solder+antenna+directly&oq=remove+SMA+connector+and+solder+antenna+directly&aqs=chrome..69i57.16186j0j7&sourceid=chrome&ie=UTF-8#kpvalbx=1
            * Video on removing the connector


[1]: https://www.rfvenue.com/blog/2014/12/15/active-v-passive-anntennas
[2]: https://techship.com/faq/difference-active-and-passive-antenna/
[3]: http://www2.unb.ca/gge/Resources/gpsworld.july98.pdf