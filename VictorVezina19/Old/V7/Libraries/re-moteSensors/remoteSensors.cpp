/*
Library for reading from various sensors, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated July 29, 2019
Released into the public domain
*/

#include "remoteSensors.h"
#include <NeoSWSerial.h>
#include <OneWire.h>
#include <DHT.h>
#include <remoteConfig.h>

remoteSensors::remoteSensors() {
}

void remoteSensors::initialise() {
    //Go through each sensor and call it's respective initialisation function
    for (uint8_t i = 0; i < NUMBER_SENSOR_TYPES; i++) {
        #ifdef AS_DO_Sensor
        if (strcmp(sensorTypes[i], "AS_DO") == 0) {
            initialiseASDO(i);
        }
        #endif
        
        #ifdef AS_EC_Sensor
        if (strcmp(sensorTypes[i], "AS_EC") == 0) {
            initialiseASEC(i);
        }
        #endif
        
        #ifdef DF_TB_Sensor
        if (strcmp(sensorTypes[i], "DF_TB") == 0) {
            initialiseDFTB(i);
        }
        #endif
        
        #ifdef DF_Temp_Sensor
        if (strcmp(sensorTypes[i], "DF_Temp") == 0) {
            initialiseDFTemp(i);
        }
        #endif
        
        #ifdef DHT22_Sensor
        if (strcmp(sensorTypes[i], "DHT22") == 0) {
            initialiseDHT22(i);
        }
        #endif
    }
}

#ifdef AS_DO_Sensor
//Initialise an Atlas Scientific Dissolved Oxygen sensor at index index
void remoteSensors::initialiseASDO(uint8_t index) {
    //Initialise the software serial for this sensor
    NeoSWSerial sensor (sensorPorts[index][0], sensorPorts[index][1]);
    sensor.begin(9600);

    //Itialise common Atlas Scientific settings
    initialiseAtlas(sensor);


    //Set proper reading outputs
    sensor.print(F("O,DO,1\r")); //For older firmware versions
    ASWait(sensor);

    sensor.print(F("O,mg,1\r"));
    ASWait(sensor);

    sensor.print(F("O,%,0\r"));
    ASWait(sensor);


    sensor.print(F("Sleep\r")); //Put sensor to sleep

    sensor.end();
}
#endif

#ifdef AS_EC_Sensor
//Initialise an Atlas Scientific Conductivity sensor at index index
void remoteSensors::initialiseASEC(uint8_t index) {
    #if AS_EC_Sensor == true
    pinMode(sensorPorts[index][2], OUTPUT);
    digitalWrite(sensorPorts[index][2], HIGH);
    #endif
    
    //Initialise the software serial for this sensor
    NeoSWSerial sensor (sensorPorts[index][0], sensorPorts[index][1]);
    sensor.begin(9600);
    
    //Itialise common Atlas Scientific settings
    initialiseAtlas(sensor);
    
    
    //Set proper probe type
    sensor.print(F("K,1.0\r"));
    ASWait(sensor);
    

    //Set proper reading outputs (Only needed for older firmware version)
    sensor.print(F("O,EC,1\r"));
    ASWait(sensor);
    
    sensor.print(F("O,TDS,0\r"));
    ASWait(sensor);
    
    sensor.print(F("O,S,0\r"));
    ASWait(sensor);
    
    sensor.print(F("O,SG,0\r"));
    ASWait(sensor);
    

    sensor.print(F("Sleep\r")); //Put sensor to sleep
    
    //If it can be turned off, turn it off
    #if AS_EC_Sensor == true
    digitalWrite(sensorPorts[index][2], LOW);
    #endif
    
    sensor.end();
}
#endif

#if defined(AS_DO_Sensor) || defined(AS_EC_Sensor)
//Initialise an Atlas Scientific sensor at index index
void remoteSensors::initialiseAtlas(Stream& sensor) {
    //For some reason it seems you have to send arandom command at startup
    sensor.print(F("R\r"));
    
    ASWait(sensor);
    
    sensor.print(F("OK,1\r")); //Turn on OK responses (does nothing on older firmware)
    ASWait(sensor);
    
    sensor.print(F("C,0\r")); //Turn off continous mode
    ASWait(sensor);
    
    sensor.print(F("Plock,1\r")); //Lock to UART mode
    ASWait(sensor);
    
    sensor.print(F("L,0\r")); //Turn off LEDs
    ASWait(sensor);
}

//Waits for an AS sensor to return a response and dumps it
void remoteSensors::ASWait(Stream& sensor) {
    while (!sensor.available());
    delay(100);
    while (sensor.available()) sensor.read();
}
#endif

#ifdef DF_TB_Sensor
//Initialise a DFRobot Turbidity sensor at index i
void remoteSensors::initialiseDFTB(uint8_t index) {
    pinMode(sensorPorts[index][0], OUTPUT); //Set the digital pin to output
    digitalWrite(sensorPorts[index][0], LOW); //Turn the sensor off
    
    pinMode(sensorPorts[index][1], INPUT); //Set the analog pin to input
}
#endif

#ifdef DF_Temp_Sensor
//Initialise a DFRobot Temperature sensor at index i
void remoteSensors::initialiseDFTemp(uint8_t index) {
    ;
}
#endif

#ifdef DHT22_Sensor
//Initialise a DHT22 sensor at index i
void remoteSensors::initialiseDHT22(uint8_t index) {
    ;
}
#endif


//Read data from all sensors based on their types
void remoteSensors::read(uint8_t* dataArr) {
    #ifdef DEBUG
    Serial.println(F("\nReading Sensors"));
    #endif
    
    uint8_t curr = 0; //Current position in the array
    //Go through each sensor and get the data, then add it to the data array
    for (uint8_t i = 0; i < NUMBER_SENSOR_TYPES; i++) {
        #ifdef AS_DO_Sensor
        if (strcmp(sensorTypes[i], "AS_DO") == 0) {
            curr += readASDO(i, &dataArr[curr]);
        }
        #endif

        #ifdef AS_EC_Sensor
        if (strcmp(sensorTypes[i], "AS_EC") == 0) {
            curr += readASEC(i, &dataArr[curr]);
        }
        #endif

        #ifdef DF_TB_Sensor
        if (strcmp(sensorTypes[i], "DF_TB") == 0) {
            curr += readDFTB(i, &dataArr[curr]);
        }
        #endif

        #ifdef DF_Temp_Sensor
        if (strcmp(sensorTypes[i], "DF_Temp") == 0) {
            curr += readDFTemp(i, &dataArr[curr]);
        }
        #endif

        #ifdef DHT22_Sensor
        if (strcmp(sensorTypes[i], "DHT22") == 0) {
            curr += readDHT22(i, &dataArr[curr]);
        }
        #endif

        //Should probably handle default
        
        #ifdef Temperature_Comp
        if (Temperature_Comp == i) {
            uint8_t backAmount = 4;
            #ifdef Temperature_Comp_Index
            backAmount += 4 * Temperature_Comp_Index;
            #endif
            
            memcpy(&lastTemp, &dataArr[curr - backAmount], sizeof(float));
        }
        #endif
        
        #ifdef Salinity_Comp
        if (Salinity_Comp == i) {
            uint8_t backAmount = 4;
            #ifdef Salinity_Comp_Index
            backAmount += 4 * Salinity_Comp_Index;
            #endif
            
            memcpy(&lastSal, &dataArr[curr - backAmount], sizeof(float));
        }
        #endif
    }
}

#ifdef AS_DO_Sensor
//Read data from an Atlas Scientific Dissolved Oxygen sensor at index i
uint8_t remoteSensors::readASDO(uint8_t index, uint8_t* data) {
    //Initialise the sensor's software serial
    NeoSWSerial sensor (sensorPorts[index][0], sensorPorts[index][1]);
    sensor.begin(9600);
    
    //Wake the sensor up
    sensor.print(F("R\r"));
    ASWait(sensor);
    sensor.print(F("R\r"));
    ASWait(sensor);
    
    #ifdef Salinity_Comp
    //Compensate for salinity
    sensor.print(F("S,"));
    sensor.print(lastSal);
    sensor.print('\r');
    ASWait(sensor);
    #endif
    
    float ans = readAtlas(sensor);
    
    #ifdef DEBUG
    Serial.print(F("Read DO sensor: "));
    Serial.println(ans);
    #endif
    
    //Put sensor to sleep
    sensor.print(F("Sleep\r"));
    
    memcpy(data, &ans, sizeof(float));
    
    sensor.end();
    return 4;
}
#endif

#ifdef AS_EC_Sensor
//Read data from an Atlas Scientific Conductivity sensor at index i
uint8_t remoteSensors::readASEC(uint8_t index, uint8_t* data) {
    #if AS_EC_Sensor == true
    digitalWrite(sensorPorts[index][2], HIGH);
    #endif
    
    //Initialise the sensor's software serial
    NeoSWSerial sensor (sensorPorts[index][0], sensorPorts[index][1]);
    sensor.begin(9600);
    
    //Wake the sensor up
    sensor.print(F("R\r"));
    ASWait(sensor);
    sensor.print(F("R\r"));
    ASWait(sensor);
    
    float ans = readAtlas(sensor);
    
    #ifdef DEBUG
    Serial.print(F("Read Conductivity sensor: "));
    Serial.println(ans);
    #endif
    

    sensor.print(F("Sleep\r")); //Put sensor to sleep
    
    //If it can be turned off, turn it off
    #if AS_EC_Sensor == true
    digitalWrite(sensorPorts[index][2], LOW);
    #endif
    
    sensor.end();
    
    memcpy(data, &ans, sizeof(float));
    return 4;
}
#endif

#if defined(AS_DO_Sensor) || defined(AS_EC_Sensor)
//Read data from an Atlas Scientific sensor at index i
float remoteSensors::readAtlas(Stream& sensor) {
    #ifdef Temperature_Comp
    //Compensate for temperature
    sensor.print(F("T,"));
    sensor.print(lastTemp);
    sensor.print('\r');
    ASWait(sensor);
    #endif
    
    float data = 0.0; //Data for this sensor
    
    char buf[40]; //Char buffer for the sensor's response
    
    uint8_t tries = 0; //How many times the sensor has been read
    
    //If a 0 is read the first time, will try again just to make sure (have had trouble with EC sensor returning 0 first try for no reason)
    while (!data && (tries++ < 1)) {
        //Make the sensor read until it returns a number
        do {
            delay(900); //Delay to not flood sensor (only 1 reading/sec)
            sensor.print(F("R\r")); //Send the read command to the sensor
            while (!sensor.available()); //Wait for it's response
            delay(100); //Delay to wait for the entire response to be ready

            int dataIndex = sensor.readBytesUntil('\r', buf, 40); //Read the response
            buf[dataIndex] = 0; //Add terminating character
        } while ((buf[0]<48) || (buf[0]>57)); //While the buffer isn't a number

        data = atof(buf); //Convert the character array response to a float
    }
    
    return data;
}
#endif

#ifdef DF_TB_Sensor
//Read data from a DFRobot turbidity sensor at index i
uint8_t remoteSensors::readDFTB(uint8_t index, uint8_t* data) {
    digitalWrite(sensorPorts[index][0], HIGH); //Turn on the sensor
    
    float ansData = analogRead(sensorPorts[index][1]); //Read the sensor
    
    digitalWrite(sensorPorts[index][0], LOW); //Turn off the sensor
    
    //Do some manipulation to the data to get a more "accurate" number (just something that looks more correct)
    float ans;
    if (ansData < 775.0) {
        ans = 775.0 - ansData;
    } else {
        int factor = ansData / 5;
        ans = ansData - (5.0 * factor);
    }
    
    memcpy(data, &ans, sizeof(float));
    
    #ifdef DEBUG
    Serial.print(F("Read Turbidity sensor: "));
    Serial.println(ans);
    #endif
    
    return 4;
}
#endif

#ifdef DF_Temp_Sensor
//Read data from a DFRobot turbidity sensor at index i
uint8_t remoteSensors::readDFTemp(uint8_t index, uint8_t* data) {
    OneWire tempSensor(sensorPorts[index][0]); //Start One Wire interface
    
    uint8_t addr[8];
    
    //Get the address of the sensor
    do {
        if (!tempSensor.search(addr)) {
            #ifdef DEBUG
            Serial.println(F("Didnt find temperature sensor"));
            #endif
            return -1;
        }
    } while (!(addr[0] == 16 || addr[0] == 40));
    
    //Make sure the CRC is valid
    if (OneWire::crc8(addr, 7) != addr[7]) {
        #ifdef DEBUG
        Serial.println(F("CRC not valid"));
        #endif
        return -1;
    }
    
    //Read temperature command
    tempSensor.reset();
    tempSensor.select(addr);
    tempSensor.write(0x44, 1);
    
    float ans;
    
    delay(750); //Temperature query time
    
    //Read the sensor's value until it's valid (conversion is done)
    do {
        //Read saved temperature value command
        tempSensor.reset();
        tempSensor.select(addr);
        tempSensor.write(0xBE);
        
        uint8_t data[2];
        
        //Get sensor's value
        for (uint8_t i = 0; i < 2; i++) {
            data[i] = tempSensor.read();
        }
        
        //Convert into deg C
        float tmp = ((data[1] << 8) | data[0]);
        ans = tmp / 16;
    } while (ans == 85); //It returns 85 if the conversion isn't yet finished
    
    memcpy(data, &ans, sizeof(float));
    
    #ifdef DEBUG
    Serial.print(F("Read Water_Temperature sensor: "));
    Serial.println(ans);
    #endif
    
    return 4;
}
#endif

#ifdef DHT22_Sensor
//Read temperature and humidity from a DHT22 sensor
uint8_t remoteSensors::readDHT22(uint8_t index, uint8_t* data) {
    DHT dht(sensorPorts[index][0], DHT22);
    dht.begin();
    
    float ans;
    do {
        ans = dht.readTemperature();
    } while (isnan(ans));
    
    memcpy(data, &ans, sizeof(uint32_t));
    
    #ifdef DEBUG
    Serial.print(F("Read DHT22 Temperature sensor: "));
    Serial.println(ans);
    #endif
    
    do {
        ans = dht.readHumidity();
    } while (isnan(ans) || ans < 0 || ans > 100);
    
    memcpy(&data[4], &ans, sizeof(uint32_t));
    
    #ifdef DEBUG
    Serial.print(F("Read DHT22 Humidity sensor: "));
    Serial.println(ans);
    #endif
    
    return 8;
}
#endif