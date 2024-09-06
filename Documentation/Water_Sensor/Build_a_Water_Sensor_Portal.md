# Build a Water Sensor

### Depending on your network preference, select one of the options provided below.

<details>
<summary>Using LoRa with one or more gateways</summary>
<br>

This approach employs a mesh network, ideal for situations where the 3G signal is unstable or unavailable over water.

<div align="center">
    <img src="/Documentation/Pictures/Mesh_Tube.png" width="60%">
</div>

A gateway is essential to facilitate the transmission of data from the node to the server.

## Build a Sensor Node

Link: [Build EndNode](/Documentation/Water_Sensor/Build_EndNode.md)

### Connect LoRa Module

**LoRA:**

- Total of 7 pins on the chip: Used Tx, Rx, Vcc and GND <u>(IGNORE SET, AUX and EN pins)</u>.
- Connect the Vcc of LoRA to the 5V of Arduino. Second, the GND of LoRA with GND.
- Connect LoRA Tx with Arduino A4.
- Connect LoRA Rx with Arduino A5.

<img src="/Documentation/Pictures/water_arduino/Water_Sensor_End_Node_2023_bb_LoRa.png" width="50%">

## Build gateway

Link: [Build Gateway](/Documentation/Water_Sensor/Build_Gateway.md)

Link: [Gateway Enclosure](/Documentation/Water_Sensor/Gateway_Enclosure_Assembly.md)

## Upload firmware for nodes

### remoteConfig.h

The primary configuration file is remoteConfig.h, located within the `Arduino/libraries/re-moteConfig/` directory. To configure your Arduinos:

1. Navigate to the re-moteConfig library: `Arduino/libraries/re-moteConfig/remoteConfig.h`.
2. This is the sole file you'll need to modify unless you're introducing custom functionalities.
3. Within the file, comment or uncomment lines as per your desired configuration.
4. Adjust the `Node_Type` value based on whether you're working with a Gateway or an End node.
5. Each setting within the file comes with comprehensive comments to guide you on its purpose and function.
6. Ensure you follow the instructions carefully for optimal performance.

To configure the sensor node, refer to the example provided below.

[Mesh remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/meshNode.h)

### Upload

Link: [Upload firmware](/Documentation/Water_Sensor/Upload_firmware.md)

## Upload firmware for Gateway

### remoteConfig.h

The primary configuration file is remoteConfig.h, located within the `Arduino/libraries/re-moteConfig/` directory. To configure your Arduinos:

1. Navigate to the re-moteConfig library: `Arduino/libraries/re-moteConfig/remoteConfig.h`.
2. This is the sole file you'll need to modify unless you're introducing custom functionalities.
3. Within the file, comment or uncomment lines as per your desired configuration.
4. Adjust the `Node_Type` value based on whether you're working with a Gateway or an End node.
5. Each setting within the file comes with comprehensive comments to guide you on its purpose and function.
6. Ensure you follow the instructions carefully for optimal performance.

To configure the gateway node, refer to the example provided below.

[Mesh remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/meshNode.h)

### Upload

Link: [Upload firmware](/Documentation/Water_Sensor/Upload_firmware.md)

</details>

---

<details>
<summary>Using 3G on sensor node without gateway</summary>
<br>

This approach involves equipping all sensor nodes with celluar connectivity. It's ideal for setups where all nodes are within areas with available celluar signals and are spaced at considerable distances from one another.

<div align="center">
    <img src="/Documentation/Pictures/SingleNode.png" width="60%">
</div>

## Build a Sensor Node

Link: [Build EndNode](/Documentation/Water_Sensor/Build_EndNode.md)

## Connect the 3G Module

**3G shield:**

- Total of 7 pins on the chip: Used Tx, Rx, Vcc and GND <u>(IGNORE SET, AUX and EN pins)</u>.
- Attach celluar shield to Arduino Mega or only connect the pins.
- Connect 3G Tx with Arduino 11.
- Connect 3G Rx with Arduino 10.

<img src="/Documentation/Pictures/water_arduino/Water_Sensor_End_Node_2023_bb_3g.png" width="50%">

## Upload firmware for nodes

### remoteConfig.h

The primary configuration file is remoteConfig.h, located within the `Arduino/libraries/re-moteConfig/` directory. To configure your Arduinos:

1. Navigate to the re-moteConfig library: `Arduino/libraries/re-moteConfig/remoteConfig.h`.
2. This is the sole file you'll need to modify unless you're introducing custom functionalities.
3. Within the file, comment or uncomment lines as per your desired configuration.
4. Each setting within the file comes with comprehensive comments to guide you on its purpose and function.
5. Ensure you follow the instructions carefully for optimal performance.

To configure the sensor node, refer to the example provided below.

[3G remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/singleNode.h)

### Upload

Link: [Upload firmware](/Documentation/Water_Sensor/Upload_firmware.md)

</details>

---

<details>
<summary>Using as a data logger and store data on the SD card without uploading to server</summary>
<br>

This option is suitable if you don't require real-time data and prefer to retrieve information solely from an SD card.

<div align="center">
    <img src="/Documentation/Pictures/DataLogger.png" width="60%">
</div>

## Build a Sensor Node

Link: [Build EndNode](/Documentation/Water_Sensor/Build_EndNode.md)

## Upload firmware for nodes

### remoteConfig.h

The primary configuration file is remoteConfig.h, located within the `Arduino/libraries/re-moteConfig/` directory. To configure your Arduinos:

1. Navigate to the re-moteConfig library: `Arduino/libraries/re-moteConfig/remoteConfig.h`.
2. This is the sole file you'll need to modify unless you're introducing custom functionalities.
3. Within the file, comment or uncomment lines as per your desired configuration.
4. Each setting within the file comes with comprehensive comments to guide you on its purpose and function.
5. Ensure you follow the instructions carefully for optimal performance.

To configure the sensor node, refer to the example provided below.

[dataLogger remoteConfig.h](Water_Sensor/Libraries/re-moteConfig/Example/dataLogger.h)

### Upload

Link: [Upload firmware](/Documentation/Water_Sensor/Upload_firmware.md)

</details>

<br><br><br>

## Build a Pipe Enclosure

Link: [Pipe Enclosure Assembly](/Documentation/Water_Sensor/Pipe_Enclosure_Assembly.md)

## Calibrate Sensors

Link: [Sensors Calibration](/Documentation/Water_Sensor/Sensors_Calibration.md)


# Testing

It's important to test your setup at every stage of this document to make sure everything is functioning correctly. It is also highly recommended that once everything is finished, you run a full test over the course of a few days. This will ensure that everything is functioning as expected.

# Deploying

Once everything is completely ready to be deployed, it is important to deploy them correctly. The first step is to choose your deployment location carefully. The LoRa mesh modules used allow for your nodes to be set up in a mesh configuration, which means that as long as a node (node 1) is within range of any other node (node2) that is connected to a gateway, the first node (node 1) will be able to connect to the gateway. This allows for a long string of end nodes connected to each other and only one gateway. The tested range of these LoRa mesh modules is about 200m through heavy obstruction (hill, buildings, etc.), about 500m through light obstruction (trees, etc.), and 650m through very light obstruction (chain link fences, nothing, etc.). This range can be improved by lifting the antenna, but by how much depends heavily on your circumstances (tested up to about +100m). It is also important to consider the legality of your deployment locations; you must ensure that you are not breaking any laws by deploying your nodes in the locations.

The next thing to consider when deploying your nodes is hardware safety. It is important to make sure that your nodes will not be dislodged or washed away by heavy rains, strong winds, or snow (depending on the weather where you're deploying). Depending on your deployment locations, it may also be important to consider hiding/securing your nodes in some way to prevent vandalism. This could be done by burying your nodes (although this may affect GPS and LoRa connectivity), painting them to blend in, or covering them with something (could be leaves, or even trash, etc.).

Once the nodes have been secured, it is important to deploy the sensors so that they are secure. For us, this involved putting an anchor into the water and securing the sensor probes to it using waterproof tape. The steps to do this will depend on the location and sensor probes you are using.

# Troubleshooting

The first step to troubleshooting is to make sure you've configured and connected everything correctly. If that doesn't help, the next step is to identify the problem. This is most easily done by isolating parts of the Arduino and seeing which ones function correctly and which ones do not. If you've been testing after every step of this document, this should be quite easy. Once you've isolated the problem (let's say your DO sensor isn't initializing), it is good practice to re-check the configuration for this module to make sure that isn't the cause of the problem. If that wasn't the problem, you then need to determine if the problem is with the module or the Arduino. This is most easily done if you have another Arduino or sensor to swap in. If not, you can try to change the pins the sensor is connected to (ideally to pins you know work). If the problem is with the sensor, you may have gotten a defective one and will have to contact the seller. If the issue is with the Arduino, you can try changing the pins the sensor is connected to to see if some of the pins are broken. If that doesn't help, try testing the Arduino with a simple LED blink sketch. If the module and Arduino work separately, but not together, you will have to do proper software bug finding and fixing, for which it is recommended that you have some form of experience with software development, preferably in Arduino or C/C++.