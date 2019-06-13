// You have to hold down RST on LoRa+GPS Shields when uploading or you will get init failed

/*-------------------------INCLUDES-----------------------------*/

#include <SPI.h>
#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"

#include <EEPROM.h>
#include <MemoryFree.h>
//#include <avr/pgmspace.h>

/*----------------------DEFINITIONS-----------------------------*/

#define LORA_RX 6
#define LORA_TX 7
#define FONA_TX 4
#define FONA_RX 5
#define FONA_RST 9

#define TIME 1558105683 //Time that it's using for data, should be gotten from GPS

/*-------------------------GLOBALS------------------------------*/

unsigned long lastPost;
uint16_t id;
bool dataGood;

/*------------------------CONSTRUCTORS--------------------------*/

SoftwareSerial loraPort(LORA_RX, LORA_TX);
SoftwareSerial fonaSS(FONA_TX, FONA_RX);

Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

/*---------------------------SETUP------------------------------*/

void setup() {
  loraPort.begin(9600);

  Serial.begin(9600);

  delay(3000);

  /*
  fonaSS.begin(4800);
  if (! fona.begin(fonaSS)) {
      Serial.println("Couldn't find FONA");
      while (true);
  }
  Serial.println(F("FONA is OK"));

  delay(1000);

  fona.setGPRSNetworkSettings(F("pda.bell.ca"));
  fona.setHTTPSRedirect(true);

  loraPort.listen();
  */
  Serial.print(F("1: "));
  Serial.println(freeMemory()); //All these freeMemory() calls are for testing

  //Checks if EEPROM has ever been used
  if (EEPROM.read(0) == 255) {
    EEPROM.write(0, 0);
    Serial.println(F("Set EEPROM[0] to 0"));
  }

  //EEPROM.write(0, 0);

  id = readConfig(loraPort);
  lastPost = millis();
  
  Serial.print(F("2: "));
  Serial.println(freeMemory());
}

void loop() {
  loraPort.listen();
  Serial.println(F("Waiting for LoRa messages"));
  Serial.print(F("3: "));
  Serial.println(freeMemory());

  while ((millis() >= lastPost) ? ((millis() - lastPost) < 60000) : ((millis() + (4294967295 - lastPost)) < 60000 )) { //Makes sure to check for overflow
    if (loraPort.available()) {
      dataGood = true;
      
      uint8_t* loraData = readData(loraPort);
      
      /*printByte(loraData[0]);
      for (int i = 1; i <= loraData[0]; i++) {
        printByte(loraData[i]);
      }*/
      
      parseLoraData(loraData);
      free(loraData);
    }
  }
  Serial.println(F("Posting data"));
  lastPost = millis();
  Serial.print(F("8: "));
  Serial.println(freeMemory());
  postData();
}

//Parse the byte array given by readData
void parseLoraData(uint8_t* loraData) {
  uint16_t address;
  float* data = malloc(sizeof(float) * loraData[3]);
  byte typeLen = loraData[0] - 3 - (loraData[3] * 4);
  char* type = malloc(sizeof(char) * (typeLen + 1));
  type[typeLen] = 0;
  memcpy(&address, &loraData[1], sizeof(uint8_t) * 2);
  memcpy(data, &loraData[4], sizeof(uint8_t) * (loraData[3] * 4));
  memcpy(type, &loraData[loraData[3] * 4 + 4], typeLen);

  uint8_t* sendArr = malloc(sizeof(uint8_t) * 2);
  memcpy(sendArr, &id, sizeof(uint8_t) * 2);

  sendData(loraPort, address, 2, sendArr);

  free(sendArr);
  
  Serial.print(F("Recieved: "));
  for (int i = 0; i < loraData[3]; i++) {
    Serial.print(data[i]);
    Serial.print(F(", "));
  }
  Serial.print(F("of type: "));
  Serial.print(type);
  Serial.print(F(", from: "));
  Serial.println((String) address);

  //Save data to EEPROM
  
  if (EEPROM.read(0) == 0) {
    EEPROM.write(0, 1);
    EEPROM.put(1, address);
    EEPROM.write(3, typeLen);
    for (int i = 0; i < typeLen; i++) {
      EEPROM.write((4 + i), type[i]);
    }
    EEPROM.write(4+typeLen, 0);
  }

  int writePos = 1;
  writePos += 2; //Skip id
  byte len = EEPROM.read(writePos); //Get type length
  writePos += len + 1; //Skip type
  len = EEPROM.read(writePos); //Get number of data points
  if (len >= 50) {
    writePos += 1 + ((50 - loraData[3]) * 8); //Overwrite last data points
  } else {
    EEPROM.write(writePos, len + (loraData[3])); //Change how many data points are currently saved
    writePos += 1 + (len * 8); //Skip data points
  }

  for (int i = 0; i < loraData[3]; i++) {
    EEPROM.put(writePos, data[i]);
    unsigned long currTime = TIME + (millis() / 1000);
    EEPROM.put(writePos+4, currTime);
    writePos += 8;
    delay(1000);
  }

  free(type);
  free(data);
}

//Post collected data to webserver
void postData() {
  if (!dataGood) {
    return;
  }
  
  //Initialising the FONA:
  fonaSS.begin(4800);
  if (! fona.begin(fonaSS)) {
      Serial.println(F("Couldn't find FONA"));
      while (true);
  }
  Serial.println(F("FONA is OK"));

  delay(1000);

  fona.setGPRSNetworkSettings(F("pda.bell.ca"));
  fona.setHTTPSRedirect(true);
  
  Serial.print(F("10: "));
  Serial.println(freeMemory());

  //Build HTTP request
  char* reqArr = buildCharRequest();
  
  Serial.print(F("12: "));
  Serial.println(freeMemory());

  Serial.print(F("-------------------------------------\n\n\n"));
  Serial.print(reqArr);
  Serial.println(F("\n\n\n-------------------------------------"));

  //Somehow check for error here, if error then send this data next time
  for (int i = 0; i < 3 && !fona.sendCheckReply(F("AT+CHTTPSSTART"), F("OK"), 30000); i++);
  delay(100);
  for(int i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSOPSE=\"www.cas.mcmaster.ca\",80,1"), F("OK") ,10000);i++);
  delay(100);
  for (int i = 0; i < 3 && !fona.sendCheckReply(F("AT+CHTTPSSEND=120"), F(">"), 10000); i++);
  delay(100);
  for (int i = 0; i < 3 && !fona.sendCheckReply(F("POST /ollie/sensor/data"), F("OK"), 10000); i++); //Should be reqArr, but Go server isn't updated to use JSON posts yet
  delay(100);
  for(int i=0;i<3 && !fona.sendCheckReply(F("AT+CHTTPSCLSE"),F("OK"),10000);i++);
  delay(100);

  free(reqArr);
  
  Serial.print(F("13: "));
  Serial.println(freeMemory());
  
  fonaSS.end();
  dataGood = false;
}

//Build the HTTP request to send through fona
//If I'm completely honest, this function is currently garbage
//Some of the code is just wrong, a bunch of the assumptions are wrong, and it should never use String objects
char* buildCharRequest() {
  Serial.print(F("10.2: "));
  Serial.println(freeMemory());

  //Get the data to Construct the string
  
  byte numId = EEPROM.read(0);

  int strLen = 101; //POST overhead
  strLen += 4 * numId - 1;
  strLen += 17;
  strLen += 23 * numId - 1; //Guess at type length
  strLen += 11;
  strLen += 6 * numId - 2; //the code for this needs to be fixed
  strLen += 11;
  strLen += 10 * numId - 2; //the code for this needs to be fixed
  strLen += 3;

  char* reqArr = calloc(sizeof(char), (strLen + 20));

  //Should put next 3 for loops into 1
  int Id[numId];
  int readPos = 1;
  for (byte i = 0; i < numId; i++) {
    EEPROM.get(readPos, Id[i]);
    readPos += 2;
    byte len = EEPROM.read(readPos);
    readPos += 1 + len;
    len = EEPROM.read(readPos);
    readPos += 1 + len;
  }
    
  char* Type[numId];
  readPos = 3;
  for (byte i = 0; i < numId; i++) {
    byte len = EEPROM.read(readPos);
    readPos++;
    char *innerType = malloc(sizeof(char) * len+1);
    for (byte j = 0; j < len; j++) {
      innerType[j] = EEPROM.read(readPos+j);
    }
    innerType[len] = 0;
    Type[i] = innerType;
    readPos += len;
    len = EEPROM.read(readPos);
    readPos += 1 + len + 2;
  }
  
  float *Data[numId];
  unsigned long *Time[numId];
  readPos = 3;
  for (byte i = 0; i < numId; i++) {
    byte len = EEPROM.read(readPos);
    readPos += 1 + len;
    len = EEPROM.read(readPos);
    EEPROM.write(readPos, 0);
    readPos++;
    float *innerData = malloc(sizeof(float) * len);
    unsigned long *innerTime = malloc(sizeof(unsigned long) * len);
    for (byte j = 0; j < len; j++) {
      EEPROM.get(readPos, innerData[j]);
      EEPROM.get(readPos+4, innerTime[j]);
      readPos += 8;
    }
    Data[i] = innerData;
    Time[i] = innerTime;
    readPos += 2;
  }


  
  //Construct the request as a String
  
  String req;
  req.reserve(strLen + 20);
  req = F("POST /ollie/sensor/data HTTPS/1.1\r\nHost: www.cas.mcmaster.ca\r\nContent-Type: application/json\r\n{\"id\":[");

  for (byte i = 0; i < numId; i++) {
    req += (String) F("\"") + (String) Id[i] + (String) F("\"");
    if (!(i+1 == numId)) {
      req += (String) F(",");
    }
  }
  
  req += (String) F("],\"measurement\":[");

  for (byte i = 0; i < numId; i++) {
    req += (String) F("\"") + (String) Type[i] + (String) F("\"");
    Serial.print(F("TYPE: "));
    Serial.println((String) Type[i]);
    if (!(i+1 == numId)) {
      req += (String) F(",");
    }
  }
  
  req += (String) F("],\"data\":[[");

  for (byte i = 0; i < numId; i++) {
    for (int j = 0; j < 1; j++) {
      req += (String) Data[i][j];
      Serial.print(F("DATA: "));
      Serial.println((String) Data[i][j]);
      if (!(j+1 == 1)) {
        req += (String) F(",");
      }
    }
    req += (String) F("]");
    if (!(i+1 == numId)) {
        req += (String) F(",[");
      }
  }

  req += (String) F("],\"time\":[[");
  
  for (byte i = 0; i < numId; i++) {
    for (int j = 0; j < 1; j++) {
      req += (String) Time[i][j];
      Serial.print(F("TIME: "));
      Serial.println((String) Time[i][j]);
      if (!(j+1 == 1)) {
        req += (String) F(",");
      }
    }
    req += (String) F("]");
    if (!(i+1 == numId)) {
        req += (String) F(",[");
      }
  }

  req += F("]} ");

  

  //Free some stuff, put string into char array and return
  
  Serial.print(F("11: "));
  Serial.println(freeMemory());

  for (int i = 0; i < numId; i++) {
    free(Type[i]);
    free(Data[i]);
    free(Time[i]);
  }

  Serial.print(F("11.2: "));
  Serial.println(freeMemory());

  for (int i = 0 ; i < req.length(); i++) {
    reqArr[i] = req[i];
    Serial.print(reqArr[i]);
  }
  Serial.println();

  return reqArr;
}


//Lower level Lora stuff:

void printByte(uint8_t b) {
  Serial.print(F(" 0x"));
  if (b <= 0xF)
    Serial.print(F("0"));
  Serial.print(b, HEX);
}

void printShort(uint16_t s) {
  Serial.print(F(" 0x"));
  if (s <= 0xFFF)
    Serial.print(F("0"));
  if (s <= 0xFF)
    Serial.print(F("0"));
  if (s <= 0xF)
    Serial.print(F("0"));
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

  printByte(frameType);
  printByte((uint8_t) 0); // frameNum
  printByte(cmdType);
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
  printByte(checksum);

  Serial.println();
}

uint8_t* readData(Stream& port) {
  uint8_t frameType = 0;
  uint8_t cmdType = 0;
  uint8_t* payload = NULL;
  int len = readFrame(port, &frameType, &cmdType, &payload);

  if (frameType != 0x05 || cmdType != 0x82) {
    Serial.println(F("BAD TYPE"));
    free(payload);
    return;
  }

  //uint16_t srcAddr = (payload[0] << 8) | payload[1];
  //uint8_t power = payload[2];
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
    Serial.print(F(" targetAddr=")); printShort(targetAddr);
    Serial.print(F(" status=")); printByte(status);
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
    Serial.println(F("ERROR"));
    free(payload);
    return;
  }

  Serial.print(F("FrameType="));
  printByte(frameType);
  Serial.print(F(" CmdType="));
  printByte(cmdType);

  if (frameType != 0x01 || cmdType != 0x82) {
    Serial.println(F("BAD TYPE"));
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
  free(payload);

  Serial.print(F(" flag=")); printShort(flag);
  Serial.print(F(" channel=")); printByte(channel);
  Serial.print(F(" txPower=")); printByte(txPower);
  Serial.print(F(" uiMode=")); printByte(uiMode);
  Serial.print(F(" eqType=")); printByte(eqType);
  Serial.print(F(" netId=")); printShort(netId);
  Serial.print(F(" nodeId=")); printShort(nodeId);
  Serial.print(F(" serPortParam=")); printByte(serPortParam);
  Serial.print(F(" airRate=")); printShort(airRate);
  Serial.println();
  return nodeId;
}
