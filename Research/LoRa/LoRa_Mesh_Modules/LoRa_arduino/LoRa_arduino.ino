#include <SoftwareSerial.h>

#define MCU_LORA_RX 7 // Connects to LoRa TX, also is D7
#define MCU_LORA_TX 8 // Conencts to LoRa RX, also is D8

SoftwareSerial loraPort = SoftwareSerial(MCU_LORA_RX, MCU_LORA_TX);
uint8_t counter = 0;

void setup() {
  pinMode(MCU_LORA_RX, INPUT);
  pinMode(MCU_LORA_TX, OUTPUT);
  loraPort.begin(9600);
  Serial.begin(9600);

  delay(3000);

  readConfig(loraPort);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)

  //readConfig(loraPort);
    counter = (counter + 1) % 100;
//    uint8_t* data = malloc(sizeof(uint8_t)*4);
//    data[0] = counter;
//    data[1] = counter + 1;
//    data[2] = counter + 2;
//    data[3] = counter + 3;
//    //uint8_t* data = &counter;
//    sendData(loraPort, 0x1024, 4, data); //1024
    readData(loraPort);
//  if (counter == 0) {
//    readConfig(loraPort);
//    counter++;
//  }
  delay(1000);

  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
}

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

// Commands

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

// Max len is 111 bytes
void sendData(Stream& port, uint16_t target, uint8_t dataLen, uint8_t* data) {
  // We add 7 bytes to the head of data for this payload
  uint8_t payloadLen = 7 + dataLen;
  uint8_t* payload = (uint8_t *) malloc(sizeof(uint8_t) * payloadLen);

  // target address as big endian short
  payload[0] = (uint8_t) ((target >> 8) & 0xFF);
  payload[1] = (uint8_t) (target & 0xFF);

  // ACK request == 1 -> require acknowledgement of recv
  payload[2] = (uint8_t) 0;1;

  // Send radius: which defaults to max of 7 hops, we can use that
  payload[3] = (uint8_t) 7;

  // Discovery routing params == 1 -> automatic routing
  payload[4] = (uint8_t) 1;

  // Source routing domain: unused when automatic routing enabled
  //    - number of relays is 0
  //    - relay list is therefor non-existent
  payload[5] = (uint8_t) 0;

  // Data length
  payload[6] = dataLen;

  // Data from index 7 to the end should be the data
  memcpy(payload + (sizeof(uint8_t) * 7), data, dataLen);

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

void readData(Stream& port) {
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
  Serial.print(" srcAddr="); printShort(srcAddr);
  Serial.print(" power="); printByte(power);
  Serial.print(" userPayloadLength="); printByte(userPayloadLength);
  for (int i = 0; i < userPayloadLength; i++)
    printByte(payload[4 + i]);

  Serial.println();
  
  free(payload);
}

void readConfig(Stream& port) {
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
}

