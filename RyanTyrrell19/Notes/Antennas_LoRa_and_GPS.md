# Active and Passive Antennas





# Other Notes

* Design and optimize the antenna 
    * Antenna shape, size, length
        * https://github.com/OpenHR20/OpenHR20/wiki/2.1)--433-MHz-and-868-MHz--Antenna-Design-Examples
        * http://e.pavlin.si/2018/06/29/small-normal-mode-antenna-for-868mhz/
    * Antenna connection
        * In regard to connecting to the LoRa MESH Module, I believe the connector can be desoldered from the board, and an antenna can simply be soldered on. Using a coaxial wire as the antenna, there shouldn’t be an issue with attaching it to the modules since, despite the extra hardware used to connect the original antenna, all it does in the end is connect the coaxial wire to the board.
        * https://en.wikipedia.org/wiki/SMA_connector
            * Maybe not quite so straight forward. Recall the necessary 50ohm resistance.
        * https://www.google.com/search?q=remove+SMA+connector+and+solder+antenna+directly&oq=remove+SMA+connector+and+solder+antenna+directly&aqs=chrome..69i57.16186j0j7&sourceid=chrome&ie=UTF-8#kpvalbx=1
            * Video on removing the connector
