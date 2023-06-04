<div align="center">
    <h1>re:mote</h1>
    <p>Open-source Low-cost Software and Hardware Infrastructure for Remote Sensing</p>
</div>

## 📎 Table of Contents

1. [About re:mote](#-about-remote)
2. [LoRa & Mesh Network](#-lora-mesh-network)
3. [Setup](#-get-started)
    - [Water Quality Sensing](#water-quality-sensing)
    - [Turtle Trackers](#turtle-trackers)
4. [Documentation](#-documentation)
5. [Acknowledgement](#-acknowledgement)

## 📒 About re:mote

### Low-cost Water Quality Sensing and Turtle Tracking

This is a software and hardware infrastructure for long-term monitoring of water quality parameters and seasonal monitoring of turtle movements which normally cost 10k or more to make, but you can do it by yourself with 1/10 of the price.

<div align="center">
    <img src="Documentation/Pictures/mw_tt.webp" height="350px">
    <p>Working Water Sensor / Turtle Tracker</p>
</div>

---

### Arduino-based motes

- Can be config with different sensors and different forms
- They have Unreliable long distances communications, network topology always changing
- Limited power supply in remote area

The network consists of Arduino-based motes:
- **Gateway** to relay end-node data to the server over the
internet using a 3G connection.
- **End Nodes** to measure water quality parameters through connected sensors.
    - **pH** shows the acidity or alkalinity, helping detect imbalances.
    - **Conductivity** sensor reveals the amount of salts present, providing insight into water quality.
    - **Dissolved Oxygen** sensor measures oxygen levels in the water, indicating the health of aquatic life.
    - **Water Temperature** sensor helps calibrate all sensor levels.
    - **Turbidity** sensor indicates water clarity, with changes signalling potential disturbances.
    - **[Custom Sensors](Documentation/Software/Custom_Sensors.md)**

### Raspberry Pi Server

- Large amounts of simply-structured data
- Erroneous data with “real” anomalies
- Visualization and further programmatic analysis

A server programmed in Go for fast and concurrent connections.
- A progressive web application designed in React with a custom API.
- A time-series database (InfluxDB) for simple data storage and a smaller footprint. Time series databases assume insertions are more frequent than queries, so it allows for the fast insertion of large amounts of data such as water quality data. [Learn More](https://gitlab.cas.mcmaster.ca/re-mote/publications/-/tree/master/FadhelSekerinskiYao18Timeseries)

## 🛰 LoRa & Mesh Network

### Low-power Long-range Sensor Network

A LoRa mesh network is a optional used for connecting motes. [YL-800N Datasheet](https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/-/blob/master/Documentation/Data%20Sheets/LoRa_MESH_Radio_YL-800N_EN.pdf)
- It is a low-bandwidth, low-power and long-range network.
- The mesh network is tolerant to faults, changing network topology, extension and contraction.

<div align="center">
    <img src="Documentation/Pictures/Mesh_Tube.png" width="800px">
</div>

## 🚀 Get started

This list will guide you to get started step-by-step.

---

### Setup [Water Quality Sensing](Water_Sensor/)

1. Get all the [hardware](Documentation/Water_Sensor/Specification.md) you need

2. Setup a [Docker Server](https://gitlab.cas.mcmaster.ca/re-mote/pi-server/-/blob/master/macwater-webserver/README.md) on a Raspberry Pi or [build by yourself](https://gitlab.cas.mcmaster.ca/re-mote/pi-server/blob/master/Documentation/RaspberryPi_Instructions.md) with the [source code](https://gitlab.cas.mcmaster.ca/re-mote/pi-server/-/tree/master/macwater-webserver) or on a [Cloud Service](https://gitlab.cas.mcmaster.ca/re-mote/publications/tree/master/HuangMengqi19Encryption&Storage)

3. Assembling the Hardware for the [SensorNode and/or Gateway](Documentation/Water_Sensor/Build_a_Water_Sensor_Portal.md)

4. [Configuring](Documentation/Setup/Water_Quality_Setup.md#configuring-and-uploading-the-software) and Uploading the Software

---

### Setup [Turtle Trackers](Turtle_Trackers/)

1. Get all the [hardware](Turtle_Trackers/Docs/Specification.md) you need

2. Setup a [Server](https://gitlab.cas.mcmaster.ca/re-mote/pi-server/blob/master/Documentation/RaspberryPi_Instructions.md) on a Raspberry Pi with the [source code](https://gitlab.cas.mcmaster.ca/re-mote/pi-server/-/tree/master/turtle-tracker-webserver) or on a [Cloud Service](https://gitlab.cas.mcmaster.ca/re-mote/publications/tree/master/HuangMengqi19Encryption&Storage)

3. Assembling the Hardware and flash the firmware for [Tracker](Turtle_Trackers/Docs/Assembly_of_Trackers.md) and [Gateway](Turtle_Trackers/Docs/Assembly_of_Gateway.md)

---

### Get Help

1. If you have questions go post a issue [here](https://github.com/re-mote-sensing/arduino-motes/issues)

## 🗂 Documentation

 - Water Sensor
    - End Node & Gateway
        - [LoRa Message Format](Documentation/Software/Message_Formats.md)
        - [Saved Data Formats](Documentation/Software/Saved_Data_Formats.md)
        - [Add Custom Sensors](Documentation/Software/Custom_Sensors.md)
    - Pi Server
        - [REST Api](https://documenter.getpostman.com/view/5847961/2s83tDpshb)
        - [Database Format](https://gitlab.cas.mcmaster.ca/re-mote/pi-server/-/blob/master/Documentation/Database_Format.md)
        - [Juypter Notebook scripts](https://gitlab.cas.mcmaster.ca/re-mote/pi-server/-/tree/master/Usefull%20Scripts) for downloading and uploading data to server
 - Turtle Tracker
    - End Node & Gateway
        - [LoRa Message Format](Turtle_Trackers/Docs/message_format_turtle_tracker.xlsx)

## 🏆 Acknowledgement

[McMaster University](https://www.mcmaster.ca)

[Global Water Futures](https://gwf.usask.ca)

[MacWater](http://www.macwater.org)

[Ontario Research Fund](https://www.ontario.ca/page/ontario-research-fund)
