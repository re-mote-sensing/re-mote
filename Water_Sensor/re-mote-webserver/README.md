# re:mote Server Setup Instructions

Docker is recommended to setup the re:mote server, which allows you to deploy the application quickly.

A **Docker container image** is a lightweight, standalone, executable package of software that includes everything needed to run an application: code, runtime, system tools, system libraries, and settings.

## Run a prebuilt Docker image (recommended)

This prebuild Docker image contains the re:mote server program and influxdb.

1. Install Docker (Windows, Linux, MacOS) [Download Docker](https://www.docker.com/get-started/).
2. Pull the Docker image from the [Docker Hub](https://hub.docker.com/r/remotesensing/macwater-server/) using command line ([Command line crash course](https://developer.mozilla.org/en-US/docs/Learn/Tools_and_testing/Understanding_client-side_tools/Command_line)).
	 ```
	 docker pull remotesensing/macwater-server:latest
	 ```
	<img src="/Documentation/Pictures/server/docker_pull.png" width="50%">
3. Run the Docker Container.
	- In the command below, Change `/DATA/AppData/remote-server` to desired location, for example if you want to store it on your desktop.
		- Windows: `C:/Users/username/Desktop/remote-server`
		- Linux: `/home/username/Desktop/remote-server`
		- MacOS: `/Users/username/Desktop/remote-server`
	```
	docker run \
		--volume=/DATA/AppData/remote-server:/var/lib/influxdb \
		-p 8080:8080 \
		-e apiKey="remote" \
		-d remotesensing/macwater-server:latest
	```
	<img src="/Documentation/Pictures/server/docker_run.png" width="50%">
4. (Optional) Use reverse proxy to setup SSL and expose to public address (Ex. Nginx Proxy Manager).

5. You are all set! If you have docker desktop, you can see it in the GUI interface.
	<img src="/Documentation/Pictures/server/docker_desktop.png" width="50%">

6. Using your IP address to view your website.
	<img src="/Documentation/Pictures/server/website.gif" width="50%">

---

# Using the website

To use your website, enter either the IP address of your personal server into a web browser (for your own server), or go to the website that hosts your website (for a cloud service). You may need to add a /# after the IP/URL. Once there, you should be able to see a map with the locations of all the nodes you have setup. To select a node, click on its pin on the map or select it from the Location drop-down menu. You can then select the measurement for which you wish to see the data in the Sensor Type drop-down menu. Sensor Types in black are available at that node, and Sensor Types in gray are not.

To see a list of nodes that contain a certain measurement, first unselect a location from the Location dropdown menu, then select the measurement you wish to use. When you expand the Location drop-down menu, the locations in black contain that measurement, and the locations in gray do not. You may also notice that when you have a location selected, it will create a path with circles on the map. Each circle represents GPS coordinates where that node was located.

Once both a Location and Sensor Type have been selected, a table containing the date, time, and value of every data point will appear under the map. Underneath the table, you will see a graph representing the sensor data through time.

---

## Files under this folder

### /backend

- A server programmed in Go for fast and concurrent connections.

### /frontend

- A progressive web application designed in React.

### Dockerfile

- Contains all the commands to assemble an image.

### run.sh

- The entry point of the container.

- It starts influxdb and creates a database "MacWater" and then starts the go server.

---

## Advanced setup

<details>
<summary>Run a custom build Docker image (If you have to change the source code)</summary>
<br>

1. Install Docker (Windows, Linux, MacOS) [Download Docker](https://www.docker.com/get-started/).
2. Download the [source code](/macwater-webserver).
2. Open the command line and navigate to this folder.
	```
	cd macwater-webserver
	```
3. Build the Container.
	 ```
	 docker build --tag remote-server .
	 ```
4. Run the Docker Container.
	- In the command below, Change `/DATA/AppData/remote-server` to desired location, for example if you want to store it on your desktop.
		- Windows: `C:/Users/username/Desktop/remote-server`
		- Linux: `/home/username/Desktop/remote-server`
		- MacOS: `/Users/username/Desktop/remote-server`
	```
	docker run \
		--volume=/DATA/AppData/remote-server:/var/lib/influxdb \
		-p 8080:8080 \
		-e apiKey="remote" \
		-d remote-server:latest
	```

</details>

---

<details>
<summary>Run without Docker (If you do not have virtualization on your machine)</summary>
<br>

- See [Server Build Instructions](/Documentation/Water_Sensor_Server/Server_Instructions.md) for instruction on how to setup the server without Docker.

</details>