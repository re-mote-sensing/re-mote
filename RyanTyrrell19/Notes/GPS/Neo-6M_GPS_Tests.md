# Power Management
## Settings consistent across all tests

| Setting               |   Set As   | Description/Note              |
|:---------------------:|:----------:|---                            |
|    Input Voltage      | 3V3        | The Breakout Board that the GPS chip is attached to contains a MIC5205 ultra-low dropout 3V3 regulator                            |

## Testing Procedure Outline

* Record Current Consumption during various modes that the GPS can be in:
    * Inactive for update and Inactive for search states, Acquisition state, Tracking state and Power Optimized Tracking (POT) state
* Test consumption with and without the antenna (or look up antenna power draw via datasheet)

* There is an option called _reduce peak current_. Through practice, this feature does not seem to do much. There may be a slight decrease in current drawn during the -Acquisition_ phase (1 -2mA). This decrease in power will just lead to a longer TTFF; most likly not worth enabling this feature.

## Power Save Mode
* The receiver is not able to download satellite data (e.g. the ephemeris) while it is working in ON/OFF or cyclic tracking operation.
* **When enabling Power Save Mode, SBAS support can be disabled (UBX-CFG-SBAS) since the receiver will be unable to download any SBAS data in this mode.**

* The duration for each state can be configured

### Cyclic Tracking

* Try with peak current setting on & off (no noticable change)

Aquisition Timout (minAqcTime) - 30 s  
Search Period - 20 s  
onTime (on Time) - 0 s  
update Period - 5 s  
doNotEnterOff - May enable if cannot get fix

**General State Cycle:**  
Aquisition (minAqcTime) -> Inactive for Search (Search Period) -> Aquisition (Gets fix) -> Tracking (onTime) -> POT (update Period) -> Aquisition...  


| Mode                | Average Value (mA) |
|:-------------------:|:------------------:|
| Inactive for search | 10.58              |
| Acquisition         | 51 - 53            |
| Tracking            | 38 - 40            |
|  POT                | 16 - 18            |

* Acquisition holds steady at the above ranges until a fix is obtained. After that, it flucuates between 40 - 53mA
    * I beleive at this point, it is bceause it is downloading satellities data
* After a good fix is obtained, Acquisition holds at 40 -41
    * At this point, I believe the only work being done is updating the posiion fix

### ON/OFF Operation

Aquisition Timout (minAqcTime) - 30 s  
Search Period - 20 s  
onTime (on Time) - 20 s  
update Period - 20 s  
doNotEnterOff - May enable if cannot get fix  

**General State Cycle:**  
Aquisition (minAqcTime) -> Inactive for Search (Search Period) -> Aquisition (Gets fix) -> Tracking (onTime) -> POT and Inactive for update state (Update Period) -> Aquisition...

| Mode                | Average Value (mA) |
|:-------------------:|:------------------:|
| Inactive for search | 10.58              |
| Acquisition         | 51 - 53            |
| Tracking            | 38 - 40            |
| Inactive for update | 10.57              |

* Acquisition holds steady at the above ranges until a fix is obtained. After that, it flucuates between 40 - 53mA

## Concluding Notes:

* Power SAving Mode doesn't seem to be appropriate for the Turtle Trackers. This mode is geard towards GPS's that will always remaine powered on and require position fixes relativley frequently.  
For our application the idea is to get a fix as quickly as possible, record the position, than cut all power to the GPS. The usefulness of PSM occurs after the GPS has aquired it's fix, and it is at this point that all power is cut to the GPS anyway's.   Therefore, it makes no difference to have the GPS run in Continuous mode or Power Saving Mode.

# GPS Accuracy
## Changeable Parameters

* **Many of these Assisting features are extremely challenging to test. It's very difficult to determine not only if the aids are being used, but if they've made any noticable difference. Their effectiveness also fluctuates with the position of the satellites**


| Setting               |   Set As   | Description                   |
|:---------------------:|:----------:|---                            |
|    Platform setting   | Pedestrian | Improve the receiver's interpretation of the measurements  and thus provide a more accurate position output. Setting  the receiver to an unsuitable platform model for the given  application environment results in a loss of receiver  performance and position accuracy.                                                            |
|      Static Mode      |   Enabled  | Allows the navigation algorithms to decrease the noise in  the position output when the velocity is below a pre-defined  ‘Static Hold Threshold’. This reduces the position wander  caused by environmental factors such as multi-path and  improves position accuracy especially in stationary applications.                      |
| NMEA Filtration Flags |     NA     | Will leave Default Settings   |
| GNSS Configuration    | Default Values | Specifies how many channels to be allocated for each satellite system (GPS, SBAS, QZSS, GLONASS). Channels are used to search for satellites. Changing the number of channels available will influence TTFF and power consumption. |
| DGNSS|     Enabled    |  See "GPS Notes"  |                            
| SBAS |     Enabled    | See "GPS Notes"   |
| Assited Data Online/Offline |    TBD   |
| Assisted Data Autonomous |     TBD   |

Aquisition Timout (minAqcTime) - 30 s  
Search Period - 20 s  
onTime (on Time) - 0 s  
update Period - 5 s 
Wait for Timefix - Enabled

GNSS - Enabled  
SBAS - Enables  
Assist Autonomous - Enabled  
Jamming Monitor - Disabled (since it needs to reference a good signal first)  
Jamming Indiactor - Use it  
Update RTC & Ephemeris - Enabled?  

## Additional Pre-Test Notes
* Can use the Hot, Warm, and Cold commands so "one can force the receiver to reset and clear data, in order to see the effects of maintaining/losing such data between restarts."

* Manual entry of Exact GPS coordinates will be logged (found using Google Maps)

* The following data will be logged to determine effectiveness of setup and accuracy of GPS. Each will be logged for a Cold, Warm, and Hot Start. After TTFF, 100 additional indexes will be logged
    * TTFF
    * PDOP, HDOP, VDOP
    * GPS Values
        * Lat, Lon, & Alt (msl) - At TTFF
        * Lat, Lon, & Alt (msl) - Recorded 100 indexes after TTFF
        * Average Lat, Lon, & Alt (msl) - Recorded 100 Indexes after TTFF
    * Num. of satellites used

* The Data outputted by the GPS reciever can be recorded and saved.

## Test Location: Cubicle in Room 201 - ITB  
### Cold Start
### Warm Start:
### Hot Start:


## Test Location: JHE Field  

Exact GPS Coordinates: 

### Cold Start:
| Test #     | TTFF    |   PDOP    |            |
|:----------:|:--------------:| :--------:     |:-------------:     |
| 1          |                |                |                    |
| 2          |                |                |                    |
| 3          |                |                |                    |

### Warm Start:



### Hot Start:





## Test Location: Cootes Paradise  
Exact GPS Coordinates: 

### Cold Start:
### Warm Start:
### Hot Start:

## SBAS & Differential GNSS

* Enable SBAS
* View output to see if connection is established and whether GPS has improved once a connection is established
* See SBAS Section in Specs. for more info

## AssistNow Offline
* u-blox provides AlmanacPlus data files in different sizes, which contain differential almanac corrections that are valid for a period of between 1 and 14 days thereafter.
* Requires an external memory device connected to the GPS chip using SPI. Alternativly, the data can be saved on thr Arduino (or maybe computer for testing) as the 'Host' and sent via the EX/TX pins to the GPS chip


## AssistNow Autonomous
* Prevents Power Save Mode from being entered until calculations are done
* AssistNow Autonomous subsystem will not produce any data and orbits while AssistNow Offline data is available

## Jamming Test
### Jamming/Interference Indicator Test
* Must be calibrated to indicate what a 'not jammed' case would be

### Jamming/Interference Monitor
* Cannot be used in Power Save Mode
* Must aquire a good fix for comparison if jamming occurs
* Must set the Thresholds for what would be classified as 'jamming'
