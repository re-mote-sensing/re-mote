# Power Management
## Settings consistent across all tests

| Setting               |   Set As   | Description/Note              |
|:---------------------:|:----------:|---                            |
|    Input Voltage      | 3V3        | The Breakout Board that the GPS chip is attached to contains a MIC5205 ultra-low dropout 3V3 regulator                            |
|      Peak Current     |   Enabled  | The peak current during acquisition can be reduced. A peak current reduction will result in longer start-up times of the receiver. This setting is independent of the activated mode (Maximum Performance, Eco or Power Save Mode). |

## Testing Procedure Outline

* Record Current Consumption during various stages of GPS aquisition (cold, warm, & hot start)
    * Utilize the start-up buttons in U-Center to do this
* Test consumption with and without the antenna (or look up antenna power draw via datasheet)

## Maximum Performance Mode

During a Cold start, a receiver in Maximum Performance Mode continuously deploys the acquisition engine to search for all satellites. Once the receiver has a position fix (or if pre-positioning information is available), the acquisition engine continues to be used to search for all visible satellites that are not being tracked.

* Only applies to **cold starts**

| Test #     | Min Value (mA) | Max Value (mA) | Average Value (mA) |
|:----------:|:--------------:| :--------:     |:-------------:     |
| 1          |                |                |                    |
| 2          |                |                |                    |
| 3          |                |                |                    |


## Eco Mode

**Stage A (analogous to a Cold Start)**:
* reciever continuously deploys the acquisition engine to search for all satellites (exactly like Maximum Performance Mode)

**Stage B (analogous to a Warm Start)**:
* Once a position can be calculated and a sufficient number of satellites are being tracked, the acquisition engine is powered off resulting in significant power savings. The tracking engine continuously tracks acquired satellites and acquires other available or emerging satellites.
    * Note that even if the acquisition engine is powered off, satellites continue to be acquired.

Stage A:

| Test #     | Min Value (mA) | Max Value (mA) | Average Value (mA) |
|:----------:|:--------------:| :--------:     |:-------------:     |
| 1          |                |                |                    |
| 2          |                |                |                    |
| 3          |                |                |                    |

Stage B:

| Test #     | Min Value (mA) | Max Value (mA) | Average Value (mA) |
|:----------:|:--------------:| :--------:     |:-------------:     |
| 1          |                |                |                    |
| 2          |                |                |                    |
| 3          |                |                |                    |

## Power Save Mode
* The receiver is not able to download satellite data (e.g. the ephemeris) while it is working in ON/OFF or cyclic tracking operation.
* **When enabling Power Save Mode, SBAS support can be disabled (UBX-CFG-SBAS) since the receiver will be unable to download any SBAS data in this mode.**

### Cyclic Tracking

### ON/OFF Operation

### Custom Power Save Mode Configuration?


# GPS Accuracy
## Settings consistent across all tests

| Setting               |   Set As   | Description                   |
|:---------------------:|:----------:|---                            |
|    Platform setting   | Pedestrian | Improve the receiver's interpretation of the measurements  and thus provide a more accurate position output. Setting  the receiver to an unsuitable platform model for the given  application environment results in a loss of receiver  performance and position accuracy.                                                            |
|      Static Mode      |   Enabled  | Allows the navigation algorithms to decrease the noise in  the position output when the velocity is below a pre-defined  ‘Static Hold Threshold’. This reduces the position wander  caused by environmental factors such as multi-path and  improves position accuracy especially in stationary applications.                      |
| NMEA Filtration Flags |     NA     | Will leave Default Settings   |
| GNSS Configuration    | Default Values | Specifies how many channels to be allocated for each satellite system (GPS, SBAS, QZSS, GLONASS). Channels are used to search for satellites. Changing the number of channels available will influence TTFF and power consumption. |

## Additional Pre-Test Notes
* Can use the Hot, Warm, and Cold commands so "one can force the receiver to reset and clear data, in order to see the effects of maintaining/losing such data between restarts."

* Manual entry of Exact GPS coordinates will be logged (found using Google Maps)

* The following data will be logged to determine effectivness of setup and accuracy of GPS. Each will be logged for a Cold, Warm, and Hot Start. After TTFF, 100 additional indexes will be logged
    * TTFF
    * PDOP, HDOP, VDOP
    * GPS Values
        * Lat, Lon, & Alt (msl) - At TTFF
        * Lat, Lon, & Alt (msl) - Recorded 100 indexes after TTFF
        * Average Lat, Lon, & Alt (msl) - Recorded 100 Indexes after TTFF
    * Num. of satellites used


* The GPS reciever configurations for each test setup have been saved
* The Data outputted by the gPS reciever has also been recorded and saved.

## Baseline Test
* All Accuracy Assisting features are disabled to determine a baseline that all other tests will be compared to
    * DGNSS - Disabled
    * SBAS - Disabled
    * Assited Data Offline - Disabled
    * Assisted Data Autonomous - Disabled

Test Location: Cubicle in Room 201 - ITB  

Test Location: JHE Field  
Exact GPS Coordinates: 

| Test #     | Time to Valid Fix (2D or 3D)    |       | Average Value (mA) |
|:----------:|:--------------:| :--------:     |:-------------:     |
| 1          |                |                |                    |
| 2          |                |                |                    |
| 3          |                |                |                    |


Test Location: Cootes Paradise  
Exact GPS Coordinates: 

## SBAS & Differential GNSS

* Enable SBAS
* View output to see if connection is established and whether GPS has improved once a connection is established
* See SBAS Section in Specs. for more info

## Providing Aided Data
* Providing general lat/lng/height data,
* Requires an external memory device connected to the GPS chip using SPI. Alternativly, the data can be saved on thr Arduino (or maybe computer for testing) as the 'Host' and sent via the EX/TX pins to the GPS chip

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
