# Sensors Calibration through Web Serial

Ensure all sensors are attached properly, and room temperature is around 25 degrees

Open the calibration website at https://serverip/configuration Chrome or a web browser with USB web serial supported (Public Website: https://macwater.cas.mcmaster.ca/configuration/)

Press `F12` to open the dev tool and navigate to the `Console` tab

Connect the Arduino board to your computer using USB.

Click `Connect` and select the device

<img src="Documentation/Pictures/calibrate/Screenshot2023-05-29at2.31.41PM.png">

### Supported Sensors

- Atlas Scienticfic pH Sensor
- Atlas Scienticfic EC Sensor
- Atlas Scienticfic DO Sensor

<img src="Documentation/Pictures/calibrate/IMG_3448.jpeg" width="50%">

## pH Sensor Calibration

<img src="Documentation/Pictures/calibrate/IMG_3443.jpeg" width="50%">

<img src="Documentation/Pictures/calibrate/Screenshot2023-05-29at2.31.22PM.png" width="50%">

Put the pH sensor into pH7 calibration liquid, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration Mid Point`

Put the pH sensor into pH4 calibration liquid, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration Low Point`

Put the pH sensor into pH10 calibration liquid, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration High Point`

Press the `View Calibration Info` button, and make sure the reading is `cal, 3` (3 Point Calibration)

## EC Sensor Calibration

<img src="Documentation/Pictures/calibrate/Screenshot2023-05-29at2.31.24PM.png" width="50%">

Ensure the sensor is dry, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration Air`

### K = 0.1

<img src="Documentation/Pictures/calibrate/IMG_3445.jpeg" width="50%">

Put the EC sensor into 84us calibration liquid, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration Low Point`

Put the EC sensor into 1413us calibration liquid, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration High Point`

### K = 1

<img src="Documentation/Pictures/calibrate/IMG_3446.jpeg" width="50%">

Put the EC sensor into 12,880us calibration liquid, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration Low Point`

Put the EC sensor into 80,000us calibration liquid, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration High Point`

Press the `View Calibration Info` button, make sure the reading is `cal, 2`

## DO Sensor Calibration

Follow the instruction on the Atlas Scientific documents to [recondition the lab-grade dissolved oxygen probe](https://files.atlas-scientific.com/LG_DO_probe.pdf#page=9)

<img src="Documentation/Pictures/calibrate/IMG_3444.jpeg" width="50%">

<img src="Documentation/Pictures/calibrate/Screenshot2023-05-29at2.31.18PM.png" width="50%">

Ensure the sensor is dry, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration Air`

Press the `View Calibration Info` button, and make sure the reading is `cal, 1`

### Calibrate with liquid only required for reading near 0

Put the DO sensor into calibration liquid, using the `read x10` button to check when the reading is stable.

After the reading is stable, press `Calibration Zero DO`

Press the `View Calibration Info` button, make sure the reading is `cal, 2`

# Sensors Calibration through UART

Follow the instruction on the Atlas Scientific documents to calibrate the sensors, calibrate info are stored on the chip.

- pH
	- https://www.atlas-scientific.com/files/pH_EZO_Datasheet.pdf#page=11
- EC
	- https://www.atlas-scientific.com/files/EC_EZO_Datasheet.pdf#page=12
- DO
	- https://www.atlas-scientific.com/files/DO_EZO_Datasheet.pdf#page=9
