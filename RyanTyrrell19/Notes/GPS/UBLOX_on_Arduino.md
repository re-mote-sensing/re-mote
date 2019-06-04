# Planner

1) Send LORa transmission with empty payload. If no ACK is received, Tracker is underwater (or Gateway is off/damaged/far away). Regardless of the reason, power back down
2) Power on GPS
3) Check if data (e.g config settings) have saved, if not, reload them onto the reciever
    * Try and make this not an issue. DO tets with the BBR
    * Read the Almanac Data
4) Allow GPS to get a fix
4.5) While GPS is getting a fix, read the following
    * RTC
    * # SVs Used
    * 
3) Read from reciver
    * RTC
        * USe to compute time since last fix