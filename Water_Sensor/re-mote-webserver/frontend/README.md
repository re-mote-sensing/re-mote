First install the required dependencies with `yarn install`.

Then `yarn build` to build the frontend in the `dist/` folder.

For local development use `yarn gulp static` when changes to the static assets are made and `yarn bundle` to build the development version of the `client.js`. Open the `dist/index.html` file in a browser.

# For Local Developmemt of the website, follow steps given below:

1.	Get latest clone of the webserver
2.	Go to influx folder and run influxd.exe file (this will result in a terminal containing INFLUXDB name on the top, followed by http commands but the page will not bring the prompt back again)
3.	Go to influx folder and run influx.exe file (upon executing, a new terminal will open and show the message “Connected to http:localhost:….	InfluxDB shell version:….	Enter an InfluxQL query 	>”
4.	Use a text editor and open file main.go inside the webserver folder and change the log.Fatal(http.ListenAndServe(“…”, router)) URL on the second last line of the file, i.e:
Replace the URL ("130.113.68.111:8000”) with “:5000”
5.	Now, using a new terminal, navigate to the cloned folder and inside the ‘webserver’ folder, type the following command in the terminal to execute main.go file:
``` sh
$ go run main.go
```
This will show you a message “Starting server for testing HTTP GET POST” and hit yes/accept if prompted to allow access
6.	Now, using a new terminal, navigate into the ‘react-sensor’ folder, type the following commands:
``` sh
$ yarn install

$ yarn build
```
Once completed, you will be shown time it took to build the package and a finished status (You should eliminate all the errors before proceeding to the next step. After fixing the errors in respective files, re-run the yarn build command again)
7.	Open a web browser and type localhost:5000 as the URL and hit enter. You should be able to access the website and see the changes you have made.
