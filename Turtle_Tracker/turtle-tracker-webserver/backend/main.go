package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/gorilla/mux"
	"github.com/r3labs/sse/v2"

	_ "github.com/influxdata/influxdb1-client" // this is important because of the bug in go mod
	client "github.com/influxdata/influxdb1-client/v2"
)

const DATA_LEN = 38

// Save data to database
// func CreateInfo(id int, lat float64, lon float64, currTime int64) {

// 	//currTime := uint64(currTimeStr)
	
// 	c, err := client.NewHTTPClient(client.HTTPConfig{
// 		Addr: "http://localhost:8086",
// 	})
// 	if err != nil {
// 		fmt.Println("Error creating InfluxDB Client: ", err.Error())
// 	}
// 	defer c.Close()

// 	bp, _ := client.NewBatchPoints(client.BatchPointsConfig{
// 		Database:  "MacTurtle",
// 		Precision: "ms",
// 	})
	
// 	// Create a point and add to batch
// 	tags := map[string]string{"id": strconv.Itoa(id)}
// 	fields := map[string]interface{}{
// 		"value": strconv.FormatFloat(lat, 'E', -1, 64)+","+strconv.FormatFloat(lon, 'E', -1, 64),
// 	}
// 	pt, err := client.NewPoint("turtleInfo", tags, fields, time.Unix(currTime, 0))
// 	if err != nil {
// 		fmt.Println("Error: ", err.Error())
// 	}
// 	bp.AddPoint(pt)

// 	// Write the batch
// 	err = c.Write(bp)
// 	if err != nil {
// 		fmt.Println("Error: ", err.Error())
// 	}
// }

func CreateInfo2(id int, lat int64, lon int64, currTime int64, count int64, battery int64, tempurature int64) {

	//currTime := uint64(currTimeStr)

	currTimeTemp := currTime
	if (currTime == 0){
		currTimeTemp = time.Now().Unix()
	}
	
	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: "http://localhost:8086",
	})
	if err != nil {
		fmt.Println("Error creating InfluxDB Client: ", err.Error())
	}
	defer c.Close()

	bp, _ := client.NewBatchPoints(client.BatchPointsConfig{
		Database:  "MacTurtle",
		Precision: "ms",
	})
	
	// Create a point and add to batch
	tags := map[string]string{"id": strconv.Itoa(id)}
	fields := map[string]interface{}{
		"z1value": strconv.FormatInt(lat, 10) + "," + strconv.FormatInt(lon, 10),
		"z2count": strconv.FormatInt(count, 10),
		"z3battery": strconv.FormatInt(battery, 10),
		"z4tempurature": strconv.FormatInt(tempurature, 10),
	}
	pt, err := client.NewPoint("turtleInfo", tags, fields, time.Unix(currTimeTemp, 0))
	if err != nil {
		fmt.Println("Error: ", err.Error())
	}
	bp.AddPoint(pt)

	// Write the batch
	err = c.Write(bp)
	if err != nil {
		fmt.Println("Error: ", err.Error())
	}
}

// Get data from database into sse
func GetInfo(server *sse.Server) {

	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: "http://localhost:8086",
	})
	if err != nil {
	}
	defer c.Close()

	// Add a data with id = -1
	// Solve empty database problem
	bp, _ := client.NewBatchPoints(client.BatchPointsConfig{
		Database:  "MacTurtle",
		Precision: "ms",
	})
	tags := map[string]string{"id": "-1"}
	fields := map[string]interface{}{
		"z1value": "0,0",
		"z2count": "0",
		"z3battery": "0",
		"z4tempurature": "0",
	}
	pt, err := client.NewPoint("turtleInfo", tags, fields, time.Unix(int64(18000), 0))
	if err != nil {
		fmt.Println("Error: ", err.Error())
	}
	bp.AddPoint(pt)
	err = c.Write(bp)
	if err != nil {
		fmt.Println("Error: ", err.Error())
	}

	query := "SELECT * FROM turtleInfo"
	q := client.NewQuery(query, "MacTurtle", "ns")
	if response, err := c.Query(q); err == nil && response.Error() == nil {

		numLocations := len(response.Results[0].Series[0].Values)
		if numLocations <= 1{
			if fmt.Sprintf("%v", response.Results[0].Series[0].Values[numLocations-1][1]) == "-1"{
				fmt.Println("No Data from database")
				return
			}
		}
		for i := 0; i < numLocations; i++ {

			// &{[{0 [{turtleInfo map[] [time 1value 2count 3battery 4tempurature id] [[18000000000000 0,0 0 0 0 -1]] false}] [] }] }
			// fmt.Println(response)

			t := fmt.Sprintf("%v", response.Results[0].Series[0].Values[i][0])
			timestamp, err := strconv.ParseInt(t[:len(t)-9], 10, 64)
			if err != nil {
			    panic(err)
			}
			id := fmt.Sprintf("%v", response.Results[0].Series[0].Values[i][1])
			if id == "-1" {
				continue
			}
			loc := fmt.Sprintf("%v", response.Results[0].Series[0].Values[i][2])
			lat := strings.Split(loc, ",")[0]
			lon := strings.Split(loc, ",")[1]

			count := fmt.Sprintf("%v", response.Results[0].Series[0].Values[i][3])

			battery := fmt.Sprintf("%v", response.Results[0].Series[0].Values[i][4])
			temperature := fmt.Sprintf("%v", response.Results[0].Series[0].Values[i][5])

			eventMessage := fmt.Sprintf(`{"id":%v, "time":"%v", "lat":%v, "lon":%v, "count":%v, "battery":%v, "temperature":%v}`, id, time.Unix(timestamp, 0).UTC(), lat, lon, count, battery, temperature)
			server.Publish("messages", &sse.Event{
				Data: []byte(eventMessage),
			})
		}
		fmt.Println("Data read from database")
	}
}

// POST request endpoint for communication between the 3G Gateway and the backend
func postHandler(server *sse.Server, w http.ResponseWriter, r *http.Request, l *log.Logger) {
	// w.Header().Set("Access-Control-Allow-Origin", "*")
	// w.Header().Set("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE")
	// w.Header().Set("Access-Control-Allow-Headers", "Accept, Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization")

	r.ParseForm() // Creates a data structure with the contents of the POST request
	if r.Method == "POST" {

		v := r.URL.Query()
		b := v.Get("b")
		if (b != "") {
			if (len(b) % DATA_LEN == 0) {
				fmt.Fprintf(w, "Message Received Successfully")

				n := len(b) / DATA_LEN
				for i := 0; i < n; i++ {
					// shift the array slice by offset
					offset := DATA_LEN * i
			
					// read hex and covert hex to int
					id, _ := strconv.ParseInt(b[2+offset:4+offset], 16, 64)
					count, _ := strconv.ParseInt(b[4+offset:8+offset], 16, 64)
					timestamp, _ := strconv.ParseInt(b[12+offset:20+offset], 16, 64)

					battery, _ := strconv.ParseInt("0", 16, 64)
					temperature, _ := strconv.ParseInt("0", 16, 64)
					
					lat := parseStr32ToInt64(b[20+offset:28+offset])
					lon := parseStr32ToInt64(b[28+offset:36+offset])
					
					// test
					fmt.Print("id: ")
					fmt.Println(id)
					fmt.Print("serial number: ")
					fmt.Println(count)
					fmt.Print("timestamp: ")
					fmt.Println(timestamp)
					fmt.Print("lat: ")
					fmt.Println(lat)
					fmt.Print("lon: ")
					fmt.Println(lon)
	
					eventMessage := fmt.Sprintf(`{"id":%v, "time":"%v", "lat":%v, "lon":%v, "count":%v, "battery":%v, "temperature":%v}`, id, time.Unix(timestamp, 0).UTC(), lat, lon, count, battery, temperature)
					l.Println(eventMessage)
					server.Publish("messages", &sse.Event{
						Data: []byte(eventMessage),
					})
					CreateInfo2(int(id), lat, lon, timestamp, count, battery, temperature)
					l.Printf("Message #%v from %v: %v\n", count, id, b)
				}
			}else {
				l.Printf("Invalid Message Received")
				fmt.Fprintf(w, "Invalid Message Received")
			}
			return
		}

		// Extract the data from the POST request form
		id, _ := strconv.Atoi(r.PostForm.Get("id"))
		count, _ := strconv.Atoi(r.PostForm.Get("count"))
		msg := r.PostForm.Get("msg")

		tokens := strings.Split(msg, ",")

		if len(tokens) == 3 {
			l.Printf("Message #%v from %v: %v\n", count, id, msg)
			fmt.Fprintf(w, "Message Received Successfully")

			timestamp, _ := strconv.ParseInt(tokens[0], 10, 64)

			lat, _ := strconv.ParseFloat(tokens[1], 64)
			lon, _ := strconv.ParseFloat(tokens[2], 64)

			lat /= 10e7
			lon /= 10e7

			lat *= 10
			lon *= 10

			eventMessage := fmt.Sprintf(`{"id":%v, "time":"%v", "lat":%v, "lon":%v}`, id, time.Unix(timestamp, 0).UTC(), lat, lon)
			l.Println(eventMessage)
			server.Publish("messages", &sse.Event{
				Data: []byte(eventMessage),
			})
			//CreateInfo(id, lat, lon, timestamp)
		} else {
			l.Printf("Invalid Message Received")
			fmt.Fprintf(w, "Invalid Message Received")
		}
	} else {
		w.WriteHeader(http.StatusMethodNotAllowed)
		fmt.Fprintf(w, "Wrong Method")
	}
}

// covert an 32-bit 2s complement hex (String) to signed int (int64)
// example:
// input1 := "d05d0f3c"
// expect1 := -799207620
// input2 := "19c8b047"
// expect2 := 432582727
func parseStr32ToInt64(input string) int64 {
	dec, _ := strconv.ParseInt(input, 16, 64)
	if dec > 2147483647 {
		return int64(-(4294967295 - dec + 1))
	} else {
		return int64(dec)
	}
}

func main() {
	f, err := os.OpenFile("output.log",
		os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		log.Println(err)
	}
	defer f.Close()

	logger := log.New(f, "", log.LstdFlags)

	fmt.Println("Starting Server")
	logger.Println("Starting Server")

	// Initialize SSE server instance
	server := sse.New()

	// Create SSE channel
	server.CreateStream("messages")

	// Initialize a new Router and add the necessary endpoints
	router := mux.NewRouter()

	router.HandleFunc("/data", func(w http.ResponseWriter, r *http.Request) {
		postHandler(server, w, r, logger)
	}).Methods("POST")
	router.HandleFunc("/events", server.HTTPHandler)

	router.PathPrefix("").Handler(http.FileServer(http.Dir("../frontend/build")))

	// Get data from database and publish into SSE
	GetInfo(server)

	// Start the server
	log.Fatal(http.ListenAndServe(":80", router))
}
