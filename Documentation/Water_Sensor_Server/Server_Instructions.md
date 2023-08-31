# **How to access RaspberryPi (RPi) on windows OS?** 


If you already know the IP address/hostname of your RPi, you can login by following the steps below (otherwise start from the RPi setup - given after step 3's completion):
The following steps are for SSH connection:

1. Download and install PuTTY software using: <https://www.putty.org/> (or a quick google search will show other websites too)

 

2. Open PuTTY software: 

In the ‘*Host Name (or IP address)*’ section, 

 

use the IP address of the website (e.g. ###.###.##.###) – without port number 

*or* 

use the hostname and hit ‘Enter’.

 

If prompted, say ‘Yes’ to the ‘PuTTY Security Alert’

 

3. This will open a command prompt, asking you to login:

 

Login as: <u>user</u>

user@ip_address’s password: <u>password</u>

 

NOTE: Both <u>user</u> and <u>password</u> details are to be filled in by the user

 

Congrats! You have successfully logged into the RaspberryPi. Now, you can communicate through an SSH connection to your RaspberryPi from the windows desktop.



# How to setup RaspberryPi as a webserver using Windows Computer?

 
First, let's install an operating system on RPi:
<br />
To begin, you will need the following to complete this process:

\-          RaspberryPi 

\-          MicroSD Card (and an adapter to plug it into your computer)

\-          Micro-USB Power Cable

\-          RJ45 Ethernet Cable (Standard Ethernet Cable)

Software Requirement:

\-          Download ‘Raspbian Stretch with desktop and recommended software’ using: <https://www.raspberrypi.org/downloads/raspbian/> (Use either Torrent or ZIP option)

\-          Download and install the recommended extraction software ‘7-zip’ to extract the image file from the ZIP version of Raspbian, using: <https://www.7-zip.org/download.html> 

(NOTE: This software may not be needed as windows has a standard extraction tool)

\-          Download and install SSH client using: <https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html>

\-          Download and install Win32 Disc Imager using: 					<https://sourceforge.net/projects/win32diskimager/files/latest/download>




<br /><br /><br /> 



<u>Option #1</u>: SSH connection with RaspberryPi (Headless Method - without the need of a keyboard and a mouse attached to the RaspberryPi)



**STEP 1**: Right-click on the Raspbian Operating System file (zip file), and using the 7-zip tool extract the file in a folder

**STEP 2**: Insert a Micro-SD card into your computer

**STEP 3**: Open Win32DiskImager:

a.       Under the Image File section, hit the folder button to navigate and select the extracted image file (extracted file in step 1)

b.       Under the Device column, select Micro-SD card drive

c.       Hit ‘Write’ and press ‘Yes’ on the next prompt to continue

Once completed, select OK and Exit the Win32DiskImager

 

Checkpoint: Now, the SD card is called ‘boot’

**STEP 4**:  To remotely access the RaspberryPi, we need to enable SSH:

a.       Inside the ‘boot (drive:)’ (SD card) folder, create a Notepad (or any other text editor) file and leave it empty. Save the file name as: “ssh” and Save as type: ‘All Files’.

NOTE: It is important save the file name with the quotation mark, and include the quotation marks in File name. 

​	Checkpoint: Inside the boot folder, you should see a file, named ‘ssh’, with no extension  

**STEP 5**: Eject the SD card, and insert it into your RaspberryPi

**STEP 6**: Connect an ethernet cable to RaspberryPi and lastly, plug in the power cable as well.

Once plugged in, LED lights are powered on RaspberryPi, and allow it to boot-up for about 1-2 minutes.

**STEP 7**: Open PuTTY software to access the RaspberryPi terminal – to make a secure connection you’ll need to know either hostname (which is set to ‘raspberrypi.local’ by default, if you have re-flashed your RaspberryPi’s operating system) or IP address of the RaspberryPi 

a.       On PuTTY configuration dialog box, under Host Name (or IP address) enter your hostname or IP address, and under Port, enter 22 (for SSH connection)

b.       Hit Open, and a command prompt will show up

c.       Inside the command prompt, enter ‘login as:’ and hit enter

d.       When prompted for password, enter the password and hit enter.

 

The default username is ‘pi’ and the password is ‘raspberry’. It is recommended that the user should change the password.

Congrats!!, you have developed an SSH connection with the RaspberryPi.


<br /><br /><br /> 


<u>Option #2</u>: Remote Desktop RaspberryPi (aka VNC)

This method will help you view the GUI interface of RaspberryPi on your laptop/PC by accessing the desktop screen of your RaspberryPi remotely

Software Requirement: 

\-          Download and install VNC Viewer using: <https://www.realvnc.com/en/connect/download/viewer/>

\-          You’ll need to know IP address of your RaspberryPi to access the desktop

\-          Connect your RaspberryPi to the internet and supply power to it.

 

**STEPS:**

**a.**       First, build an SSH connection with your RaspberryPi as mentioned in the previous method (using PuTTY software)

**b.**      Once logged into your RaspberryPi, in the command prompt, type ‘sudo raspi-config’ and hit enter

**c.**       Under the Raspberry Pi Software Configuration Tool (raspi-config), look for VNC Options (this could be under ‘advanced options’ or ‘interfacing options’ depending upon model of the RaspberryPi)

**d.**      Navigate to the VNC option and hit enter to enable it. (You may have to hit enter a couple of times) 

**e.**       Finally, exit the raspi-config menu through ‘Finish’ option. After this, you will be brought back to the command prompt again

**f.**        Now, reboot your RPi by typing ‘sudo reboot’ in the command prompt.

**g.**       Wait for it to reboot again, give it 1 -2 mins

**h.**      After a minute or two, open VNC viewer software and enter the hostname or ip address of your RaspberryPi

**i.**        This will log you into your RPi and the desktop screen will be displayed.







## Transfer files between your computer and RaspberryPi – in order to mirror an existing webserver

 

Requirement:

\-          Download and install FileZilla Client software using: <https://filezilla-project.org/>

 

Use the interface of this software to transfer files between your computer and RPi.





## Start InfluxDB on RPi:

 

**STEP 1**: In the command terminal of RPi, execute the following commands:

```sh
$ wget <https://dl.influxdata.com/influxdb/releases/influxdb-1.5.3_linux_amd64.tar.gz>

$ tar xvfz influxdb-1.5.3_linux_amd64.tar.gz	
```

NOTE: DO NOT INCLUDE $ SYMBOLS



***OR* STEP 1**: Install InfluxDB on RPi using ‘sudo apt-get install influxdb’ command in the RPi terminal



**STEP 2**: Execute the influxd file (on the terminal, type ‘influxd’) – This step helps make connection with the database. Leave this terminal running for as long as you want to stay connected with the database

**STEP 3**: Open a new terminal, run influx file (in the new terminal, type ‘influx’)

 

### TROUBLESHOOTING:

 

·         If you are unable to access the influx database, try adding repository keys by following the commands below (make sure to disconnect with the influx database)

```sh
$ curl -aL https://repos.influxdata.com/influxdb.key | sudo apt-key add –
$ echo "deb https://repos.influxdata.com/debian stretch stable" | sudo tee /etc/apt/sources.list.d/influxdb.list
$ sudo apt-get update
$ sudo apt-get install telegraf
$ sudo apt-get --reinstall install influxdb
$ sudo influxd
$ sudo influx
```







## Install Go Language on RPi:

 

In the RPi terminal, type the commands:

```sh
$ sudo apt-get install golang	
$ go build main.go
```



**ERROR? If upon executing ‘main.go’ file inside ‘webserver’ folder a gorilla/mux dependency missing error is shown, follow the next steps to fix the issue**:

<u>*Add GOPATH to the sudo root by following the commands*:</u>

```sh
$ mkdir ~/go
$ sudo bash
```

```shell
root@raspberrypi:/home/pi# env
root@raspberrypi:/home/pi# source ~/.bashrc
```



Last command will open .bashrc file, and inside this file, add the following two lines at the end of the file:

```powershell
export GOPATH=~/go

export PATH=$PATH:$GOPATH/bin
```

<u>Save and exit the file.</u>



```shell
root@raspberrypi:/home/pi# echo $GOPATH  
root@raspberrypi:/home/pi# go get -u github.com/gorilla/mux
root@raspberrypi:/home/pi# exit
```



At user@raspberrypi: terminal: -

\-          Navigate to the main.go file inside the webserver folder, and run the file:

```sh
$ go build main.go
```



**ERROR? Main.go: cannot find package "github.com/influxdata/influxdb1-client/v2", apply the following changes:**

```sh
~/Desktop/pi-server/webserver $ sudo bash

```

```shell
root@raspberrypi:/home/pi/Desktop/pi-server/webserver# source ~/.bashrc
root@raspberrypi:/home/pi/Desktop/pi-server/webserver# go get -u github.com/influxdata/influxdb1-client/v2
root@raspberrypi:/home/pi/Desktop/pi-server/webserver# go build main.go
root@raspberrypi:/home/pi/Desktop/pi-server/webserver# ./main

```







## Add data to InfluxDB:

 

Convenient method to add data to the database is using Postman software. Download and install Postman software (available for Linux, Windows and Mac).

Open Postman software and on builder page, select POST option from the drop-down menu. 

 

*DATA 1*:

Enter request URL: Fill in this section using syntax given on the next line:

http://localhost:###/sensor/info?id=32&name=first&coords=43.2585,-79.9201

Hit ‘Send’           (*NOTE: ### represents router number allocated in the main.go file for local development*)





*DATA 2*:

<http://localhost:###/sensor/data?id=32&measurement=Temperature&value=25>

Hit ‘Send’

Add more data points by changing the value listed at the end of the URL. For example, modify 25 to 5.

Now, whenever you insert a new value to the database, it can be viewed on the website just by refreshing the page.

