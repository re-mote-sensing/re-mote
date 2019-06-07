/*-------------------------INCLUDES-----------------------------*/
/*
#include <AES.h>
#include <base64.h>
#include <AES_config.h>
#include <AESLib.h>
*/

#include <SPI.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

/*----------------------DEFINITIONS-----------------------------*/

#define TYPE "Dissolved_Oxygen" //Sensor type
#define SERVER_ADDRESS 0x0003
#define SENSOR_RX 8
#define SENSOR_TX 9
#define LORA_RX 5
#define LORA_TX 6

/*-------------------------GLOBALS------------------------------*/

uint16_t id;
/*
byte aes_key[] = { 0x15, 0x2B, 0x7E, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
byte aes_iv[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
AESLib aesLib;
*/

/*--------------------SENSOR CONSTRUCTORS-----------------------*/

SoftwareSerial sensorSerial(SENSOR_RX, SENSOR_TX);
SoftwareSerial loraPort(LORA_RX, LORA_TX);

/*---------------------------SETUP------------------------------*/

void setup() {
  sensorSerial.begin(9600);
  loraPort.begin(9600);

  Serial.begin(9600);

  delay(3000);
  
  id = readConfig(loraPort);

  //Check if EEPROM has ever been used
  unsigned int initialCheck = 0;
  EEPROM.get(0, initialCheck);
  //Serial.println(initialCheck);
  if (initialCheck >= EEPROM.length()) {
    int test = 2;
    EEPROM.put(0, test);
    Serial.println("Set EEPROM[0] to 2");
  }

  //int i = 2;
  //EEPROM.put(0, i);

  
  //aes_init();
}

/*
void aes_init() {
  aesLib.gen_iv(aes_iv);
  // workaround for incorrect B64 functionality on first run...
  byte *message = (byte *) "HELLO WORLD";
  
  byte enc_iv[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  char encrypted[22];
  aesLib.encrypt64(message, encrypted, aes_key, enc_iv);
}
*/

void loop() {
  /*
  Serial.println("---------------------------Starting loop------------------------");
  //AES encryption
  char *message = "Hello_World";
  
  byte enc_iv[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  char encrypted[100];
  aesLib.encrypt64(message, encrypted, aes_key, enc_iv);
  
  Serial.println(encrypted);
  
  byte dec_iv[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  char decrypted[100];
  aesLib.decrypt64(encrypted, decrypted, aes_key, dec_iv);

  Serial.println(decrypted);
  

  /////////////////////////////////////////////////////////
  */
  unsigned long start = millis();
  
  sensorSerial.listen();
  
  sensorSerial.print("R\r"); // Command to read value from Atlas Scientific sensors

  while (sensorSerial.available() <= 0);
  
  char buf[48];
  int dataIndex = sensorSerial.readBytesUntil('\r', buf, 48);
  buf[dataIndex] = 0;
  if ((buf[0]>=48) && (buf[0]<=57)) { //Makes sure the data is a number
    float data = atof(buf);
    
    bool recieved = sendSensorData(data);

    if (!recieved) {
      Serial.println("Didn't recieve ack");

      //copy data to EEPROM to send next time
      int address = 2;
      EEPROM.get(0, address);
      //Checks for exceeding EEPROM length
      //Don't really like the way I did it, but it should never happen anyway (hopefully)
      if (!(address + 3 >= EEPROM.length())) {
        EEPROM.put(address, data);
        if (!(address + 4 >= EEPROM.length())) {
          EEPROM.put(0, address+4);
        }
      }

      if (millis() < start) { //Overflow check
        delay(45000 - (millis() + (4294967295-start)));
      } else {
        delay(45000 - (millis() - start));
      }
    } else {
      Serial.println("Recieved ack");

      //Send more backed up data if it exists
      int memAddress = 2;
      while (recieved) {
        EEPROM.get(0, memAddress);
        if (((memAddress - 2) / 4) > 0) {
          delay(3000);
          recieved = sendSavedData();
        } else {
          break;
        }
      }

      if (recieved) {
        if (millis() < start) { //Overflow check
          delay(60000 - (millis() + (4294967295-start)));
        } else {
          delay(60000 - (millis() - start));
        }
      } else {
        if (millis() < start) { //Overflow check
          delay(45000 - (millis() + (4294967295-start)));
        } else {
          delay(45000 - (millis() - start));
        }
      }
    }
  }
}

//Takes in sensor data, forms that+global variables into message, sends message, then waits for ack
bool sendSensorData(float data) {
  int memAddress = 2;
  EEPROM.get(0, memAddress);
  int numPoints = (memAddress - 2) / 4;

  //Can't send more than 111 bytes of data
  if (numPoints * 4 > (111 - (2 + 1 + 4 + strlen(TYPE)))) {
    numPoints = (111 - (2 + 1 + 4 + strlen(TYPE)))/4;
  }
  
  uint8_t* dataArr = malloc(sizeof(uint8_t) * (2 + 1 + ((numPoints + 1) * 4) + strlen(TYPE)));

  //Build begining of message
  uint8_t dataPoints = numPoints + 1;
  memcpy(dataArr, &id, sizeof(uint8_t) * 2);
  memcpy(&dataArr[2], &dataPoints, sizeof(uint8_t));
  memcpy(&dataArr[3], &data, sizeof(uint8_t) * 4);

  //Add data to dataArr from backed up data
  int i;
  float currData = 0;
  for (i = 1; i <= numPoints; i++) {
    EEPROM.get(memAddress-(i*4), currData);
    memcpy(&dataArr[3+(i*4)], &currData, sizeof(uint8_t) * 4);
  }
  
  for (int j = 0; j < strlen(TYPE); j++) {
    dataArr[3+(i*4)+j] = TYPE[j];
  }
  
  Serial.print("Sending: ");
  Serial.print(data);
  Serial.print(", plus ");
  Serial.print(numPoints);
  Serial.println(" points from before");

  loraPort.listen();

  //Send the data through LoRa
  sendData(loraPort, SERVER_ADDRESS, (2 + 1 + ((numPoints + 1) * 4) + strlen(TYPE)), dataArr);

  //Wait for acknoledgement
  unsigned long beforeAck = millis();
  bool recieved = false;
  while ((millis() >= beforeAck) && (millis() - beforeAck < 5000) ) { //First check is for overflow
    if (loraPort.available()) {
      uint8_t* loraData = readData(loraPort);
      uint16_t address;
      memcpy(&address, &loraData[1], sizeof(uint8_t) * 2);
      if (address == SERVER_ADDRESS) { //Check that acknoledgement is from server
        free(loraData);
        recieved = true;

        int newAddress = memAddress - 4*numPoints;
        EEPROM.put(0, newAddress);
        
        break;
      }
      free(loraData);
    }
  }
  free(dataArr);
  return recieved;
}

//Sends backup data; uses data in EEPROM+global variables to form message, sends message, then waits for ack
bool sendSavedData() {
  int memAddress = 2;
  EEPROM.get(0, memAddress);
  int numPoints = (memAddress - 2) / 4;

  //Can't send more than 111 bytes of data
  if (numPoints * 4 > (111 - (2 + 1 + strlen(TYPE)))) {
    numPoints = (111 - (2 + 1 + strlen(TYPE)))/4;
  }
  
  uint8_t* dataArr = malloc(sizeof(uint8_t) * (2 + 1 + (numPoints * 4) + strlen(TYPE)));

  //Build begining of message
  memcpy(dataArr, &id, sizeof(uint8_t) * 2);
  memcpy(&dataArr[2], &numPoints, sizeof(uint8_t));

  //Add data to dataArr from backed up data
  int i;
  float currData = 0;
  for (i = 1; i <= numPoints; i++) {
    EEPROM.get(memAddress-(i*4), currData);
    memcpy(&dataArr[3+((i-1)*4)], &currData, sizeof(uint8_t) * 4);
  }

  i--;
  
  for (int j = 0; j < strlen(TYPE); j++) {
    dataArr[3+(i*4)+j] = TYPE[j];
  }
  
  Serial.print("Sending ");
  Serial.print(numPoints);
  Serial.println(" points from before");

  loraPort.listen();
  
  //Send the data through LoRa
  sendData(loraPort, SERVER_ADDRESS, (2 + 1 + (numPoints * 4) + strlen(TYPE)), dataArr);

  //Wait for acknoledgement
  unsigned long beforeAck = millis();
  bool recieved = false;
  while ((millis() >= beforeAck) && (millis() - beforeAck < 5000) ) { //First check is for overflow
    if (loraPort.available()) {
      uint8_t* loraData = readData(loraPort);
      uint16_t address;
      memcpy(&address, &loraData[1], sizeof(uint8_t) * 2);
      if (address == SERVER_ADDRESS) { //Check that acknoledgement is from server
        free(loraData);
        recieved = true;

        int newAddress = memAddress - 4*numPoints;
        EEPROM.put(0, newAddress);
        
        break;
      }
      free(loraData);
    }
  }
  free(dataArr);
  return recieved;
}


//Lower level LoRa Stuff:

void printByte(uint8_t b) {
  Serial.print(" 0x");
  if (b <= 0xF)
    Serial.print("0");
  Serial.print(b, HEX);
}

void printShort(uint16_t s) {
  Serial.print(" 0x");
  if (s <= 0xFFF)
    Serial.print("0");
  if (s <= 0xFF)
    Serial.print("0");
  if (s <= 0xF)
    Serial.print("0");
  Serial.print(s, HEX);
}


uint8_t readByte(Stream& port) {
  while (!port.available());
  return port.read();
}

int readFrame(Stream& port, byte* rFrameType, byte* rCmdType, byte** rPayload) {
  uint8_t checksum = 0;
  
  uint8_t frameType = readByte(port);
  uint8_t frameNum = readByte(port);
  uint8_t cmdType = readByte(port);
  uint8_t payloadLen = readByte(port);

  checksum ^= frameType;
  checksum ^= frameNum;
  checksum ^= cmdType;
  checksum ^= payloadLen;

  uint8_t* payload = (uint8_t *) malloc(sizeof(uint8_t) * payloadLen);
  for (int i = 0; i < payloadLen; i++) {
    payload[i] = readByte(port);
    checksum ^= payload[i];
  }

  *rFrameType = frameType;
  *rCmdType = cmdType;
  *rPayload = payload;

  uint8_t frameCheck = readByte(port);
  checksum ^= frameCheck;

  if (checksum != 0)
    return -1;
  return payloadLen;
}

void writeFrame(Stream& port, uint8_t frameType, uint8_t cmdType, uint8_t payloadLen, uint8_t* payload) {
  uint8_t checksum = 0;

  checksum ^= frameType;
  checksum ^= 0; // frameNum which is unused and always 0
  checksum ^= cmdType;
  checksum ^= payloadLen;

  //printByte(frameType);
  //printByte((uint8_t) 0); // frameNum
  //printByte(cmdType);
  printByte(payloadLen);

  port.write(frameType);
  port.write((uint8_t) 0); // frameNum
  port.write(cmdType);
  port.write(payloadLen);

  for (int i = 0; i < payloadLen; i++) {
    checksum ^= payload[i];
    port.write(payload[i]);
    printByte(payload[i]);
  }

  port.write(checksum);
  //printByte(checksum);

  //Serial.println();
}

uint8_t* readData(Stream& port) {
  uint8_t frameType = 0;
  uint8_t cmdType = 0;
  uint8_t* payload = NULL;
  int len = readFrame(port, &frameType, &cmdType, &payload);

  if (frameType != 0x05 || cmdType != 0x82) {
    Serial.println("BAD TYPE");
    free(payload);
    return;
  }

  uint16_t srcAddr = (payload[0] << 8) | payload[1];
  uint8_t power = payload[2];
  uint8_t userPayloadLength = payload[3];
  //Serial.print(" srcAddr="); printShort(srcAddr);
  //Serial.print(" power="); printByte(power);
  //Serial.print(" userPayloadLength="); printByte(userPayloadLength);

  uint8_t *ans = malloc(sizeof(uint8_t) * (userPayloadLength + 1));
  ans[0] = userPayloadLength;
  for (int i = 0; i < userPayloadLength; i++) {
    ans[i+1] = payload[4+i];
    //printByte(payload[4 + i]);
  }
  //Serial.println();
  free(payload);
  return ans;
}

// Max len is 111 bytes
void sendData(Stream& port, uint16_t target, uint8_t dataLen, uint8_t* data) {
  // We add 7 bytes to the head of data for this payload
  uint8_t payloadLen = 6 + dataLen;
  uint8_t* payload = (uint8_t *) malloc(sizeof(uint8_t) * payloadLen);

  // target address as big endian short
  payload[0] = (uint8_t) ((target >> 8) & 0xFF);
  payload[1] = (uint8_t) (target & 0xFF);

  // ACK request == 1 -> require acknowledgement of recv
  payload[2] = (uint8_t) 0;//1;

  // Send radius: which defaults to max of 7 hops, we can use that
  payload[3] = (uint8_t) 7;

  // Discovery routing params == 1 -> automatic routing
  payload[4] = (uint8_t) 1;

  // Source routing domain: unused when automatic routing enabled
  //    - number of relays is 0
  //    - relay list is therefor non-existent
  //payload[5] = (uint8_t) 0;

  // Data length
  payload[5] = dataLen;

  // Data from index 7 to the end should be the data
  memcpy(payload + (sizeof(uint8_t) * 6), data, dataLen);

  // frameType = 0x05, cmdType = 0x01 for sendData
  writeFrame(port, 0x05, 0x01, payloadLen, payload);

  free(payload);

  uint8_t frameType = 0;
  uint8_t cmdType = 0;
  uint8_t* responsePayload = NULL;
  int len = readFrame(port, &frameType, &cmdType, &responsePayload);

  if (frameType == 0x5 && cmdType == 0x81) {
    // Application data sending response
    uint16_t targetAddr = (responsePayload[0] << 8) | responsePayload[1];
    uint8_t status = responsePayload[2];
    Serial.print(" targetAddr="); printShort(targetAddr);
    Serial.print(" status="); printByte(status);
    Serial.println();
  }
  free(responsePayload);
}

uint16_t readConfig(Stream& port) {
  writeFrame(port, 0x01, 0x02, 0x00, NULL);

  uint8_t frameType = 0;
  uint8_t cmdType = 0;
  uint8_t* payload = NULL;
  
  int len = readFrame(port, &frameType, &cmdType, &payload);
  
  if (len < 0) {
    Serial.println("ERROR");
    free(payload);
    return;
  }

  Serial.print("FrameType=");
  printByte(frameType);
  Serial.print(" CmdType=");
  printByte(cmdType);

  if (frameType != 0x01 || cmdType != 0x82) {
    Serial.println("BAD TYPE");
    free(payload);
    return;
  }

  uint16_t flag = (payload[0] << 8) | payload[1];
  uint8_t channel = payload[2];
  uint8_t txPower = payload[3];
  uint8_t uiMode = payload[4];
  uint8_t eqType = payload[5];
  uint16_t netId = (payload[6] << 8) | payload[7];
  uint16_t nodeId = (payload[8] << 8) | payload[9];
  // 10, 11, 12 reserved
  uint8_t serPortParam = payload[13];
  uint16_t airRate = (payload[14] << 8) | payload[15];

  Serial.print(" flag="); printShort(flag);
  Serial.print(" channel="); printByte(channel);
  Serial.print(" txPower="); printByte(txPower);
  Serial.print(" uiMode="); printByte(uiMode);
  Serial.print(" eqType="); printByte(eqType);
  Serial.print(" netId="); printShort(netId);
  Serial.print(" nodeId="); printShort(nodeId);
  Serial.print(" serPortParam="); printByte(serPortParam);
  Serial.print(" airRate="); printShort(airRate);
  Serial.println();
  return nodeId;
}
