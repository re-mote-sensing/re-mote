# Build a Water Sensor

### Choose one of the options below

<details>
<summary>Using mesh network (LoRa) with a gateway</summary>
<br>

This is an option using mesh network where 3G signal is not available/stable on the water.

A gateway is required to relay the data from node to server.

## Build Sensor Node

Link: [Build EndNode](Documentation/Water_Sensor/Build_EndNode.md)

### Connect LoRa Module

**LoRA:**

- Total 7 pins on the chip: Used Tx, Rx, Vcc and GND <u>(IGNORE SET, AUX and EN pins)</u>.
- Connect the Vcc of LoRA with the 5V of Arduino. Secondly, GND of LoRA with GND
- Connect LoRA Tx with Arduino A4
- Connect LoRA Rx with Arduino A5

<img src="Documentation/Pictures/water_arduino/Water_Sensor_End_Node_2023_bb_LoRa.png" width="50%">

## Build Gateway

Link: [Build Gateway](Documentation/Water_Sensor/Build_Gateway.md)

Link: [Gateway Enclosure](Documentation/Water_Sensor/Gateway_Enclosure_Assembly.md)

## Flash firmware for Nodes

### remoteConfig.h

Follow the following example to config the sensor node

The remoteConfig.h file should be in your Arduino Library folder `Arduino/libraries/re-moteConfig/remoteConfig.h`

[Mesh remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/meshNode.h)

### Flash

Link: [Flash firmware](Documentation/Water_Sensor/Flash_firmware.md)

## Flash firmware for Gateway

### remoteConfig.h

Follow the following example to config the gateway node

The remoteConfig.h file should be in your Arduino Library folder `Arduino/libraries/re-moteConfig/remoteConfig.h`

[Mesh remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/meshNode.h)

### Flash

Link: [Flash firmware](Documentation/Water_Sensor/Flash_firmware.md)

</details>

---

<details>
<summary>Using 3G on sensor node without gateway</summary>
<br>

This is an option using 3G on all sensor nodes where all nodes have 3G signal available and they are far from each other.

## Build Sensor Node

Link: [Build EndNode](Documentation/Water_Sensor/Build_EndNode.md)

## Connect 3G Module

**3G Shield:**

- Total 7 pins on the chip: Used Tx, Rx, Vcc and GND <u>(IGNORE SET, AUX and EN pins)</u>.
- Attach 3G Shield to Arduino Mega or only connect the Pins
- Connect 3G Tx with Arduino 11
- Connect 3G Rx with Arduino 10

<img src="Documentation/Pictures/water_arduino/Water_Sensor_End_Node_2023_bb_3g.png" width="50%">

## Flash firmware for Nodes

### remoteConfig.h

Follow the following example to config the sensor node

The remoteConfig.h file should be in your Arduino Library folder `Arduino/libraries/re-moteConfig/remoteConfig.h`

[3G remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/singleNode.h)

### Flash

Link: [Flash firmware](Documentation/Water_Sensor/Flash_firmware.md)

</details>

---

<details>
<summary>Using as a data logger and store data on SD card without uploading to server</summary>
<br>

This is an option if you do not need real-time data and retrieve data only from an SD card.

## Build Sensor Node

Link: [Build EndNode](Documentation/Water_Sensor/Build_EndNode.md)

## Flash firmware for Nodes

### remoteConfig.h

Follow the following example to config the sensor node

The remoteConfig.h file should be in your Arduino Library folder `Arduino/libraries/re-moteConfig/remoteConfig.h`

[dataLogger remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/dataLogger.h)

### Flash

Link: [Flash firmware](Documentation/Water_Sensor/Flash_firmware.md)

</details>

### Build a Pipe Enclosure

Link: [Pipe Enclosure Assembly](Documentation/Water_Sensor/Pipe_Enclosure_Assembly.md)

### Calibrate Sensors

Link: [Sensors Calibration](Documentation/Water_Sensor/Sensors_Calibration.md)