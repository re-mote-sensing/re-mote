# Relevent U-Center Headings & Other Notes:

## Overview

* Start-up

* AID (GPS Aiding)
    * ALM (Almanac)
    * EPH (Ephemeris)
    * HUI (GPS Health, UTC and Ionosphere Parameter)
    * INI (Initial Data)

* CFG (Config)
    * CFG (configuration)
    * DGNSS (Differential GNSS Configuration)
    * GNSS (GNSS Config)
    * ITFM (Jamming/Interference Monitor)
    * NAV5 (Navigation 5)
    * NAVX5 (Navigation Expert 5)
    * PM (Power Management)
    * PM2 (Extended Power Management)
    * RST (Reset)
    * RXM (Reciever Manager)
    * SBAS (SBAS Settings)
    * USB (Universal Serial Bus):

* MON (Hardware Status)
    * HW (Hardware Status)
    * TXBUF (TX Buffer)

* NAV (Navigation)
    * AOPSTATUS (AssistNow Autonomous Status)
    * CLOCK (Clock Status)
    * DOP (Dilution of Precision)
    * SBAS (SBAS Status)
    * SOL (Navigation Solution)
    * STATUS (Navigation Status)
    * SVINFO (SV Information)

* Extra Notes
    * Power Management
        * Various States
        * Satellite data download
        * Communication
        * Cooperation with the AssistNow Autonomous feature
    * Jamming/Interference
        * Jamming/Interference Indicator
        * Jamming/Interference Monitor

Start-up:
===
* Data For Starup:
    * Ephemeris
        * Data containing the precise location of satellites
    * Almanac
        * General info about satellites
    * Health
        * Health of the satellites (I'm assuming)
    * Klobuchar
        * A model used to reduce the error caused by the ionosphere
    * Position
        * Estimate of the GPS's location
    * Clock Drift
        * Degree by which the GPS recievers clock misaligns with the real time (I think)
    * UTC Paramters
        * Coordinated Universal Time
    * RTC Time
        * Current time or at least an estimate of the current time
    * SFDR Parameters
        * Used with Automotive Dead Reckoning (Not Important)
    * SFDR Vehicle Monitor
        * Used with Automotive Dead Reckoning (Not Important)
    * TCT Parameters
        * ???
    * AssistNow Autonomous
        * Computed data used to extrapolate the ephemeris data

* Hot Start has **all data**
* Warm Start has all data **except Ephemeris**
* Cold Start has **no data**

AID (GPS Aiding)
===
ALM (Almanac)
---
* Display's what Almanac data has been downloaded, from which satetliite, and how many of the _pages_ have been downlaoded (reference to how Almanac data is downloaded and stored)  
![alt-text][1]

EPH (Ephemeris):
---
* Display's what Ephemeris data has been downloaded, from which satetliite, and how many of the _pages_ have been downlaoded (reference to how Ephemeris data is downloaded and stored)  
![alt-text][2]

HUI (GPS Health, UTC and Ionosphere Parameter):
---
* Data relevant to the start-up time of the GPS. This data is automatically updated and saved to the GPS Modules BBR (Battery-Backed RAM)
    * UTC Parameters and Klobuchar are both needed for a Warm Start. I beleive these valeus are obtaiend with the Almanac data
![alt-text][3]

INI (Initial Data):
---
* Data relevant to the start-up time of the GPS. This data is automatically updated and saved to the GPS Modules BBR (Battery-Backed RAM)
    * Position and Time are both needed for a Warm Start. They are automatically saved to the BBR
![alt-text][4]


CFG (Config):
===
* Used to Configure various paramters


CFG (configuration)
---
Sets the current configuration of the GPS reciever. Can set current configuration to be Permanent, restore the default configurations, and choose where the configuration is saved to  
![alt-text][5]

The following diagram illustrates the process of:

![alt-text][Config. Diagram]

DGNSS (Differential GNSS Configuration):
---
* Enable/Disable DGNSS
    * [DGNSS][DGNSS] is a kind of _GNSS Augmentation system_ based on an enhancement to primary GNSS constellation(s) information by the use of a **network of ground-based reference stations** which enable the broadcasting of _differential information_ to the user to improve the accuracy of the position  
![alt-text][6]

GNSS (GNSS Config)
---
* Determines which GNSS configurations to enable, as well as how many channesl to allocate to each
    * The GNSS's of interest are **GPS, SBAS, QZSS, and GLONASS**
    * Channels are used to search for satellites. Changing the number of channels available will influence TTFF and power consumption.  
![alt-text][7]

ITFM (Jamming/Interference Monitor)
---
* Enables/disables the Interference Monitor and sets the thresholds used to determine if Jamming is occuring
    * These thresholds are relative to what a good fix would be so the GPS reciever must at least get one good fix first before it can determine if jamming is occuring for future fixes  
![alt-text][8]


NAV5 (Navigation 5):
---
* Various parameters that are best left as the default values.
* Only _Dynamic Model_ need be changed. This value reflects what sort of movement the GPS reveiver will undergo  
    * The settings improve the receiver's interpretation of the measurements and thus provide a more accurate position output. Setting the receiver to an unsuitable platform model for the given application environment results in a loss of receiver performance and position accuracy.
* **The** _Navigation Input_ **settings are already optimized. Do not change any parameters unless advised by u-blox support engineers.**
* _Navigation Ouput Filters_ configure the **quality of the position fixes**   
![alt-text][9]


NAVX5 (Navigation Expert 5):
---
* Used to enable/disable Auto Autonomous  
![alt-text][10]


PM (Power Management):
---
* _Update Period_: Time between two position fix attempts
* _Search Period_: Time between two acquisition attempts if the receiver is unable to get a position fix
*_Acquisition timeout_: Time after which the receiver stops acquisition and enters _Inactive for search_ state
On-time
    * If the receiver determines that it needs more time for the given starting conditions, it will automatically prolong this time. If set to zero, the acquisition timeout is exclusively determined by the receiver. In case of a very weak or no GPS signal, the timeout determined by the receiver may be shortened in order to save power. However, the acquisition timeout will never be shorter than the configured value.

* _On-time_: Time the receiver remains in Tracking state and produces position fixes
*_Wait for timefix_: Wait for time fix before entering Tracking state

* _Update RTC_: Enables periodic Real Time Clock (RTC) update
* _Update Ephemeris_: Enables periodic ephemeris update
    * The RTC is calibrated every 5 minutes and the ephemeris data is updated approximately every 30 minutes. See chapter Satellite

* _Peak Current Reduction_:  Reduces the peak current during acquisition. A peak current reduction will result in longer start-up times of the receiver
    * This setting is independent of various Power Saving Modes

![alt-text][11]

PM2 (Extended Power Management):
---
* _ON/OFF (long update Period)_:
* _Cyclic Tracking (Short update Period)_:
    * The above modes of operation are more for geard towards scenarios where the GPS reciever is always on.

* _Update Period_: Same as in previous section
* _Search Period_:  Same as in previous section
* _minAcqTime_: Same as Acquisition timeout in previous section
* _On Time_:  Not sure if this differs from the previous on Time. Same name but the two parameters are set to different values...
* _maxStartupStateDur_: **??????**

* _waitTimeFix_: if enabled, the transition from _Acquisition_ to _Tracking_ state is made only if the time is known and within the configured limits, and the receiver is continuously producing position fixes for more than two seconds. Thus enabling the wait for timefix option usually delays the transition from Acquisition to Tracking state by a few seconds.
* doNotEnterOff: If enabled, the receiver acts differently in case it can't get a fix: instead of entering _Inactive for search_ state, it keeps trying to acquire a fix. In other words, the receiver will never be in _Inactive for search_ state and therefore the search period and the acquisition timeout are obsolete.


* _Update RTC_: Same as in previous section
* _Update Ephemeris_: Same as in previous section


![alt-text][12]

RST (Reset):
---
* Provides various reset option. _Hot, Warm, and Cold_ Startup options are of key interest. User Defined allows you to choose what data is available for startup. Used to measure Startup times with varying data provided  
![alt-text][13]

RXM (Reciever Manager):
---
* Used to select the Power Mode
* _Continuous Mode_: GPS is always in _Aquisition_ mode; maximum power consumption
* _Power Save Mode_: Enabling this allows you to utilise the various Power Saving parameters mention previously

![alt-text][14]

SBAS (SBAS Settings):
---
* Enable/disable SBAS (see GPS Notes for more info abour SBAS)
* The u-blox GPS receiver automatically chooses the best SBAS satellite as its primary source.

- As each GEO services a specific region, the correction signal is only useful within that region. Therefore, mission planning is crucial to determine the best possible configuration. The different stages (Testmode vs. Operational) of the various SBAS systems further complicate this task.
* Example 1: SBAS Receiver in North America
    * At the time of writing, the WAAS system is in operational stage, whereas the EGNOS system is still in test mode. Therefore, and especially in the eastern parts of the US, care must be taken in order not to have EGNOS satellites taking preference over WAAS satellites. This can be achieved by disallowing Test Mode use (this inhibits EGNOS satellites from being used as a correction data source), but keeping the PRN Mask to have all SBAS GEOs enabled (which allows EGNOS GEOs to be used for navigation).

* Although u-blox GPS receivers try to select the best available SBAS correction data, it is recommended to disallow the usage of unwanted SBAS satellites by configuration.

![alt-text][15]

USB (Universal Serial Bus):
---
This port can be used for communication purposes and to power the GPS receiver. The USB interface supports two different power modes: 

* _Self Powered Mode_: the receiver is powered by its own power supply. VDDUSB is used to detect the availability of the USB port, i.e. whether the receiver is connected to a USB host.
* _Bus Powered Mode_: the device is powered by the USB bus, therefore no additional power supply is needed. In this mode the default maximum current that can be drawn by the receiver is 100 mA for u-blox 6 (120 mA for u-blox 5). See CFG-USB for a description on how to change this maximum. Configuring Bus Powered Mode indicates that the device will enter a low power state with disabled GPS functionality when the host suspends the device, e.g. when the host is put into stand-by mode.

![alt-text][25]

MON (Hardware Status)
===
* Used to Monitor various paramters

HW (Hardware Status):
---
* Main interest here is the Jamming Status

![alt-text][16]

TXBUF (TX Buffer):
---
* Monitor traffic throug hthe variosu serial ports. Most messages & commands are sent through UART1. If the buffer is full, issues may occur with sending/reciving data to/from the GPS reciever

![alt-text][17]

NAV (Navigation)
===

AOPSTATUS (AssistNow Autonomous Status):
---
* View current status of AssitNow Autonomous (idle, calculating, etc.) and which satellites it has extrapolated the ephemeris data for  
![alt-text][18]

CLOCK (Clock Status):
---
* Contains various clock data
* Of interest because _Clock Drift_ is required for quicker startup

![alt-text][19]

DOP (Dilution of Precision):
---
* Provides info on the various DOP parameters. Useful for determining the accuracy of the GPS fix
![alt-text][20]

SBAS (SBAS Status):
---
* Displays various info regarding which SBAS is in use, among other info

![alt-text][21]

SOL (Navigation Solution):
---
* Provides a list of the more important data recieved from the GPS reciever.

![alt-text][22]

STATUS (Navigation Status):
---
* Provides a list of the more important data recieved from the GPS reciever.

![alt-text][23]

SVINFO (SV Information):
---
* Displays various info regarding which satellites are in use, among other info


![alt-text][24]


# Extra Notes (Put anything highlighed in pink here)

## Power Management

### Various States (Need to find current consumption for each state)

Power Saving Mode is based on a state machine with five different states: _Inactive for update_ and _Inactive for search_ states,
_Acquisition_ state, _Tracking_ state and _Power Optimized Tracking (POT)_ state.

* _Inactive states_: Most parts of the receiver are switched off.
* _Acquisition state_: The receiver actively searches for and acquires signals. **Maximum power consumption.**
* _Tracking state_: The receiver continuously tracks and downloads data. Less power consumption than in
Acquisition state.
* _POT state_: The receiver repeatedly loops through a sequence of tracking (TRK), calculating the position fix
(Calc), and entering an idle period (Idle). No new signals are acquired and no data is downloaded. Much less
power consumption than in Tracking state.  

![alt-text][Power State Machine]

### Satellite data download
* The receiver is not able to download satellite data (e.g. the ephemeris) while it is working in ON/OFF or cyclic tracking operation. Therefore it has to temporarily switch to **continuous operation** for the time the satellites transmit the desired data. To save power the receiver schedules the downloads according to an internal timetable and only switches to continuous operation while data of interest is being transmitted by the SVs. **Each SV transmits its own ephemeris data**. **Ephemeris data download is feasible when the corresponding SV has been tracked with a minimal C/No over a certain period of time.** The download is scheduled in a 30 minute grid or immediately when fewer than a certain number of visible SVs have valid ephemeris data. **Almanac, ionosphere, UTC correction and SV health data are transmitted by all SVs simultaneously. Therefore these parameters can be downloaded when a single SV is tracked with a high enough C/No.**
    * [C/No][C/No] basically is a figure indicate how noisy the incoming signal is

### Communication
*  When PSM is enabled, communication with the receiver (e.g. UBX message to disable PSM) requires particular attention. This is because the receiver may be in Inactive state and therefore unable to receive any message through its interfaces.

#### Cooperation with the AssistNow Autonomous feature
* If both PSM and AssistNow Autonomous features are enabled, the receiver won't enter Inactive for update state as long as AssistNow Autonomous carries out calculations. This prevents losing data from unfinished calculations and, in the end, reduces the total extra power needed for AssistNow Autonomous. The delay before entering Inactive for update state, if any, will be in the range of several seconds, rarely more than 20 seconds.

* Only entering Inactive for update state is affected by AssistNow Autonomous. In other words: in cyclic tracking operation, AssistNow Autonomous will not interfere with the PSM (apart from the increased power consumption).

* Enabling the AssistNow Autonomous feature will lead to increased power consumption while prediction is calculated. The main goal of PSM is to reduce the overall power consumption. Therefore for each application special care must be taken to judge whether AssistNow Autonomous is beneficial to the overall power consumption or not.

### Other
* When enabling Power Save Mode, SBAS support can be disabled (UBX-CFG-SBAS) since the receiver will be unable to download any SBAS data in this mode

## Jamming/Interference
### Jamming/Interference Indicator

* used as an indicator for continuous wave (narrowband) jammers/interference only. The interpretation of the value depends on the application. 
* It is necessary to run the receiver in the application and then calibrate the 'not jammed' case. If the value rises significantly above this threshold, this indicates that a continuous wave jammer is present.

### Jamming/Interference Monitor

* used as an indicator for both broadband and continuous wave (CW) jammers/interference. 
* It is independent of the (CW only) jamming indicator described above
* This monitor reports whether jamming has been detected or suspected by the receiver. The receiver monitors the background noise and looks for significant changes.

The monitor has four states as shown in the following table:

![alt-text][Jamming Monitor]

* The monitor algorithm relies on comparing the currently measured spectrum with a reference from when a good fix was obtained. Thus the monitor will only function when the receiver has had at least one (good) first fix, and will report 'Unknown' before this time.

* **This functionality is not supported in Power Save Mode (PSM).**


[1]: https://i.ibb.co/nBhr5rK/1.png
[2]: https://i.ibb.co/4NtBbW5/2.png
[3]: https://i.ibb.co/JcShNsf/3.png
[4]: https://i.ibb.co/DpHvj3j/4.png

[5]: https://i.ibb.co/wJt7g1m/1.png
[6]: https://i.ibb.co/10RJFtH/2.png
[7]: https://i.ibb.co/88Cqq99/3.png
[8]: https://i.ibb.co/fqzD0qq/4.png
[9]: https://i.ibb.co/JmcL7wM/5.png
[10]: https://i.ibb.co/kB4xLxC/6.png
[11]: https://i.ibb.co/zSgFVr5/7.png
[12]: https://i.ibb.co/JBMNS35/8.png
[13]: https://i.ibb.co/gZ4FCPb/9.png
[14]: https://i.ibb.co/Mp2dzbk/10.png
[15]: https://i.ibb.co/FDQKKBG/11.png

[16]: https://i.ibb.co/R6WYXqy/1.png
[17]: https://i.ibb.co/hR0fY2f/2.png

[18]: https://i.ibb.co/Msq3b5L/1.png
[19]: https://i.ibb.co/2S62QbS/2.png
[20]: https://i.ibb.co/rFN185B/3.png
[21]: https://i.ibb.co/x2HnYpT/4.png
[22]: https://i.ibb.co/r2CR0BF/5.png
[23]: https://i.ibb.co/9ZMtY6L/6.png
[24]: https://i.ibb.co/qxJsZ6L/7.png
[25]: https://i.ibb.co/SK7ywd4/12.png

[DGNSS]: https://gssc.esa.int/navipedia/index.php/Differential_GNSS
[C/No]: https://en.wikipedia.org/wiki/Carrier-to-noise-density_ratio
[Power State Machine]: https://i.ibb.co/hKnHMw5/2.png
[Config. Diagram]: https://i.ibb.co/QrDwQtk/3.png
[Jamming Monitor]: https://i.ibb.co/g7bvwyR/4.png