# Build a Water Sensor

### Choose one of the options below

<details>
<summary>Using mesh network (LoRa) with a gateway</summary>
<br>

This is an option using mesh network where 3G signal is not available/stable on the water.

<div align="center">
    <img src="Documentation/Pictures/Mesh_Tube.png" width="60%">
</div>

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

The main configuration file, remoteConfig.h, can be found under the re-moteConfig library file `Arduino/libraries/re-moteConfig/remoteConfig.h`. This is the only file you need to change to configure the Arduinos (assuming you aren't adding custom support). You need to comment and uncomment out lines depending on your configuration, change `Node_Type` if you're uploading to a Gateway or an End node. The settings in the file all have clear comments that explain what that setting does.

Follow the following example to config the sensor node

[Mesh remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/meshNode.h)

### Flash

Link: [Flash firmware](Documentation/Water_Sensor/Flash_firmware.md)

## Flash firmware for Gateway

### remoteConfig.h

The main configuration file, remoteConfig.h, can be found under the re-moteConfig library file `Arduino/libraries/re-moteConfig/remoteConfig.h`. This is the only file you need to change to configure the Arduinos (assuming you aren't adding custom support). You need to comment and uncomment out lines depending on your configuration, change `Node_Type` if you're uploading to a Gateway or an End node. The settings in the file all have clear comments that explain what that setting does.

Follow the following example to config the gateway node

[Mesh remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/meshNode.h)

### Flash

Link: [Flash firmware](Documentation/Water_Sensor/Flash_firmware.md)

</details>

---

<details>
<summary>Using 3G on sensor node without gateway</summary>
<br>

This is an option using 3G on all sensor nodes where all nodes have 3G signal available and they are far from each other.

<div align="center">
    <img src="Documentation/Pictures/SingleNode.png" width="60%">
</div>

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

The main configuration file, remoteConfig.h, can be found under the re-moteConfig library file `Arduino/libraries/re-moteConfig/remoteConfig.h`. This is the only file you need to change to configure the Arduinos (assuming you aren't adding custom support). You need to comment and uncomment out lines depending on your configuration. The settings in the file all have clear comments that explain what that setting does.

Follow the following example to config the sensor node

[3G remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/singleNode.h)

### Flash

Link: [Flash firmware](Documentation/Water_Sensor/Flash_firmware.md)

</details>

---

<details>
<summary>Using as a data logger and store data on SD card without uploading to server</summary>
<br>

This is an option if you do not need real-time data and retrieve data only from an SD card.

<div align="center">
    <img src="Documentation/Pictures/DataLogger.png" width="60%">
</div>

## Build Sensor Node

Link: [Build EndNode](Documentation/Water_Sensor/Build_EndNode.md)

## Flash firmware for Nodes

### remoteConfig.h

The main configuration file, remoteConfig.h, can be found under the re-moteConfig library file `Arduino/libraries/re-moteConfig/remoteConfig.h`. This is the only file you need to change to configure the Arduinos (assuming you aren't adding custom support). You need to comment and uncomment out lines depending on your configuration. The settings in the file all have clear comments that explain what that setting does.

Follow the following example to config the sensor node

[dataLogger remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/dataLogger.h)

### Flash

Link: [Flash firmware](Documentation/Water_Sensor/Flash_firmware.md)

</details>

## Build a Pipe Enclosure

Link: [Pipe Enclosure Assembly](Documentation/Water_Sensor/Pipe_Enclosure_Assembly.md)

## Calibrate Sensors

Link: [Sensors Calibration](Documentation/Water_Sensor/Sensors_Calibration.md)


# Testing

It's important to test your setup at every stage of this document to make sure everything is functioning correctly. It is also highly recommended that once everything is finished, you run a full test over the course of a few days. This will ensure that everything is functioning as expected.

# Deploying

Once everything is completely ready to be deployed, it is important to deploy them correctly. The first step is to choose your deployment location carefully. The LoRa mesh modules used allow for your nodes to be set up in a mesh configuration, which means that as long as a node (node 1) is within range of any other node (node2) that is connected to a gateway, the first node (node 1) will be able to connect to the gateway. This allows for a long string of end nodes connected to each other and only one gateway. The tested range of these LoRa mesh modules is about 200m through heavy obstruction (hill, buildings, etc.), about 500m through light obstruction (trees, etc.), and 650m through very light obstruction (chain link fences, nothing, etc.). This range can be improved by lifting the antenna, but by how much depends heavily on your circumstances (tested up to about +100m). It is also important to consider the legality of your deployment locations; you must ensure that you are not breaking any laws by deploying your nodes in the locations.

The next thing to consider when deploying your nodes is hardware safety. It is important to make sure that your nodes will not be dislodged or washed array by heavy rains, strong winds, or snow (depending on the weather where you're deploying). Depending on your deployment locations, it may also be important to consider hiding/securing your nodes in some way to prevent vandalism. This could be done by burying your nodes (although this may affect GPS and LoRa connectivity), painting them to blend in, or covering them with something (could be leaves, or even trash, etc.).

Once the nodes have been secured, it is important to deploy the sensors so that they are secure. For us, this involved putting a large steel pole into the water, and securing the sensor probes to it using water proof tape. The steps to do this will depend on the location and sensor probes you are using.

# Troubleshooting

The first step to troubleshooting is to make sure you've configured and connected everything correctly. If that didn't help, the next step is to identify the problem. This is most easily done by isolating parts of the Arduino and seeing which ones function correctly and which ones do not. If you've been testing after every step of this document, this should be quite easy. Once you've isolated the problem (let's say your DO sensor isnt initialising), it is good practice to re-check the configuration for this module to make sure that isn't the cause of the problem. If that wasn't the problem, you then need to determine if the problem is with the module or the Arduino. This is most easily done if you have another Arduino or sensor to swap in. If not, you can try to change the pins the sensor is connected to (ideally to pins you know work). If the problem is with the sensor, you may have gotten a defective one and will have to contact the seller. If the issue is with the Arduino, you can try changing the pins the sensor is connected to to see if some of the pins are broken. If that doesn't help, try testing the Arduino with a simple LED blink sketch. If the module and Arduino work seperately, but not together, you will have to do proper software bug finding and fixing, for which it is recommended that you have some form of experience with software development, preferrably in Arduino or C/C++.