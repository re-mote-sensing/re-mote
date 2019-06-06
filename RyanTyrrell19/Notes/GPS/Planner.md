# Future Plans

* Look at which ephemeris data has been downloaed for each sateliites
* Use Watchdog timer to sleep during specific time - get GPS time when available to self-correct
* Look through UBX protocol to see what data can be read from GPS
* Use the LoRa Module to check if Tracker is underwater. If no acknowledgment is recieved, GPS is underwater
    * Issues - Gateway breaks at some point, Gateway is out of range
    * Pros - Much cheaper on power
* Use either LoRa or GPs, check the noise level


* Look at different GPS modules 
* Watchdog accuracy test
* RTC on GPS?
* Read RTC without fix (and if it's reliable)
* antennas (what would be good for our application)
* Bounce back to LoRa every so often

* Send/recieve messages using UBX via the Arduino


* Lora test
    * Go to Cootes
    * Test range & record signal strength
    * Specify settings, location (GPS coordiantes)
        * 
    * Results when LoRa is in differnt elvations/locations
    * Vary settings of LoRa/ location settings (line-of-site/Field/hills/trees)
    * Record other relevant data ()
    * antenna (with good/ okay/ no antenna)
    * vary distance (use google maps)


# Meeting Outline & Notes
1) Go over GPS Notes again, going over the sections that have been added and given in more detail
    * How to guage the GPS accuracy (PDOP)
    * Improving GPS Accuracy (DGNSS, SBAS)
    * Almanac Data, Ephemeris data, trasnmission of this data
    * Start-up modes and their requirments
2) Show U-Center Notes
3) Testing Notes
    * Power Saving Mode doesn't seem to be appropriate for the Turtle Trackers. This mode is geard towards GPS's that will always remaine powered on and require position fixes relativley frequently.  
For our application the idea is to get a fix as quickly as possible, record the position, than cut all power to the GPS. The usefulness of PSM occurs after the GPS has aquired it's fix, and it is at this point that all power is cut to the GPS anyway's.   Therefore, it makes no difference to have the GPS run in Continuous mode or Power Saving Mode.
4) Thoughts and Ideas (below)


# OverView and Plan:

* Overall Goal - Reduce how long the GPS's are ON to minimize current drawn and maximize battery life
* How to acheive this - Provide the GPS with data that will aid in aquiring a quicker fix.
    * Almanac Data - Have GPS run for 12.5 min initially to download all data. Data will be useable for entire duration of project
    * Other data - DYnamically downloaded while deployed
* How is this data provided
    * Data is stored on the GPS's BBR
    * Data is transmitted to the GPS's via the Gateway's
* How long should the GPS run for?
    * Run time should be dynamically chosen based on the following
        * Type of Start (Cold, Warm, Hot)
        * Time since last fix (i.e. time underwater)
        * Potentially - Cloudy/clear conditions?
    * Run time will influence,
        * ephemeric data downloaded
        * Aided data downloaded (Assit Autonomous)





## Additional Ideas:
* **Could have the Gateway transmit the ephemeris data to the Turtle Trackers to minimize the time the trackers remain on**
* **Use the Gateways as a Base Station**


## Extra Notes:
* GPS must initially run for approx. 12.5min to aquire all Almanac data. The more the better. Acquiring it from all satellites would be ideal

* Time underwater is propotional to the additional amount of time the logger can spend to get a fix
    * If the Tracker is underwater, it won't turn on to get a fix. For everytime this occurs, the Tracker is allowed to spend that much longer getting a fix once it surfaces without taking a loss on battery life
    * Have a counter. If Tracker  does not acquire a fix after x counts (each count representing time it should have gotten a fix but didn't because it was underwater), increase the time the Tracker stays on when it can get a fix

* **Many of these Assisting features are extremely challenging to test. It's very difficult to determine not only if the aids are being used, but if they've made any noticable difference. Their effectiveness also fluctuates with the position of the satellites**
    * If the Trackers and Gateways are in sync, can have the Gateways braodcast relevant data for the Trackers to use

* I think Assist Autonomous starts running after it has downloaded the ephemeris data from a satellite. Look at how long it takes to download the data to get an idea of total time to completion

* **Use of an RTC will be useful for the following reasons,**
    * Provides accurate time to the GPS
    * Allow us ti implement a secific time interval for the GPS Trackers, which can increase the battery life

## Potential Plan A:
* The original GPS's data not stored data for future use, resulting in them always running a Cold Start. The addition of a BBR is the major change to the GPS's (minor changes include altering the configuration sttings via U-center)
* In scenario's that the turtles is out of the water, and the GPS is capable of getting a fix every hour, with the aid of the BBR, a fix can be obtained in < 30sec.
    * If time is givene to allow ephemeric data to be downloaded, this can be reduced to approx. 1sec (problem is knowing how long to leave the GPS on for to download this data)

* Have a minimum time that the GPS will ALWAYS stay on for.
    * Around 30sec should be good. 
        * If it already has ephemeric data, it will get a fix quickly, and provide enough time to download more data
            * Time can be cut down for this scenario
        * If it is starting from a Warm Start, 30sec should be good for a fix only
        * From a cold start, use the underwater counter to add additional time accordingly

## Potential Plan B
* The best option may be to configure everything via U-center, and stick to recieving NMEA messages via Arduino. You can't really monitor when or if Assist Autonomous has done anything. Same goes for downloading Ephemeris and Alamanc data. Maybe every so often, let the GPS run a bit longer after getting a fix to get some ephemeris data (but that may just be a waste of time since it won't be valid for very long)
    * Can save the confiuration options and quickly load it onto the GPS via U-center. Could probs do via Arduino (just load and send the text file)


# Other
* Let the other GPS module run for a while and record the log. Do that again tomorrow and comapre the data (and see if the data even saved overnight)
* Both the original and the NEO-6M GPS's **DID NOT** save the data (at least not long enough to be useful). After 16 hours, the set configuration and almanac data was gone)
    * 1 hour intervals may be okay

* **100% confirmed - The Original GPS's do not have memory to store the config. or other data**

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

## Questions
* how long does it take to get Autonomous data? Can computations be done while the GPS is in a Low Power Mode? What requirements must be met before Autonomous can do its calculations?

* How does the GPS know when the saved data is no longer valid? (it would need a clock)
    * Note that the receiver requires an **approximate value of the absolute time** to calculate AssistNow Autonomous orbits. For best performance it is, therefore, recommended to supply this information to the receiver using the UBX-AID-INI message in a scenario without a running RTC (i.e. without backup battery).


[1]: https://www.measurementsystems.co.uk/docs/TTFFstartup.pdf