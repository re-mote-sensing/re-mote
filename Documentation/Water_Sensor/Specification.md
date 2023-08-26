# Specification for Water Quality Sensing

## Server Node

Any PC that runs Docker can be used (Windows, Linux, MacOS).

| Part           | Link                                                         |
| -------------- | ------------------------------------------------------------ |
| Raspberry Pi 4 | https://www.raspberrypi.com/products/raspberry-pi-4-model-b/ |
| SD card 32 GB *  | https://www.amazon.ca/gp/product/B08GY9NYRM/                 |

* You have the flexibility to use various SD cards; here's an example for reference.

<br><br>

## Water Sensor End Node

| Part                            | Link                                                               |
| ------------------------------- | ------------------------------------------------------------------ |
| Arduino Mega                    | https://www.amazon.ca/gp/product/B01H4ZDYCE/                       |
| GPS Receiver - GP-735           | https://www.sparkfun.com/products/13670                            |
| JST connector (for GPS)         | https://www.digikey.ca/en/products/detail/sparkfun-electronics/PRT-10361/6605206 |
| MicroSD Card *                  | https://www.amazon.ca/gp/product/B08J4HJ98L/                                     |
| MicroSD Card Reader Module      | https://www.amazon.ca/gp/product/B07GXBP672/                                     |
| AA Battery Holder (3 x 1.5v) *  | https://www.amazon.ca/gp/product/B07PJT5NHL/                       |

* You're not limited to a specific SD card; here's an example for your consideration. However, it's essential to note that the SPI card reader lacks error correction. To mitigate potential issues, opt for a reputable, high-quality brand.
* You can explore alternative battery options, provided the voltage falls within the 4.5 to 5.5V range.

## Sensors

- It's recommended to use the specified sensors. However, if you opt for alternative sensors, be aware that programming skills might be necessary for proper integration.

| Part                    | Link                                                               |
| ----------------------- | ------------------------------------------------------------------ |
| Conductivity Kit        | https://atlas-scientific.com/kits/conductivity-k-0-1-kit/          |
| Dissolved Oxygen Kit    | https://atlas-scientific.com/kits/dissolved-oxygen-kit/            |
| Turbidity Sensor        | https://www.dfrobot.com/product-1394.html                          |
| Temperature Sensor Kit  | https://www.dfrobot.com/product-1354.html                          |
| Industrial pH Probe     | https://atlas-scientific.com/probes/industrial-ph-probe/           |
| EZO™ Carrier Board for pH | https://atlas-scientific.com/carrier-boards/electrically-isolated-ezo-carrier-board-gen-2/ |
| EZO™ pH Circuit         | https://atlas-scientific.com/embedded-solutions/ezo-ph-circuit/    |

### Additional Hardware
Depending on your network preference, please select one of the options provided below.

---

<details>
<summary>Using mesh network (LoRa) with a gateway</summary>
<br>

This approach employs a mesh network, ideal for situations where the 3G signal is unstable or unavailable over water.

<div align="center">
    <img src="Documentation/Pictures/Mesh_Tube.png" width="60%">
</div>

A gateway is essential to facilitate the transmission of data from the node to the server.

## Water Sensor End Node with **LoRa** Additional Components

| Part                    | Link                                                               |
| ----------------------- | ------------------------------------------------------------------ |
| Mesh LoRa [YL-800N Datasheet](https://gitlab.cas.mcmaster.ca/re-mote/arduino-motes/-/blob/master/Documentation/Data%20Sheets/LoRa_MESH_Radio_YL-800N_EN.pdf)               | https://www.dfrobot.com/product-1670.html                          |

<br>

## Water Sensor Gateway Node (Only if using LoRa on End Node)

| Part                                        | Link                                                                             |
| ------------------------------------------- | -------------------------------------------------------------------------------- |
| Arduino Uno                                 | https://www.canadarobotix.com/products/60                                        |
| 3G Shield *                                 | https://www.tinyosshop.com/3g-gprs-gsm-shield-for-arduino-sim5320a               |
| Mesh LoRa                                   | https://www.dfrobot.com/product-1670.html                                        |
| MicroSD Card                                | https://www.amazon.ca/gp/product/B08J4HJ98L/                                     |
| MicroSD Card Reader Module                  | https://www.amazon.ca/gp/product/B07GXBP672/                                     |
| DHT Sensor                                  | https://www.amazon.ca/gp/product/B07CM2VLBK/                                     |
| SIM Card with data plan (120 days)          |                                                                                  |

* Depending on your location or country, you may require a different version of the 3G shield that is compatible with your SIM card.

</details>

---

<details>
<summary>Using 3G on sensor node without gateway</summary>
<br>

This approach involves equipping all sensor nodes with 3G connectivity. It's ideal for setups where all nodes are within areas with available 3G signal and are spaced at considerable distances from one another.

<div align="center">
    <img src="Documentation/Pictures/SingleNode.png" width="60%">
</div>

## Water Sensor End Node with **3G** Additional Components

| Part                    | Link                                                               |
| ----------------------- | ------------------------------------------------------------------ |
| 3G Shield *                                 | https://www.tinyosshop.com/3g-gprs-gsm-shield-for-arduino-sim5320a               |
| SIM Card with data plan (120 days) *        |                                                                                  |

* Depending on your location or country, you may require a different version of the 3G shield that is compatible with your SIM card.
* As of 2023 in Canada, Virgin/Fido offers a $10 tablet plan that aligns well with the requirements of this project. Alternatively, considering an IoT SIM card could also be an effective solution.

</details>

---

<details>
<summary>Using as a data logger and store data on SD card without uploading to server</summary>
<br>

This option is suitable if you don't require real-time data and prefer to retrieve information solely from an SD card.

<div align="center">
    <img src="Documentation/Pictures/DataLogger.png" width="60%">
</div>

No additional hardware needed.

</details>