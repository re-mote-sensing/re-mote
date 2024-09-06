# re:mote Server Setup Instructions

Docker is recommended to setup the re:mote server, which allows you to deploy the application quickly.

A **Docker container image** is a lightweight, standalone, executable package of software that includes everything needed to run an application: code, runtime, system tools, system libraries, and settings.

## Run a prebuilt Docker image (recommended)

This prebuild Docker image contains the re:mote server program and influxdb.

1. Install Docker (Windows, Linux, MacOS) [Download Docker](https://www.docker.com/get-started/).

2. Pull the Docker image from the [Docker Hub](https://hub.docker.com/r/remotesensing/re-mote-server/) using command line ([Command line crash course](https://developer.mozilla.org/en-US/docs/Learn/Tools_and_testing/Understanding_client-side_tools/Command_line)).
	 ```
	 docker pull remotesensing/re-mote-server:latest
	 ```

	<img src="/Documentation/Pictures/server/docker_pull.png" width="50%">

3. Run the Docker Container.
	- In the command below, Change `/DATA/AppData/remote-server` to desired location, for example if you want to store it on your desktop.
		- Windows: `C:/Users/{username}/Desktop/remote-server`
		- Linux: `/home/{username}/Desktop/remote-server`
		- MacOS: `/Users/{username}/Desktop/remote-server`
	```
	docker run \
		--volume=/DATA/AppData/remote-server:/var/lib/influxdb \
		-p 8080:8080 \
		-e apiKey="remote" \
		-d remotesensing/re-mote-server:latest
	```

	<img src="/Documentation/Pictures/server/docker_run.png" width="50%">

4. (Optional) Use reverse proxy to setup SSL and expose to public address (Ex. Nginx Proxy Manager).

5. You are all set! If you have docker desktop, you can see it in the GUI interface, docker desktop can view all instances and easier to maintain or restart after shutdown.

	<img src="/Documentation/Pictures/server/docker_desktop.png" width="50%">

6. Using your IP address to view your website.
	- Find local IP address
		- Windows: `ipconfig`
		- Linux/MacOS: `ifconfig`

	<img src="/Documentation/Pictures/server/website.gif" width="50%">

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
2. Download the [source code](/re-mote-webserver).
2. Open the command line and navigate to this folder.
	```
	cd re-mote-webserver
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

<br><br><br><br>