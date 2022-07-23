# Tracker Assembly

## Hardware Assembly

Cut GPS cable

![tracker_gps_cable.jpg](./images/assembly/tracker_gps_cable.jpg)

Cut power cable

![tracker_power_cable.jpg](./images/assembly/tracker_power_cable.jpg)

Get all parts ready

![tracker_ready_for_soldering.jpg](./images/assembly/tracker_ready_for_soldering.jpg)

Connect the cable to moteino and solder in place

**GPS ([Manual](./manual/GP-735T-150203.pdf)):**

- Connect GPS pin 6 to D6
- Connect GPS pin 4 to D4
- Connect GPS pin 3 to D3
- Connect GPS pin 2 to Vin / PWR
- Connect GPS pin 1 to Ground

![tracker_gps_pins.png](./images/assembly/tracker_gps_pins.png)

Check with your battery if your battery has reversed Vcc and Ground, the power cable should also be reversed, in this case, the cable is reversed

![tracker_soldering.jpg](./images/assembly/tracker_soldering.jpg)

## Flash firmware

Connect TX, RX, Reset, and power cable to moteino

![moteino_layout.jpg](./images/assembly/moteino_layout.jpg)

![tracker_moteino_flasher.jpg](./images/assembly/tracker_moteino_flasher_2.jpg)

Install the `Moteino AVR Boards` package in Arduino IDE: [Programming & Libraries](https://lowpowerlab.com/guide/moteino/programming-libraries/)

Move `../Arduino_Code/Library/libraries` to Arduino Library folder

- Mac: ~/Documents/Arduino/libraries/
- Windows: /Users/{username}/Documents/Arduino/libraries/

Flash with Arduino IDE

## Enclosure

3D Printed Tracker Enclosure: [Enclosure](./3D_Printed_Enclosure/README.md)

Spray `Plasti Dip` to make the 3D printing waterproof

![tracker_enclosure_printed.jpg](./images/assembly/tracker_enclosure_printed.jpg)

Spray `Rust Aerosol Spray Paint` to make the screws not rusted

![tracker_screws_waterproofing.jpg](./images/assembly/tracker_screws_waterproofing.jpg)

## Prepare to deploy:

Charge the batteries

Caution: The 1600mAh Battery (currently using) Vcc and Gnd are reversed.

![tracker_battery_charging.jpg](./images/assembly/tracker_battery_charging.jpg)

Put the moteino and battery into the enclosure

![tracker_enclosure_open.jpg](./images/assembly/tracker_enclosure_open.jpg)

Seal the enclosure with a Gasket

![tracker_gasket_for_sealing.jpg](./images/assembly/tracker_gasket_for_sealing.jpg)

Completed Trackers

![tracker_complete.jpg](./images/assembly/tracker_complete.jpg)
