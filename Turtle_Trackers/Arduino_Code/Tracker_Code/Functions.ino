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

  if (!LoRa.begin(LORA_FREQ)) {
    DEBUG_SERIAL.println(F("LoRa init failed. Check your connections."));
    while (true);
  }
  // LoRa.setSignalBandwidth(7.8E3); // 
  // LoRa.setSpreadingFactor(12); // maximum SF to get longest range
  LoRa.setTxPower(LORA_TX_POWER); // maximum tx power to get longest range
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa.enableCrc(); // Enables the LoRa module's built in error checking
  pinMode(LoRa_RESET, OUTPUT);

  DEBUG_SERIAL.println(F("Initialize Successfully."));
}

/* ------------------------ GPS ------------------------- */

// Config GPS Module
void configGPS(){
  pinMode(GPS_EN, OUTPUT);
  gpsPort.begin(GPS_BAUDRATE);
  enableGPS();
  delay(1000); // Make sure the GPS is fully turned on (ms)
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
void readGPSValid(){
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
  #if INVERT_IQ_MODE
  LoRa.enableInvertIQ();                // active invert I and Q signals
  #endif
  LoRa.receive();                       // set receive mode
}

// Turn LoRa into transmit mode
void LoRa_txMode(){
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
  LoRa.sleep(); // test
  resetLoRa(); // Turn off LoRa hardware before sleep
  delay(100);
  for (uint8_t i = 0; i < sleep_cycles; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); // Sleeps the tracker for ~SLEEP_CYCLES*8 mins
  }
  DEBUG_SERIAL.println(F("lowPowerMode End."));
}

// Double the sleep cycle x2
void doubleSleepCycle(){
  if (trackerSleepCycles == MAX_SLEEP_CYCLES) // If already the maximum sleep cycle, nothing changed
    return;
  trackerSleepCycles *= 2; // Double sleep cycle
  if (trackerSleepCycles > MAX_SLEEP_CYCLES) // If sleep cycle is larger than maximum, change it to max time
    trackerSleepCycles = MAX_SLEEP_CYCLES;
}

/* ------------------------ LoRa Buff ------------------------- */
// A simple cycle buff implementation acting as datpoint buff.
// Write the vlaue into the buffer
// return false if buf is full, otherwise return true
boolean writeBuf(circularBuffer* buf, uint8_t value) {
  // write i.e. mem[] = value
  if (buf->bufLen == BUF_SIZE) {
    DEBUG_SERIAL.println("Buff is full!");
    return false;
  }

  // WRITE
  buf->bufArray[buf->writeIndex] = value;
  #if DEBUG
  DEBUG_SERIAL.print("Write into buffer: " );
  printByte(value);
  DEBUG_SERIAL.println();
  #endif
  
  (buf->bufLen)++;
  (buf->writeIndex)++;
  if (buf->writeIndex == BUF_SIZE) {
    buf->writeIndex = 0;
  }
  return true;
}

// Read one element from the buffer, after reading that element will be lazy deleted
// return false if buf is empty, otherwise return true
boolean readOneFromBuf(circularBuffer* buf) {
  // read i.e lora.wirte()
  if (buf->bufLen == 0) {
    DEBUG_SERIAL.println("Buffer is empty!");
    return false;
  }

  // READ
  #if DEBUG
  DEBUG_SERIAL.print("Read from buffer: " );
  printByte(buf->bufArray[buf->readIndex]);
  DEBUG_SERIAL.println();
  #endif
  
  (buf->bufLen)--;
  (buf->readIndex)++;
  if (buf->readIndex == BUF_SIZE) {
    buf->readIndex = 0;
  }
  return true;
}

// Read n element from the buffer, after reading that element will be lazy deleted
// return false if buf is empty, otherwise return true
boolean readNFromBuf(circularBuffer* buf, uint8_t n) {
  if (buf->bufLen - n < 0) {
    DEBUG_SERIAL.println("ERROR: Not enough remaining elements to be removed.");
    return false;
  }

  // otherwise, update bufLen and readIndex correspondingly (lazy delection).
  (buf->bufLen) = (buf->bufLen) - n;
  (buf->readIndex) = ((buf->readIndex) + n) % BUF_SIZE;
}

// Test if the buff is full
boolean isFullBuf(circularBuffer* buf) {
  return (buf->bufLen == BUF_SIZE);
}

// Write the whole buff into lora message 
// Usage:
//  LoRa.beginPacket();
//  LoRa_writeFromBuff(&loraBuf);
//  LoRa.endPacket(true);    
void LoRa_writeFromBuff(circularBuffer* buf) {
  DEBUG_SERIAL.println("-------- Buff sent:  ----------");
  for (uint8_t i = 0; i < buf->bufLen; i++) {
    uint8_t addr = ((buf->readIndex) + i) % (buf->bufLen); // calculate the address in physical memory
    LoRa.write((uint8_t) (buf->bufArray)[addr]);
    #if DEBUG
    printByte((uint8_t) (buf->bufArray)[addr]);
    #endif
  }    
  DEBUG_SERIAL.println();
  DEBUG_SERIAL.println("-------------------------------");             
}


/* ------------------------ Helper for Debug ------------------------- */

#if DEBUG
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

// Show the content of the physical memory of the bufArray (in physical order)
void showBuf(circularBuffer* buf) {
  Serial.println("------------------");
  Serial.print("writeIndex :");
  Serial.println(buf->writeIndex);
  Serial.print("readIndex :");
  Serial.println(buf->readIndex);
  Serial.print("bufLen :");
  Serial.println(buf->bufLen);
  Serial.println("Content: ");
  for (uint8_t i = 0; i < BUF_SIZE; i++) {
    printByte((buf->bufArray)[i]);
  }
  Serial.println();
  Serial.println("------------------");
}
#endif
