# Requirements and Existing Website Features

## Requirements

**Requirement #:** 1 <br/>
**Description:** The product shall allow manual entry of sensors' data such as its measurement type, value and timestamp <br/>
**Rationale:** To be able to enter lab samples on sensors that is currently not remotely connected to the database or captured before the system was configured<br/>

**Requirement #:** 2 <br/>
**Description:** The product shall allow pictures or videos with location tag to be stored into database <br/>
**Rationale:** Pictures or videos may provide more context regarding the data obtained from the sensors <br/>

**Requirement #:** 3<br/>
**Description:** The product shall have a map with markers of various sensors' locations<br/>
**Rationale:** There are different sensors located throughout the city, observing the location of sensors may provide more context to sensor data obtained <br/>

**Requirement #:** 4<br/>
**Description:** The markers on the map representing the sensors shall be colour coded<br/>
**Rationale:** Different types of sensors(such as pH and Dissolved Oxygen) are used, the colour provides a simple way to find the specific sensor in question<br/>

**Requirement #:** 5 <br/>
**Description:** The markers on the map representing the sensors shall have a tag popup when clicked on<br/>
**Rationale:** The tag popup will show more information regarding the sensor, including current air temperature and wind speed. Also,the tag will include a link to view more detailed information regarding the sensor such as the plot of the sensors' data<br/>

**Requirement #:** 6<br/>
**Description:** The product shall plot sensor data from database that is preconfigured<br/>
**Rationale:** The sensor will send their data to a specified database remotely, and this data should be able to be plotted<br/>

**Requirement #:** 7<br/>
**Description:** The product shall be able to set the time range on plot of sensor data. <br/>
**Rationale:** In general the plot created should have the sensor's measurement on the y-axis and time on the x-axis. Something that might be useful would be the ability to choose to look at the fine details or general trend of sensor data depending on the situation<br/>

**Requirement #:** 8 <br/>
**Description:** The product shall be able to graph multiple datasets of sensor measurement in the same plot<br/>
**Rationale:** Relationships can be found when different sensors of the same type of measurement located in different stations are graphed on the same plot.<br/>

**Requirement #:** 9 <br/>
**Description:** The plot shall show a tooltip containing the data point plotted<br/>
**Rationale:** Useful if the user wants to see the numerical value of the datapoint used in the plot<br/>

**Requirement #:** 10 <br/>
**Description:** The plot shall update as soon as the database remotely receives new data from sensor<br/>
**Rationale:** The data from sensor might want to be monitored in real-time<br/>

**Requirement #:** 11<br/>
**Description:**  The product shall show general information regarding sensor data obtained from the database,for example maximum or minimum pH value from sensor X for a specified time window<br/>
**Rationale:** Useful for quick analysis of the sensor data<br/>

**Requirement #:**  12 <br/>
**Description:** Sensor data which includes its location, measurement type, measurement value and timestamp shall be sent remotely to a specified database that is configured beforehand <br/>
**Rationale:** The database of our choosing might be more optimized for time series data, thus reducing response time when graphing the sensor data<br/>
