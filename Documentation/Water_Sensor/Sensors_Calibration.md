# Sensors Calibration through Web Serial

Ensure that all sensors are attached properly and that room temperature is around 25 degrees.

Open the calibration website at https://serverip/configuration on Chrome or a web browser with USB web serial supported (Public website: https://macwater.cas.mcmaster.ca/configuration/).

Press `F12` to open the development tool and navigate to the `Console` tab.

Connect the Arduino board to your computer using USB.

Click `Connect` and select the device.

<img src="Documentation/Pictures/calibrate/Screenshot2023-05-29at2.31.41PM.png">

### Supported Sensors

- Atlas Scienticfic pH Sensor
- Atlas Scienticfic EC Sensor
- Atlas Scienticfic DO Sensor

<img src="Documentation/Pictures/calibrate/IMG_3448.jpg" width="50%">

## pH Sensor Calibration

<img src="Documentation/Pictures/calibrate/IMG_3443.jpg" width="50%">

<img src="Documentation/Pictures/calibrate/Screenshot2023-05-29at2.31.22PM.png" width="50%">

Place the pH sensor in the pH7 calibration liquid, using the `read x10` button to check that the reading is stable.

After the reading is stable, press `calibration mid-point`.

Place the pH sensor in the pH4 calibration liquid, using the `read x10` button to check that the reading is stable.

After the reading is stable, press `Calibration Low Point`.

Place the pH sensor in the pH10 calibration liquid, using the `read x10` button to check that the reading is stable.

After the reading is stable, press `Calibration High Point`.

Press the `View Calibration Info` button, and make sure the reading is `cal, 3` (3 Point Calibration).

## EC Sensor Calibration

<img src="Documentation/Pictures/calibrate/Screenshot2023-05-29at2.31.24PM.png" width="50%">

Ensure that the sensor is dry, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration Air`.

### K = 0.1

<img src="Documentation/Pictures/calibrate/IMG_3445.jpg" width="50%">

Put the EC sensor into the 84us calibration liquid, using the `read x10` button to check that the reading is stable.

After the reading is stable, press `Calibration Low Point`.

Put the EC sensor into the 1413us calibration liquid, using the `read x10` button to check that the reading is stable.

After the reading is stable, press `Calibration High Point`.

### K = 1

<img src="Documentation/Pictures/calibrate/IMG_3446.jpg" width="50%">

Place the EC sensor into the 12,880us calibration liquid, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration Low Point`.

Put the EC sensor into the calibration liquid 80,000us, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration High Point`.

Press the `View Calibration Info` button, make sure that the reading is `cal, 2`.

## DO Sensor Calibration

Follow the instructions in the Atlas Scientific documents to [recondition the lab-grade dissolved oxygen probe](https://files.atlas-scientific.com/LG_DO_probe.pdf#page=9)

<img src="Documentation/Pictures/calibrate/IMG_3444.jpg" width="50%">

<img src="Documentation/Pictures/calibrate/Screenshot2023-05-29at2.31.18PM.png" width="50%">

Ensure that the sensor is dry, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration Air`.

Press the `View Calibration Info` button, and make sure the reading is `cal, 1`

### Calibrate with liquid only required for reading near 0.

Put the DO sensor in calibration liquid, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration Zero DO`.

Press the `View Calibration Info` button, make sure that the reading is `cal, 2`

# Sensors Calibration through UART

Follow the instruction on the Atlas Scientific documents to calibrate the sensors, calibrate info is stored on the chip.

- pH
	- https://www.atlas-scientific.com/files/pH_EZO_Datasheet.pdf#page=11
- EC
	- https://www.atlas-scientific.com/files/EC_EZO_Datasheet.pdf#page=12
- DO
	- https://www.atlas-scientific.com/files/DO_EZO_Datasheet.pdf#page=9