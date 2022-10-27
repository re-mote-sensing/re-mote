# Tracker Assembly

## Hardware Assembly

Cut GPS cable

<img src="./images/assembly/tracker_gps_cable.jpg" width="300px" alt="Image">

Cut power cable

<img src="./images/assembly/tracker_power_cable.jpg" width="300px" alt="Image">

Get all parts ready

<img src="./images/assembly/tracker_ready_for_soldering.jpg" width="500px" alt="Image">

Connect the cable to moteino and solder in place

**GPS ([Manual](./manual/GP-735T-150203.pdf)):**

- Connect GPS pin 6 to D6
- Connect GPS pin 4 to D4
- Connect GPS pin 3 to D3
- Connect GPS pin 2 to Vin / PWR
- Connect GPS pin 1 to Ground

<img src="./images/assembly/tracker_gps_pins.png" width="300px" alt="Image">

Check with your battery if your battery has reversed Vcc and Ground, the power cable should also be reversed, in this case, the cable is reversed

<img src="./images/assembly/tracker_soldering.jpg" width="500px" alt="Image">

## Flash firmware

Connect TX, RX, Reset, and power cable to moteino

<img src="./images/assembly/moteino_layout.jpg" width="500px" alt="Image">

<br>

<img src="./images/assembly/tracker_moteino_flasher_2.jpg" width="500px" alt="Image">

Install the `Moteino AVR Boards` package in Arduino IDE: [Programming & Libraries](https://lowpowerlab.com/guide/moteino/programming-libraries/)

Move `../Arduino_Code/Library/libraries` to Arduino Library folder

- Mac: ~/Documents/Arduino/libraries/
- Windows: /Users/{username}/Documents/Arduino/libraries/

Flash with Arduino IDE

## Enclosure

3D Printed Tracker Enclosure: [Enclosure](./3D_Printed_Enclosure/README.md)

Spray `Plasti Dip` to make the 3D printing waterproof

<img src="./images/assembly/tracker_enclosure_printed.jpg" width="500px" alt="Image">

Spray `Rust Aerosol Spray Paint` to make the screws not rusted

<img src="./images/assembly/tracker_screws_waterproofing.jpg" width="500px" alt="Image">

## Prepare to deploy:

Charge the batteries

Caution: The 1600mAh Battery (currently using) Vcc and Gnd are reversed.

<img src="./images/assembly/tracker_battery_charging.jpg" width="500px" alt="Image">

Put the moteino and battery into the enclosure

<img src="./images/assembly/tracker_enclosure_open.jpg" width="500px" alt="Image">

Seal the enclosure with a Gasket

<img src="./images/assembly/tracker_gasket_for_sealing.jpg" width="500px" alt="Image">

Completed Trackers

<img src="./images/assembly/tracker_complete.jpg" width="500px" alt="Image">
