OverView and Plan:

* Overall Goal - Reduce how long the GPS's are on to minimum current drawn and maximize battery life
* How to acheive this - Provide the GPS was data that will aid in aquring a quicker fix.
* How is this data provided



* GPS must initially run for approx. 12.5min to aquire all Almanac data. The more the better. Acquiring it from all satellites would be ideal

* Ideally, want GPS to Hot Start. 

* **Many of these Assisting features are extremely challenging to test. It's very difficult to determine not only if the aids are being used, but if they've made any noticable difference. Their effectiveness also fluctuates with the position of the satellites**


* The best option may be to configure everything via U-center, and stick to recieving NMEA messages via Arduino. You can't really monitor when or if Assist Autonomous has done anything. Same goes for downloading Ephemeris and Alamanc data. Maybe every so often, let the GPS run a bit longer after getting a fix to get some ephemeris data (but that may just be a waste of time since it won't be valid for very long)
    * Can save the confiuration options and quickly load it onto the GPS via U-center. Could probs do via Arduino (just load and send the text file)

* **Could have the Gateway transmit the ephemeris data to the Turtle Trackers to minimize the time the trackers remain on**
* **Use the Gateways as a Base Station**

    * If the Trackers and Gateways are in sync, can have the Gateways braodcast relevant data for the Trackers to use






* I think Assist Autonomous starts running after it has downloaded the ephemeris data from a satellite. Look at how long it takes to download the data to get an idea of total time to completion

* Let the other GPS module run for a while and record the log. Do that again tomorrow and comapre the data (and see if the data even saved overnight)
* Both the original and the NEO-6M GPS's **DID NOT** save the data (at least not long enough to be useful). After 16 hours, the set configuration and almanac data was gone)
    * 1 hour intervals may be okay


* **100% confirmed - The Original GPS's do not have memory to store the config. or other data**

* Time underwater is propotional to the additional amount of time the logger can spend to get a fix
    * If the Tracker is underwater, it won't turn on to get a fix. FOr everytime this occurs, the Tracker is allowed to spend that much longer getting it fix once it surfaces without taking a loss on battery life
    * Have a counter. If Tracker is does not acquire a fix after x coiunts (each count representing  time it should have gotten a fix but didn't because it was underwater), increase the time the Tracker stays on when it can get a fix

* Use noise/jammer/combo of 2 to check the following
    * Underwater and won't get a fix
    * Above water but very cloudy - fix unlikley
    * Above water and clear skies - easy fix

Hardware Configuration:
===

* Configuration data, Almanac data, ephemeris data, and Assist Autonomous data will be saved to the BBR (Battery Backed RAM)
*  GPS's most recent position, time, etc. are automatically saved to the BBR

* The battery is **suppose to** automatically recharges when the unit is powered and has a lifespan of approx. 2 weeks (the Arduino will still peridocially be waking up so the battery should always have a charge)
    * May need an RTC for this data to be useful


* Using the SPI pins, the GPS can utilise the Arduinos EEPROM. This can be used to store Assist Offline Data and the GPS's most recent position
    * Using this data requires a **RTC**
    * It will garuentee a fast TTFF for **at least** the first 14 days of operation
    * "u-blox provides correction data valid from **1 to 35 days**. The size of these files increases with the length of the prediction period, from as little as **3 kB to 125 kB**. Positioning accuracy decreases with the length of the correction data duration, with 1–3 day data providing relatively high accuracy and 10–35 day data progressively less accuracy."




Software Configuration:
===


Algorithm:
---
* Needs to know:
    * When to aquire new ephemeris data
    * When ephemeris data has expired
    * When Jamming is occuring
    * When to perform Autonomous calculations
    * How much time to provide for getting a fix, performing calculations, and aquireing new data
    * When it is safe to power the GPS off
 
* Combo of Downloaded data and AssitAutonomous (in case memory is an issue)
    * Download data 3 days apart. Use AssistAutonomous to determine data within those time blocks


* how long does it take to get Autonomous data? Can computations be done while the GPS is in a Low Power Mode? What requirements must be met before Autonomous can do its calculations?

* How does the GPS know when the saved data is no longer valid? (it would need a clock)
    * Note that the receiver requires an **approximate value of the absolute time** to calculate AssistNow Autonomous orbits. For best performance it is, therefore, recommended to supply this information to the receiver using the UBX-AID-INI message in a scenario without a running RTC (i.e. without backup battery).

Concluding Thoughts:
===







[1]: https://www.measurementsystems.co.uk/docs/TTFFstartup.pdf