package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"strconv"
	"time"
	"strings"
	"math"
	"os"

	"github.com/gorilla/mux"
	"github.com/influxdata/influxdb1-client/v2"
)

// Default Api Key, unless os defined
const apiKey = "remote"

type ResultData struct {
	Id			interface{} `json:"id"`
	Type 		interface{} `json:"type"`
	Data		[][2]string `json:"data"`
}

type ResultInfo struct {
	Locations 	[][]string `json:"locations"`
}

type ResultPath struct {
	Name    	interface{} `json:"name"`
	Locations 	[][2]string `json:"locations"`
}

type ResultType struct {
	Types 		[]string `json:"types"`
}

// Display sensor info
func GetInfo(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Header().Set("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE")
	w.Header().Set("Access-Control-Allow-Headers", "Accept, Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization")
	//params := mux.Vars(r)
	v := r.URL.Query()

	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: "http://localhost:8086",
	})
	if err != nil {
	}
	defer c.Close()
	
	if (v.Get("id") == "" && v.Get("type") == "") {
		s := client.NewQuery("SHOW TAG values from sensorInfo with key=\"id\"", "MacWater", "ns")
		if response, err := c.Query(s); err == nil && response.Error() == nil {
			locationsArr := [][]string{}
			for i := 0; i < len(response.Results[0].Series[0].Values); i++ {
				sensorId := response.Results[0].Series[0].Values[i][1].(string)
				query := "SELECT LAST(value), \"name\" FROM sensorInfo WHERE id='" + sensorId + "'"
				q := client.NewQuery(query, "MacWater", "ns")

				if response, err := c.Query(q); err == nil && response.Error() == nil {
					sensorName := response.Results[0].Series[0].Values[0][2]
					sensorCoords := response.Results[0].Series[0].Values[0][1]
					sensorTime :=response.Results[0].Series[0].Values[0][0]
					currArr := []string{}
					currArr = append(currArr, sensorId)
					currArr = append(currArr, sensorName.(string))
					currArr = append(currArr, sensorCoords.(string))
					currArr = append(currArr, sensorTime.(json.Number).String())
					locationsArr = append(locationsArr, currArr)
				}
			}
			json.NewEncoder(w).Encode(&ResultInfo{
				Locations: locationsArr})
		}
	} else if (v.Get("id") == "") {
		s := client.NewQuery("SHOW SERIES FROM sensorMeasurements", "MacWater", "ns")
		if response, err := c.Query(s); err == nil && response.Error() == nil {
			locationsArr := [][]string{}
			for i := 0; i < len(response.Results[0].Series[0].Values); i++ {
				info := strings.Split(response.Results[0].Series[0].Values[i][0].(string), ",")
				sensorType := strings.Split(info[2], "=")[1]
				if (sensorType != v.Get("type")) {
					continue
				}
				sensorId := strings.Split(info[1], "=")[1]
				query := "SELECT LAST(value), \"name\" FROM sensorInfo WHERE id='" + sensorId + "'"
				q := client.NewQuery(query, "MacWater", "ns")

				if response, err := c.Query(q); err == nil && response.Error() == nil {
					sensorName := response.Results[0].Series[0].Values[0][2]
					sensorCoords := response.Results[0].Series[0].Values[0][1]
					sensorTime :=response.Results[0].Series[0].Values[0][0]
					currArr := []string{}
					currArr = append(currArr, sensorId)
					currArr = append(currArr, sensorName.(string))
					currArr = append(currArr, sensorCoords.(string))
					currArr = append(currArr, sensorTime.(json.Number).String())
					locationsArr = append(locationsArr, currArr)
				}
			}
			json.NewEncoder(w).Encode(&ResultInfo{
				Locations: locationsArr})
		}
	} else {
		query := "SELECT * FROM sensorInfo WHERE id='" + v.Get("id") + "'"
		q := client.NewQuery(query, "MacWater", "ns")
		if response, err := c.Query(q); err == nil && response.Error() == nil {
			numLocations := len(response.Results[0].Series[0].Values)
			sensorName := response.Results[0].Series[0].Values[numLocations-1][2]
			sensorLocations := make([][2]string, numLocations)
			for i := 0; i < numLocations; i++ {
				sensorLocations[i][0] = response.Results[0].Series[0].Values[i][0].(json.Number).String()
				sensorLocations[i][1] = response.Results[0].Series[0].Values[i][3].(string)
			}
			json.NewEncoder(w).Encode(&ResultPath{
				Name: sensorName,
				Locations: sensorLocations})
		}
	}
}

// Create new sensor information
func CreateInfo(w http.ResponseWriter, r *http.Request) {
	fmt.Println("CreateInfo")
	//params := mux.Vars(r)
	v := r.URL.Query()

	// Check apiKey
	key := v.Get("k") // apiKey
	k := apiKey
	val, ok := os.LookupEnv("apiKey")
	if ok {
		k=val
	}
	if key != k {
		fmt.Println("Unauthorized access to CreateInfo")
		return;
	}

	name := v.Get("name")
	coords := v.Get("coords")
	id := v.Get("id")
	currTimeStr := v.Get("time")
	
	currTime, err := strconv.ParseUint(currTimeStr, 10, 32)
	if err != nil {
		fmt.Println("Error: ", err.Error())
	}
	
	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: "http://localhost:8086",
	})
	if err != nil {
		fmt.Println("Error creating InfluxDB Client: ", err.Error())
	}
	defer c.Close()

	bp, _ := client.NewBatchPoints(client.BatchPointsConfig{
		Database:  "MacWater",
		Precision: "ms",
	})
	
	// Create a point and add to batch
	tags := map[string]string{"id": id,"name": name}
	fields := map[string]interface{}{
		"value": coords,
	}
	pt, err := client.NewPoint("sensorInfo", tags, fields, time.Unix(int64(currTime), 0))
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

// Display sensor type
func GetType(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Header().Set("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE")
	w.Header().Set("Access-Control-Allow-Headers", "Accept, Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization")
	//params := mux.Vars(r)
	v := r.URL.Query()

	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: "http://localhost:8086",
	})
	if err != nil {
	}
	defer c.Close()
	
	if (v.Get("id") == "") {
		s := client.NewQuery("SHOW TAG VALUES FROM sensorMeasurements WITH key=\"sensortype\"", "MacWater", "ns")
		if response, err := c.Query(s); err == nil && response.Error() == nil {
			typeArr := []string{}
			for i := 0; i < len(response.Results[0].Series[0].Values); i++ {
				typeArr = append(typeArr, response.Results[0].Series[0].Values[i][1].(string))
			}
			json.NewEncoder(w).Encode(&ResultType{
				Types: typeArr})
		}
	} else {
		query := "SHOW SERIES FROM sensorMeasurements"
		q := client.NewQuery(query, "MacWater", "ns")
		if response, err := c.Query(q); err == nil && response.Error() == nil {
			typeArr := []string{}
			for i := 0; i < len(response.Results[0].Series[0].Values); i++ {
				info := strings.Split(response.Results[0].Series[0].Values[i][0].(string), ",")
				sensorId := strings.Split(info[1], "=")[1]
				if (sensorId != v.Get("id")) {
					continue
				}
				typeArr = append(typeArr, strings.Split(info[2], "=")[1])
			}
			json.NewEncoder(w).Encode(&ResultType{
				Types: typeArr})
		}
	}
}

// Display data by time or data range
func GetData(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Header().Set("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE")
	w.Header().Set("Access-Control-Allow-Headers", "Accept, Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization")
	//params := mux.Vars(r)
	v := r.URL.Query()

	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: "http://localhost:8086",
	})
	if err != nil {
	}
	defer c.Close()
	if v.Get("start") == "" {
		q := client.NewQuery("SELECT * FROM sensorMeasurements WHERE id = '"+v.Get("id")+"' and sensortype = '"+v.Get("type")+"'", "MacWater", "ns")
		if response, err := c.Query(q); err == nil && response.Error() == nil {
			w.Header().Set("Content-Type", "application/json")
			sensorID := response.Results[0].Series[0].Values[0][1]
			sensorType := response.Results[0].Series[0].Values[0][2]
			dataLength := len(response.Results[0].Series[0].Values)
			sensorData := make([][2]string, dataLength)
			for i := 0; i < dataLength; i++ {
				sensorData[i][0] = response.Results[0].Series[0].Values[i][0].(json.Number).String()
				sensorData[i][1] = response.Results[0].Series[0].Values[i][3].(json.Number).String()
			}
			json.NewEncoder(w).Encode(&ResultData{Id: sensorID,
				Type: sensorType,
				Data:        sensorData})
		}
	}
	if v.Get("start") != "" {
		q := client.NewQuery("SELECT * FROM sensorMeasurements WHERE id = '"+v.Get("id")+"' and sensortype = '"+v.Get("type")+"' and time > "+v.Get("start")+" and time < "+v.Get("end"), "MacWater", "ns")
		if response, err := c.Query(q); err == nil && response.Error() == nil {
			w.Header().Set("Content-Type", "application/json")
			sensorID := response.Results[0].Series[0].Values[0][1]
			sensorType := response.Results[0].Series[0].Values[0][2]
			dataLength := len(response.Results[0].Series[0].Values)
			sensorData := make([][2]string, dataLength)
			for i := 0; i < dataLength; i++ {
				sensorData[i][0] = response.Results[0].Series[0].Values[i][0].(json.Number).String()
				sensorData[i][1] = response.Results[0].Series[0].Values[i][3].(json.Number).String()
			}
			json.NewEncoder(w).Encode(&ResultData{Id: sensorID,
				Type: sensorType,
				Data:        sensorData})
		}
	}
}

// Add new data to database
func CreateData(w http.ResponseWriter, r *http.Request) {
	fmt.Println("CreateData")
	v := r.URL.Query()

	// Check apiKey
	key := v.Get("k") // apiKey
	k := apiKey
	val, ok := os.LookupEnv("apiKey")
	if ok {
		k=val
	}
	if key != k {
		fmt.Println("Unauthorized access to CreateInfo")
		return;
	}
	
	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: "http://localhost:8086",
	})
	if err != nil {
		fmt.Println("Error creating InfluxDB Client: ", err.Error())
	}
	defer c.Close()
	
	bp, _ := client.NewBatchPoints(client.BatchPointsConfig{
		Database:  "MacWater",
		Precision: "ms",
	})
	
	b := v.Get("data")
	if (b != "") {
		numNodes, err := strconv.ParseUint(string(b[0:2]), 16, 0)
		if err != nil {
			fmt.Println("Error: ", err.Error())
		}

		currIndex := 2;
		for node := uint64(0); node < numNodes; node++ {
			id, err := strconv.ParseInt(string(b[currIndex+2:currIndex+4]) + string(b[currIndex:currIndex+2]), 16, 0)
			if err != nil {
				fmt.Println("Error: ", err.Error())
			}
			// fmt.Println("ID: ", id)
			currIndex += 4

			numLocations, err := strconv.ParseUint(string(b[currIndex:currIndex+2]), 16, 0)
			if err != nil {
				fmt.Println("Error: ", err.Error())
			}
			// fmt.Println("numLocations: ", numLocations)
			currIndex += 2

			var name string
			if (numLocations > 0) {
				nameLen, err := strconv.ParseUint(string(b[currIndex:currIndex+2]), 16, 0)
				if err != nil {
					fmt.Println("Error: ", err.Error())
				}
				// fmt.Println("nameLen: ", nameLen)
				currIndex += 2

				for i := uint64(0); i < nameLen; i++  {
					currInt, err := strconv.ParseInt(string(b[currIndex:currIndex+2]), 16, 0)
					if err != nil {
						fmt.Println("Error: ", err.Error())
					}
					// fmt.Println("currInt: ", currInt)
					currIndex += 2

					name += string(currInt)
				}
			}

			var locations []uint64
			currLocIndex := uint64(0)

			for i := uint64(0); i < numLocations; i++  {
				currLocation, err := strconv.ParseUint(string(b[currIndex:currIndex+2]), 16, 0)
				if err != nil {
					fmt.Println("Error: ", err.Error())
				}
				// fmt.Println("currLocation: ", currLocation)
				currIndex += 2

				locations = append(locations, currLocation);
			}

			numTypes, err := strconv.ParseUint(string(b[currIndex:currIndex+2]), 16, 0)
			if err != nil {
				fmt.Println("Error: ", err.Error())
			}
			// fmt.Println("numTypes: ", numTypes)
			currIndex += 2

			var types []string
			for i := uint64(0); i < numTypes; i++ {
				currLen, err := strconv.ParseUint(string(b[currIndex:currIndex+2]), 16, 0)
				if err != nil {
					fmt.Println("Error: ", err.Error())
				}
				currIndex += 2

				currType := ""
				for j := uint64(0); j < currLen; j++ {
					currInt, err := strconv.ParseInt(string(b[currIndex:currIndex+2]), 16, 0)
					if err != nil {
						fmt.Println("Error: ", err.Error())
					}
					// fmt.Println("currInt: ", currInt)
					currIndex += 2

					currType += string(currInt)
				}

				types = append(types, currType)
			}

			numData, err := strconv.ParseUint(string(b[currIndex:currIndex+2]), 16, 0)
			if err != nil {
				fmt.Println("Error: ", err.Error())
			}
			// fmt.Println("numData: ", numData)
			currIndex += 2

			for i := uint64(0); i < numData; i++ {
				currTimeStr := string(b[currIndex+6:currIndex+8]) + string(b[currIndex+4:currIndex+6]) + string(b[currIndex+2:currIndex+4]) + string(b[currIndex:currIndex+2])
				currTime, err := strconv.ParseUint(currTimeStr, 16, 32)
				if err != nil {
					fmt.Println("Error: ", err.Error())
				}
				// fmt.Println("currTime: ", currTime)
				currIndex += 8

				if ((currLocIndex < numLocations) && (i + 1 == locations[currLocIndex])) {
					currLocIndex++

					currLatStr := string(b[currIndex+6:currIndex+8]) + string(b[currIndex+4:currIndex+6]) + string(b[currIndex+2:currIndex+4]) + string(b[currIndex:currIndex+2])
					currLatInt, err := strconv.ParseUint(currLatStr, 16, 32)
					if err != nil {
						fmt.Println("Error: ", err.Error())
					}
					// fmt.Println("currLatInt: ", currLatInt)
					currIndex += 8

					currLat := math.Float32frombits(uint32(currLatInt))


					currLonStr := string(b[currIndex+6:currIndex+8]) + string(b[currIndex+4:currIndex+6]) + string(b[currIndex+2:currIndex+4]) + string(b[currIndex:currIndex+2])
					currLonInt, err := strconv.ParseUint(currLonStr, 16, 32)
					if err != nil {
						fmt.Println("Error: ", err.Error())
					}
					// fmt.Println("currLonInt: ", currLonInt)
					currIndex += 8

					currLon := math.Float32frombits(uint32(currLonInt))

					currLoc := fmt.Sprintf("%f", currLat) + "," + fmt.Sprintf("%f", currLon)

					tags := map[string]string{"id": strconv.Itoa(int(id)), "name": name}
					fields := map[string]interface{}{
						"value": currLoc,
					}

					// fmt.Println("CheckPoint sensorInfo")
					// fmt.Println(id)
					// fmt.Println(name)
					// fmt.Println(currLoc)

					pt, err := client.NewPoint("sensorInfo", tags, fields, time.Unix(int64(currTime), 0))
					if err != nil {
						fmt.Println("Error: ", err.Error())
					}
					bp.AddPoint(pt)
				}

				for j := uint64(0); j < numTypes; j++ {
					// fmt.Println("CheckPoint 0")
					currDataStr := string(b[currIndex+6:currIndex+8]) + string(b[currIndex+4:currIndex+6]) + string(b[currIndex+2:currIndex+4]) + string(b[currIndex:currIndex+2])
					currDataInt, err := strconv.ParseUint(currDataStr, 16, 32)
					if err != nil {
						fmt.Println("Error: ", err.Error())
					}
					// fmt.Println("currDataInt: ", currDataInt)
					currIndex += 8

					currData := math.Float32frombits(uint32(currDataInt))

					// Create a point and add to batch
					tags := map[string]string{"id": strconv.Itoa(int(id)), "sensortype": types[j]}
					fields := map[string]interface{}{
						"value": currData,
					}

					// fmt.Println("CheckPoint sensorMeasurements")
					// fmt.Println(id)
					// fmt.Println(types[j])
					// fmt.Println(currData)

					pt, err := client.NewPoint("sensorMeasurements", tags, fields, time.Unix(int64(currTime), 0))
					if err != nil {
						fmt.Println("Error: ", err.Error())
					}
					bp.AddPoint(pt)
				}
			}
		}
	} else {
		id := v.Get("id")
		sensorType := v.Get("type")
		
		value, err := strconv.ParseFloat(v.Get("value"), 32)
		if err != nil {
			fmt.Println("Error: ", err.Error())
		}
		
		Time, err := strconv.ParseUint(v.Get("time"), 10, 32)
		if err != nil {
			fmt.Println("Error: ", err.Error())
		}
		
		tags := map[string]string{"id": id, "sensortype": sensorType}
		fields := map[string]interface{}{
			"value": value,
		}
		
		pt, err := client.NewPoint("sensorMeasurements", tags, fields, time.Unix(int64(Time), 0))
		if err != nil {
			fmt.Println("Error: ", err.Error())
		}
		
		bp.AddPoint(pt)
	}
	
	err = c.Write(bp)
	if err != nil {
		fmt.Println("Error: ", err.Error())
	}
}

// Main function to boot up everything
func main() {
	router := mux.NewRouter()
	router.HandleFunc("/sensor/info", GetInfo).Methods("GET")
	router.HandleFunc("/sensor/info", CreateInfo).Methods("Post")
	router.HandleFunc("/sensor/type", GetType).Methods("GET")
	router.HandleFunc("/sensor/data", GetData).Methods("GET")
	router.HandleFunc("/sensor/data", CreateData).Methods("POST")
	fmt.Printf("MacWater server started ...\n")
	k := apiKey
	val, ok := os.LookupEnv("apiKey")
	if ok {
		k=val
	}
	fmt.Println("apiKey: ", k)
	router.PathPrefix("").Handler(http.FileServer(http.Dir("../frontend/dist")))
	log.Fatal(http.ListenAndServe(":8080", router))
}
