/*
  Turtle Tracker Arduino code used in the re-mote setup found at 
  https://gitlab.cas.mcmaster.ca/re-mote
*/

/* ------------------------ Tracker (general) ------------------------- */

// Initialize Serial, GPS, and LoRa.
void initilaize() {
  DEBUG_SERIAL.begin(PC_BAUDRATE); // Start Debug Serial
  DEBUG_SERIAL.println(F("Initializing..."));

  configGPS();

  if (!LoRa.begin(915E6)) {
    DEBUG_SERIAL.println(F("LoRa init failed. Check your connections."));
    while (true);
  }
  // LoRa.setSignalBandwidth(7.8E3); // test
  // LoRa.setSpreadingFactor(12); // maximum SF to get longest range
  LoRa.setTxPower(LORA_TX_POWER); // maximum tx power to get longest range
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  // LoRa.enableCrc(); // Enables the LoRa module's built in error checking

  pinMode(LoRa_RESET, OUTPUT);

  DEBUG_SERIAL.println(F("Initialize Successfully."));
}

// Send registration message
void sendRegistration() {
  DEBUG_SERIAL.println(F("Trying to send registration..."));
  // allocate space for message
  uint8_t* message = (uint8_t*) malloc(sizeof(uint8_t) * REG_LEN);
  // write message type and sensor#
  message[0] = (uint8_t) 0xc0;
  // write node id
  message[1] = (uint8_t) NODE_ID;
  
  // write unix time (HARDCODED)
  // In [4]: int('0xca35115d', base=16)
  // Out[4]: 3392475485
  // !!! reverse <= little edian
  message[2] = (uint8_t) 0x5D;
  message[3] = (uint8_t) 0x11;
  message[4] = (uint8_t) 0x35;
  message[5] = (uint8_t) 0xCA;
  
  // send message
  LoRa_sendMessage(message, REG_LEN);
  // free variable
  free(message);
}

// Send sensor data message
void sendSensorData() {
  // read data
  unsigned long unixTime = (NeoGPS::clock_t) fix.dateTime + 946684800; // 32 bits i.e 4 bytes
  long latitude = fix.latitudeL(); // 32 bits i.e 4 bytes
  long longitude = fix.longitudeL(); // 32 bits i.e 4 bytes
//  unsigned long unixTime = 1656555632; // 32 bits i.e 4 bytes
//  long latitude = 432582727; // 32 bits i.e 4 bytes
//  long longitude = -799207620; // 32 bits i.e 4 bytes

  // send data
  DEBUG_SERIAL.println(F("Trying to send sensor data..."));
  uint8_t* message = (uint8_t*) malloc(sizeof(uint8_t) * SEN_LEN);
  // write message type and sensor#
  message[0] = (uint8_t) 0xd0;
  // write node id
  message[1] = (uint8_t) NODE_ID;
  uint8_t* temp; // a helper pointer used for memcpy().
  // write unix time
  temp = (uint8_t*) &unixTime;
  memcpy(&message[2], temp, sizeof(uint8_t) * 4);
  // write latitude
  temp = (uint8_t*) &latitude;
  memcpy(&message[6], temp, sizeof(uint8_t) * 4);
  // write longitude
  temp = (uint8_t*) &longitude;
  memcpy(&message[10], temp, sizeof(uint8_t) * 4);
  // send message
  LoRa_sendMessage(message, SEN_LEN);
  free(message);
}

/* ------------------------ GPS ------------------------- */

// Config GPS Module
void configGPS(){
  pinMode(GPS_EN, OUTPUT);
  gpsPort.begin(GPS_BAUDRATE);
  enableGPS();
  delay(1000);
  TurtleTracker_UBX_2022 turtleTracker_UBX(Serial,gpsPort);
  turtleTracker_UBX.configGPS();
}

// Enable GPS Module
void enableGPS(){
  digitalWrite(GPS_EN, HIGH);
  gpsPort.listen();
}

// Disable GPS Module
void disableGPS(){
  digitalWrite(GPS_EN, LOW);
}

// Try to get GPS fix data and stored into fix variable
void readGPS() {
  while (gps.available(gpsPort)) {
    fix = gps.read();
  }
}

// Get vaild data from GPS and stored into fix variable
void readGPSvaild(){
  // Run the GPS for some time to try and get a fix
  long start = millis();
  DEBUG_SERIAL.println(F("Reading GPS..."));
  while (millis() - start < GPS_TIMEOUT && (!fix.valid.location || !fix.valid.time || fix.dateTime == lastTime)) {
    readGPS();
  }
  if (fix.valid.time)
    lastTime = fix.dateTime; // Save the timestanp for the last GPS fix data
}

// Check if the fix variable is vaild
bool ifVaildFix(){
  return (fix.valid.location && fix.valid.time);
}

/* ------------------------ LoRa ------------------------- */

// Reset the RFM95 Module, turn it off before sleep
void resetLoRa(){
  digitalWrite(LoRa_RESET, HIGH);
  delay(200);
  digitalWrite(LoRa_RESET, LOW);
}

// Turn LoRa into receive mode
void LoRa_rxMode(){
//  DEBUG_SERIAL.println("MODE: RX -- i.e. receive");
  #if INVERT_IQ_MODE
  LoRa.enableInvertIQ();                // active invert I and Q signals
  #endif
  LoRa.receive();                       // set receive mode
}

// Turn LoRa into transmit mode
void LoRa_txMode(){
//  DEBUG_SERIAL.println("MODE: TX -- i.e. idle");
  LoRa.idle();                          // set standby mode
  #if INVERT_IQ_MODE
  LoRa.disableInvertIQ();               // normal mode
  #endif
}

// Use LoRA send a memory block pointed by message with length messageLength
void LoRa_sendMessage(uint8_t* message, uint8_t messageLength) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  // add payload
  for (uint8_t i = 0; i < messageLength; i++) {
    LoRa.write((uint8_t) message[i]);
  }                  
  LoRa.endPacket(true);                 // finish packet and send it
}

/* ------------------------ Low Power ------------------------- */

// Turn off GPS, LoRa and sleep the arduino
void enterLowPowerMode(uint8_t sleep_cycles){
  DEBUG_SERIAL.print("lowPowerMode Start. with sleep cyles = ");
  DEBUG_SERIAL.println(sleep_cycles);
  disableGPS(); // Make sure GPS is off before sleep
//  LoRa.end(); // Ture off LoRa SPI before sleep
  LoRa.sleep(); // test
  resetLoRa(); // Ture off LoRa hardware before sleep
  delay(100);
  for (uint8_t i = 0; i < sleep_cycles; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); // Sleeps the tracker for ~SLEEP_CYCLES*8 mins
  }
  DEBUG_SERIAL.println(F("lowPowerMode End."));
}

/* ------------------------ LoRa Buff ------------------------- */
// A simple cycle buff implementation acting as datpoint buff.

// Write a uint8_t value into the buff. If the buff is full, do nothing.
void writeBuf(uint8_t* buf, int* writeIndex, int* bufLen, uint8_t value) {
  // write i.e. mem[] = value
  if (*bufLen == BUF_SIZE) {
    DEBUG_SERIAL.println(F("Buff is full!"));
    return;
  }

  // WRITE
  buf[*writeIndex] = value;
  #if DEBUG == true
  DEBUG_SERIAL.print(F("Write into buffer: "));
  printByte(value);
  DEBUG_SERIAL.println();
  #endif  //DEBUG == true
  
  (*bufLen)++;
  (*writeIndex)++;
  if (*writeIndex == BUF_SIZE) {
    *writeIndex = 0;
  }
}

// Read the value at readIndex of the buff.
void readBuf(uint8_t* buf, int* readIndex, int* bufLen) {
  // read i.e lora.wirte()
  if (*bufLen == 0) {
    DEBUG_SERIAL.println(F("Buffer is empty!"));
    return;
  }

  // READ
  DEBUG_SERIAL.print(F("Read from buffer: "));
  #if DEBUG == true
  printByte(buf[*readIndex]);
  #endif  //DEBUG == true
  DEBUG_SERIAL.println();
  
  (*bufLen)--;
  (*readIndex)++;
  if (*readIndex == BUF_SIZE) {
    *readIndex = 0;
  }
}

// Test if the buf is full.
boolean isFullBuf(int bufLen, int maxBufLen) {
  if (bufLen == maxBufLen) {
    return true;
  } else {
    return false;
  }
}

// "fake" reading n elements (i.e. lazy deleting n elements from the buff)
void lazyDeleteNFromBuf(uint8_t* buf, int* readIndex, int* bufLen, int n) {
  if ((*bufLen) - n < 0) {
    DEBUG_SERIAL.println(F("ERROR: Not enough remaining elements to be removed."));
    return;
  }

  // otherwise, update bufLen and readIndex correspondingly (lazy delection).
  (*bufLen) = (*bufLen) - n;
  (*readIndex) = ((*readIndex) + n) % BUF_SIZE;
}

// Read buff into lora message 
// Usage:
//  LoRa_txMode();
//  LoRa.beginPacket();
//  LoRa_writeFromBuff(&buf[0], &readIndex, &bufLen)
//  LoRa.endPacket(true);    
void LoRa_writeFromBuff(uint8_t* buf, int* readIndex, int* bufLen) {
  DEBUG_SERIAL.println(F("-------- Buff sent:  ----------"));
  for (uint8_t i = 0; i < *bufLen; i++) {
    int addr = ((*readIndex) + i) % (*bufLen); // calculate the address in physical memory
    LoRa.write((uint8_t) buf[addr]);
    #if DEBUG == true
    printByte((uint8_t) buf[addr]);
    #endif  //DEBUG == true
  }    
  DEBUG_SERIAL.println();
  DEBUG_SERIAL.println(F("-------------------------------"));             
}

/* ------------------------ Helper for Debug ------------------------- */

#if DEBUG == true
// Prints a byte as hex to the Serial port
void printByte(uint8_t b) {
  DEBUG_SERIAL.print(F(" 0x"));
  if (b <= 0xF)
    DEBUG_SERIAL.print(F("0"));
  DEBUG_SERIAL.print(b, HEX);
}

// Pause the program for debuging
void pause(){
  Serial.println(F("Wait input (G): "));
  while(true){
    if(Serial.available() > 0)
      if(Serial.read() == 'G')
        break;
  }
}

// Prints various GPS data
void printLocationData(){
  DEBUG_SERIAL.print(F("Status: "));
  DEBUG_SERIAL.println(fix.status);
  delay(100);
  DEBUG_SERIAL.print(F("valid.location: "));
  DEBUG_SERIAL.println(fix.valid.location);
  delay(100);
  DEBUG_SERIAL.print(F("latitudeL: "));
  DEBUG_SERIAL.println(fix.latitudeL());
  delay(100);
  DEBUG_SERIAL.print(F("longitudeL: "));
  DEBUG_SERIAL.println(fix.longitudeL());
  delay(100);
  DEBUG_SERIAL.print(F("dateTime: "));
  DEBUG_SERIAL.println(fix.dateTime);
  delay(100);
}

// Show the physical memory content representing the LoRa buff for debug
void showBuf(uint8_t* buf, int* writeIndex, int* readIndex, int* bufLen) {
  DEBUG_SERIAL.println(F("------------------"));
  DEBUG_SERIAL.print(F("writeIndex :"));
  DEBUG_SERIAL.println(*writeIndex);
  DEBUG_SERIAL.print(F("readIndex :"));
  DEBUG_SERIAL.println(*readIndex);
  DEBUG_SERIAL.print(F("bufLen :"));
  DEBUG_SERIAL.println(*bufLen);
  DEBUG_SERIAL.println(F("Content: "));
  for (int i = 0; i < BUF_SIZE; i++) {
    printByte(buf[i]);
  }
  DEBUG_SERIAL.println();
  DEBUG_SERIAL.println(F("------------------"));
}
#endif  //DEBUG == true
