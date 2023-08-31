package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"

	"github.com/gorilla/mux"
)

type Result struct {
	Id          interface{} `json:"id"`
	Measurement interface{} `json:"measurement"`
	Data        [][2]string `json:"data"`
}

// Display a single data by time
func GetData(w http.ResponseWriter, r *http.Request) {
	//http://localhost:9090/api/v1/query?query=push_time_seconds{exported_job=%22some_job2%22}

	v := r.URL.Query()
	resp, err := http.Get("http://localhost:9090/api/v1/query?query=push_time_seconds{exported_job=%22" + v.Get("id") + "%22,measurement=%22" + v.Get("measurement") + "%22}")
	if err != nil {
		// handle error
	}
	defer resp.Body.Close()
	contents, err := ioutil.ReadAll(resp.Body)
	// fmt.Printf("%s\n", string(contents))
	w.Header().Set("Content-Type", "application/json")
	var data map[string]interface{}
	json.Unmarshal(contents, &data)
	sensorID := data["data"].(map[string]interface{})["result"].([]interface{})[0].(map[string]interface{})["metric"].(map[string]interface{})["exported_job"]
	sensorMeasurement := data["data"].(map[string]interface{})["result"].([]interface{})[0].(map[string]interface{})["metric"].(map[string]interface{})["measurement"]
	dataLength := len(data["data"].(map[string]interface{})["result"].([]interface{}))
	//testdata := data["data"].(map[string]interface{})["result"].([]interface{})[0].(map[string]interface{})["value"].([]interface{})[1].(string)
	sensorData := make([][2]string, dataLength)
	for i := 0; i < dataLength; i++ {
		sensorData[i][0] = data["data"].(map[string]interface{})["result"].([]interface{})[i].(map[string]interface{})["value"].([]interface{})[1].(string)
		sensorData[i][1] = data["data"].(map[string]interface{})["result"].([]interface{})[i].(map[string]interface{})["metric"].(map[string]interface{})["value"].(string)
	}

	json.NewEncoder(w).Encode(&Result{Id: sensorID, Measurement: sensorMeasurement, Data: sensorData})
	fmt.Println(data["data"])
	// w.Write(contents)
}

// create a new item
func CreateData(w http.ResponseWriter, r *http.Request) {
	v := r.URL.Query()

	req, err := http.NewRequest("POST", "http://localhost:9091/metrics/job/"+v.Get("id")+"/measurement/"+v.Get("measurement"), nil)
	if err != nil {
		// handle err
	}
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		// handle err
	}
	defer resp.Body.Close()

}

// main function to boot up everything
func main() {
	router := mux.NewRouter()
	// router.HandleFunc("/datasets", GetAll).Methods("GET")
	router.HandleFunc("/sensor/data", GetData).Methods("GET")
	router.HandleFunc("/sensor/data", CreateData).Methods("POST")
	// router.HandleFunc("/deletedataset/{job}", DeleteAll).Methods("GET")
	fmt.Printf("Starting server for testing HTTP GET POST...\n")
	router.PathPrefix("/static/").Handler(http.StripPrefix("/static/", http.FileServer(http.Dir("static"))))
	log.Fatal(http.ListenAndServe(":8000", router))
}
