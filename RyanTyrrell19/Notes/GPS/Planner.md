* GPS must initially run for approx. 15min to aquire enough Almanac data. The more the better. Aquiring it from all satellites would be ideal
    * The GPS will also record other

* 

* Ideally, want satelliete to Hot Start. For this to occur, the following conditions must be met: [[1]]
    * It has a valid Almanac.
    * The cuurent location is not w











Hardware Configuration:
===

* Configuration data, Almanac data, ephemeris data, and Assist Autonomous data will be saved to the BBR (Battery Backed RAM)
*  GPS's most recent position, time, etc. are automatically saved to the BBR
    * The bttery automatically recharged when the untit is powerd and has a lifespan of 2 weeks (the Arduino will still peridocially be waking up so the battery should always have a charge)

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

* Original GPS, Neo-6M
* Use Arduino's EEPROM, BBC Memory
* 
* Combo of Downloaded data and AssitAutonomous (in case memory is an issue)
    * Download data 3 days apart. Use AssistAutonomous to determine data within those time blocks


* how long does it take to get Autonomous data? Can computations be done while the GPS is in a Low Power Mode?
* How does the GPS know when the saved data is no longer valid? (it would need a clock)
    * Note that the receiver requires an approximate value of the absolute time to calculate AssistNow Autonomous orbits. For best performance it is, therefore, recommended to supply this information to the receiver using the UBX-AID-INI message in a scenario without a running RTC (i.e. without backup battery).
13.8.4

Option A:
---
* 


Option B:
---


[1]: https://www.measurementsystems.co.uk/docs/TTFFstartup.pdf