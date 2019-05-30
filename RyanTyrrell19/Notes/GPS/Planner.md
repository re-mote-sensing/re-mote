* GPS must initially run for approx. 15min to aquire enough Almanac data. The more the better. Aquiring it from all satellites would be ideal
    * The GPS will also record other


* Ideally, want satelliete to Hot Start. For this to occur, the following conditions must be met: [[1]]
    * It has a valid Almanac.
    * The cuurent location is not w


* The best option may be to configure everything via U-center, and stick to recieving NMEA messages via Arduino. You can't really monitor when or if Assist Autonomous has done anything. Same goes for downloading Ephemeris and Alamanc data. Maybe every so often, let the GPS run a bit longer after getting a fix to get some ephemeris data (but that may just be a waste of time since it won't be valid for very long)

* I think Assist Autonomous starts running after it has downloaded the ephemeris data from a satellite. Look at how long it takes to download the data to get an idea of total time to completion

* Could have the Gateway transmit the ephemeris data to the Turtle Trackers to minimize the time the trackers remain on

* Let the Other GPS module run for awhile and record the log. DO that again tomorrow and comapre the data (and see if the data even saved overnight)




Hardware Configuration:
===

* Configuration data, Almanac data, ephemeris data, and Assist Autonomous data will be saved to the BBR (Battery Backed RAM)
*  GPS's most recent position, time, etc. are automatically saved to the BBR
    * The battery automatically recharges when the unit is powered and has a lifespan of approx. 2 weeks (the Arduino will still peridocially be waking up so the battery should always have a charge)
    * May need an RTC for this data to be useful



* Using the SPI pins, the GPS can utilise the Arduinos EEPROM. This can be used to store Assist Offline Data and the GPS's most recent position
    * Using this data requires a **RTC**
    * It will garuentee a fast TTFF for **at least** the first 14 days of operation
    * "u-blox provides correction data valid from **1 to 35 days**. The size of these files increases with the length of the prediction period, from as little as **3 kB to 125 kB**. Positioning accuracy decreases with the length of the correction data duration, with 1–3 day data providing relatively high accuracy and 10–35 day data progressively less accuracy."




Software Configuration:
===

1) Power is provided to the GPS Module. It's configuration data 
2) sdd


Algorithm:
---
* Needs to know:
    * When to aquire new ephemeris data
    * When ephemeris data has expired
    * When Jamming is occuring
    * When to perform Autonomous calculations
    * How much time to provide for getting a fix, performing calculations, and aquireing new data
    * 
s EEPROM, BBC Memory
* 
* Combo of Downloaded data and AssitAutonomous (in case memory is an issue)
    * Download data 3 days apart. Use AssistAutonomous to determine data within those time blocks


* how long does it take to get Autonomous data? Can computations be done while the GPS is in a Low Power Mode? What requirements must be met before Autonomous can do its calculations?

* How does the GPS know when the saved data is no longer valid? (it would need a clock)
    * Note that the receiver requires an **approximate value of the absolute time** to calculate AssistNow Autonomous orbits. For best performance it is, therefore, recommended to supply this information to the receiver using the UBX-AID-INI message in a scenario without a running RTC (i.e. without backup battery).

Option A:
---
* 


Option B:
---


[1]: https://www.measurementsystems.co.uk/docs/TTFFstartup.pdf