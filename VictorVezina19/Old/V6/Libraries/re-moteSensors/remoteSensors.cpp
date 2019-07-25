/*
Library for reading from various sensors, used in the re-mote setup found at https://gitlab.cas.mcmaster.ca/re-mote
Created by Victor Vezina, last updated July 9, 2019
Released into the public domain
*/

#include "remoteSensors.h"
#include <NeoSWSerial.h>
#include <OneWire.h>
#include <DHT.h>
#include <remoteConfig.h>

#define EN false

remoteSensors::remoteSensors(int numberSensors, char* sensorTypes[], uint8_t sensorPorts[][2]) {
    _numberSensors = (uint8_t) numberSensors;
    _sensorTypes = sensorTypes;
    _sensorPorts = sensorPorts;
}

void remoteSensors::initialise() {
    //Go through each sensor and call it's respective initialisation function
    for (uint8_t i = 0; i < _numberSensors; i++) {
        if (strcmp(_sensorTypes[i], "Dissolved_Oxygen") == 0) {
            initialiseDO(i);
        } else if (strcmp(_sensorTypes[i], "Conductivity") == 0) {
            initialiseConductivity(i);
        } else if (strcmp(_sensorTypes[i], "Turbidity") == 0) {
            initialiseTurbidity(i);
        } else if (strcmp(_sensorTypes[i], "Water_Temperature") == 0) {
            initialiseWaterTemp(i);
        } else if (strcmp(_sensorTypes[i], "Air_Temperature") == 0) {
            initialiseDHT22(i);
        } else if (strcmp(_sensorTypes[i], "Humidity") == 0) {
            initialiseDHT22(i);
        }
    }
}

//Initialise an Atlas Scientific Dissolved Oxygen sensor at index index
void remoteSensors::initialiseDO(uint8_t index) {
    //Initialise the software serial for this sensor
    NeoSWSerial sensor (_sensorPorts[index][0], _sensorPorts[index][1]);
    sensor.begin(9600);
    delay(100);
    
    //Itialise common Atlas Scientific settings
    initialiseAtlas(sensor);
    
    
    //Set proper reading outputs
    sensor.print(F("O,DO,1\r")); //For older firmware versions
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;

    sensor.print(F("O,mg,1\r"));
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;

    sensor.print(F("O,%,0\r"));
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;
    
    
    sensor.print(F("Sleep\r")); //Put sensor to sleep
    
    sensor.end();
}

//Initialise an Atlas Scientific Conductivity sensor at index index
void remoteSensors::initialiseConductivity(uint8_t index) {
    //Initialise the software serial for this sensor
    NeoSWSerial sensor (_sensorPorts[index][0], _sensorPorts[index][1]);
    sensor.begin(9600);
    delay(100);
    
    //Itialise common Atlas Scientific settings
    initialiseAtlas(sensor);
    
    
    //Set proper probe type
    sensor.print(F("K,1.0\r"));
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;


    //Set proper reading outputs (Only needed for older firmware version)
    sensor.print(F("O,EC,1\r"));
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;

    sensor.print(F("O,TDS,0\r"));
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;

    sensor.print(F("O,S,0\r"));
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;

    sensor.print(F("O,SG,0\r"));
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;


    //Put sensor to sleep or turn it off
    if (EN) {
        Serial.println(F("This hasn't been implemented yet"));
        while (true) ;
    } else {
        sensor.print(F("Sleep\r")); //Put sensor to sleep
    }
    
    sensor.end();
}

//Initialise an Atlas Scientific sensor at index index
void remoteSensors::initialiseAtlas(Stream& sensor) {
    //For some reason it seems you have to send arandom command at startup
    sensor.print(F("R\r"));
    
    //Wait for response and get rid of it
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;
    
    sensor.print(F("OK,1\r")); //Turn on OK responses (does nothing on older firmware)
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;
    
    sensor.print(F("C,0\r")); //Turn off continous mode
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;
    
    sensor.print(F("Plock,1\r")); //Lock to UART mode
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;
    
    sensor.print(F("L,0\r")); //Turn off LEDs
    while (!(sensor.available())) ;
    delay(100);
    while (sensor.available()) sensor.read() ;
}

//Initialise a DFRobot Turbidity sensor at index i
void remoteSensors::initialiseTurbidity(uint8_t index) {
    pinMode(_sensorPorts[index][0], OUTPUT); //Set the digital pin to output
    digitalWrite(_sensorPorts[index][0], LOW); //Turn the sensor off
    
    pinMode(_sensorPorts[index][1], INPUT); //Set the analog pin to input
}

//Initialise a DFRobot Temperature sensor at index i
void remoteSensors::initialiseWaterTemp(uint8_t index) {
    ;
}

//Initialise a DHT22 sensor at index i
void remoteSensors::initialiseDHT22(uint8_t index) {
    ;
}


//Read data from all sensors based on their types
void remoteSensors::read(uint8_t* dataArr) {
    #ifdef DEBUG
    Serial.println(F("\nReading Sensors"));
    #endif
    
    uint8_t curr = 0; //Current position in the array
    //Go through each sensor and get the data, then add it to the data array
    for (uint8_t i = 0; i < _numberSensors; i++) {
        float data; //Data from this sensor
        
        if (strcmp(_sensorTypes[i], "Dissolved_Oxygen") == 0) {
            data = readDO(i);
        } else if (strcmp(_sensorTypes[i], "Conductivity") == 0) {
            data = readConductivity(i);
        } else if (strcmp(_sensorTypes[i], "Turbidity") == 0) {
            data = readTurbidity(i);
        } else if (strcmp(_sensorTypes[i], "Water_Temperature") == 0) {
            data = readWaterTemp(i);
        } else if (strcmp(_sensorTypes[i], "Air_Temperature") == 0) {
            data = readAirTemp(i);
        } else if (strcmp(_sensorTypes[i], "Humidity") == 0) {
            data = readHumidity(i);
        } else { //Should probably handle this better
            data = -1;
        }
        
        //Copy this sensor's data into the data array
        memcpy(&dataArr[curr], &data, sizeof(uint8_t) * 4);
        curr += 4;
    }
}

//Read data from an Atlas Scientific Dissolved Oxygen sensor at index i
float remoteSensors::readDO(uint8_t index) {
    //Initialise the sensor's software serial
    NeoSWSerial sensor (_sensorPorts[index][0], _sensorPorts[index][1]);
    sensor.begin(9600);
    delay(100);
    
    float ans = readAtlas(sensor);
    
    #ifdef DEBUG
    Serial.print(F("Read DO sensor: "));
    Serial.println(ans);
    #endif
    
    //Put sensor to sleep
    sensor.print(F("Sleep\r"));
    
    sensor.end();
    return ans;
}

//Read data from an Atlas Scientific Conductivity sensor at index i
float remoteSensors::readConductivity(uint8_t index) {
    //Initialise the sensor's software serial
    NeoSWSerial sensor (_sensorPorts[index][0], _sensorPorts[index][1]);
    sensor.begin(9600);
    delay(100);
    
    float ans = readAtlas(sensor);
    
    #ifdef DEBUG
    Serial.print(F("Read Conductivity sensor: "));
    Serial.println(ans);
    #endif
    
    //Put sensor to sleep or turn it off
    if (EN) {
        Serial.println(F("This hasn't been implemented yet"));
        while (true) ;
    } else {
        sensor.print(F("Sleep\r"));
    }
    
    sensor.end();
    return ans;
}

//Read data from an Atlas Scientific sensor at index i
float remoteSensors::readAtlas(Stream& sensor) {
    //Compensate for temp(both) and salinity(DO)??
    
    //Wake the sensor up
    sensor.print(F("R\r"));
    
    float data = 0.0; //Data for this sensor
    
    char buf[40]; //Char buffer for the sensor's response
    
    uint8_t tries = 0; //How many times the sensor has been read
    
    //If a 0 is read the first time, will try again just to make sure (have had trouble with EC sensor returning 0 first try for no reason)
    while (!data && (tries++ < 1)) {
        //Make the sensor read until it returns a number
        do {
            delay(900); //Delay to not flood sensor (only 1 reading/sec)
            sensor.print(F("R\r")); //Send the read command to the sensor
            while (sensor.available() <= 0); //Wait for it's response
            delay(100); //Delay to wait for the entire response to be ready

            int dataIndex = sensor.readBytesUntil('\r', buf, 16); //Read the response
            buf[dataIndex] = 0; //Add terminating character
        } while ((buf[0]<48) || (buf[0]>57)); //While the buffer isn't a number

        data = atof(buf); //Convert the character array response to a float
    }
    
    return data;
}

//Read data from a DFRobot turbidity sensor at index i
float remoteSensors::readTurbidity(uint8_t index) {
    digitalWrite(_sensorPorts[index][0], HIGH); //Turn on the sensor
    delay(250);
    
    float data = analogRead(_sensorPorts[index][1]); //Read the sensor
    
    digitalWrite(_sensorPorts[index][0], LOW); //Turn off the sensor
    
    //Do some manipulation to the data to get a more "accurate" number (just something that looks more correct)
    float ans;
    if (data < 775.0) {
        ans = 775.0 - data;
    } else {
        int factor = data / 5;
        ans = data - (5.0 * factor);
    }
    
    #ifdef DEBUG
    Serial.print(F("Read Turbidity sensor: "));
    Serial.println(ans);
    #endif
    
    return ans;
}

//Read data from a DFRobot turbidity sensor at index i
float remoteSensors::readWaterTemp(uint8_t index) {
    OneWire tempSensor(_sensorPorts[index][0]); //Start One Wire interface
    delay(100);
    
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
    
    #ifdef DEBUG
    Serial.print(F("Read Water_Temperature sensor: "));
    Serial.println(ans);
    #endif
    
    return ans;
}

//Read the temperature from a DHT22
float remoteSensors::readAirTemp(uint8_t index) {
    DHT dht(_sensorPorts[index][0], DHT22);
    dht.begin();
    delay(100);
    
    float ans = dht.readTemperature();
    
    #ifdef DEBUG
    Serial.print(F("Read Air_Temperature sensor: "));
    Serial.println(ans);
    #endif
    
    return ans;
}

//Read the humidity from a DHT22
float remoteSensors::readHumidity(uint8_t index) {
    DHT dht(_sensorPorts[index][0], DHT22);
    
    dht.begin();
    
    float ans = dht.readHumidity();
    
    #ifdef DEBUG
    Serial.print(F("Read Humidity sensor: "));
    Serial.println(ans);
    #endif
    
    return ans;
}