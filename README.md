<div align="center">
    <h1>re:mote</h1>
    <p>Open-source Low-cost Software and Hardware Infrastructure for Remote Sensing</p>
</div>

## 📎 Table of Contents

1. [About re:mote](#-about-remote)
2. [LoRa & Mesh Network](#-lora--mesh-network)
3. [Setup](#-get-started)
    - [Water Quality Sensing](#setup-water-quality-sensing)
    - [Turtle Tracker](#setup-turtle-tracker)
4. [Documentation](#-documentation)
5. [Acknowledgement](#-acknowledgment)

## 📒 About re:mote

### Low-cost Water Quality Sensing and Turtle Tracking

This is a software and hardware infrastructure for long-term monitoring of water quality parameters and seasonal monitoring of turtle movements that normally cost a lot more to make, but you can do it by yourself at a fraction of the price compared to commercial products. It supports various configurations, including different sensors, network options, and enclosures, making it adaptable to diverse environments. Designed for low power consumption for 3 month using 6 AA batteries., it can be deployed in a remote area with limited cell connection.

<div align="center">
    <img src="/Documentation/Pictures/mw_tt.webp" height="350px">
    <p>Working Water Sensor / Turtle Tracker</p>
</div>

---

## 🛰 LoRa & Mesh Network

### Low-power long-range sensor network

A LoRa mesh network is optionally used for connecting motes (motes can operate standalone).
- It is a low-bandwidth, low-power, and long-range network.
- The mesh network is tolerant to faults, changing network topology, extension, and contraction.

<div align="center">
    <img src="/Documentation/Pictures/Mesh_Tube.png" width="800px">
</div>

### Motes

- Mote is an old term of an IoT device which is connected to network.

Water monitoring network:
- **Gateway** (one or multiple) to relay motes data to the server over the
internet using a CAT-M1/NB-IoT/LTE/3G connection.
- **End Motes** (one or multiple) to measure water quality parameters through attached sensors.
    - **pH** sensor shows the acidity or alkalinity, helping to detect imbalances.
    - **Conductivity** sensor reveals the amount of salts present, providing insight into water quality.
    - **Dissolved Oxygen** sensor measures oxygen levels in the water, indicating the health of aquatic life.
    - **Water Temperature** sensor helps calibrate all sensor levels.
    - **Turbidity** sensor indicates water clarity, with changes signalling potential disturbances.
    - **[Custom sensors](/Documentation/Water_Sensor/Custom_Sensors.md)**

### Server

For real-time monitoring, the Raspberry Pi is an affordable option. However, a regular PC can also be utilized if preferred.

The server is programmed in Golang for fast and concurrent connections.
- A progressive web application designed in React with a custom API.
    - A Progressive Web Application (PWA) enables you to access and browse your data conveniently on your phone.
    - A custom API facilitates easy integration with various other systems.
- A time-series database (InfluxDB) for simple data storage and a smaller footprint. Time series databases assume that insertions are more frequent than queries, so it allows for the fast insertion of large amounts of data, such as water quality data. [Learn More](https://gitlab.cas.mcmaster.ca/re-mote/publications/-/tree/master/FadhelSekerinskiYao18Timeseries)

## 🚀 Get started

This list will guide you step by step to get started.

---

### Setup [Water Quality Sensing](Water_Sensor/)

1. Get all the [hardware](/Documentation/Water_Sensor/Specification.md) you need.

2. Setup a water sensor server with one of the options below. They perform with the same functionaility, with docker you do not need to configured all dependency and run on your own server or computer.
    - Install water sensor server on [Docker](Water_Sensor/re-mote-webserver/). **(Recommended)**
    - Install water sensor server on [Linux](/Documentation/Water_Sensor_Server/Server_Instructions.md).
    - Install water sensor server on a [cloud service](https://gitlab.cas.mcmaster.ca/re-mote/publications/tree/master/HuangMengqi19Encryption&Storage).

3. Assembly of Hardware for the [SensorMotes and/or Gateways](/Documentation/Water_Sensor/Build_a_Water_Sensor_Portal.md).

4. [Testing and Deploying](/Documentation/Water_Sensor/Build_a_Water_Sensor_Portal.md#testing).

---

### Setup [Turtle Tracker](Turtle_Tracker/)

1. Get all the [hardware](Turtle_Tracker/Docs/Specification.md) you need.

2. Setup a water sensor server with one of the options below.
    - Install water sensor server on [Linux](/Documentation/Water_Sensor_Server/Server_Instructions.md) with the [source code](Turtle_Tracker/turtle-tracker-webserver).
    - Install turtle tracker server on a [cloud service](https://gitlab.cas.mcmaster.ca/re-mote/publications/tree/master/HuangMengqi19Encryption&Storage).

3. Assemble Hardware and Upload the Firmware for [Tracker](Turtle_Tracker/Docs/Assembly_of_Trackers.md) and [Gateway](Turtle_Tracker/Docs/Assembly_of_Gateway.md).

---

### Get help

- If you have any questions or encounter issues, post them [here](https://github.com/re-mote-sensing/re-mote/issues).

## 🗂 Documentation

- For developers only.

 - Water Sensor
    - End Mote & Gateway
        - [LoRa message format](/Documentation/Water_Sensor/Message_Formats.md)
        - [Saved data formats](/Documentation/Water_Sensor/Saved_Data_Formats.md)
        - [Add custom sensors](/Documentation/Water_Sensor/Custom_Sensors.md)
    - Server
        - [Jupyter analysis](Jupyter_Analysis/)
        - [REST api](https://documenter.getpostman.com/view/5847961/2s83tDpshb)
        - [Database format](/Documentation/Water_Sensor_Server/Database_Format.md)
 - Turtle Tracker
    - End Mote & Gateway
        - [LoRa message format](Turtle_Tracker/Docs/message_format_turtle_tracker.xlsx)
 - Research
    - [Research documents](Research/)

## 🏆 Acknowledgment

[McMaster University](https://www.mcmaster.ca)

[Global Water Futures](https://gwf.usask.ca)

[MacWater](http://www.macwater.org)

[Ontario Research Fund](https://www.ontario.ca/page/ontario-research-fund)
