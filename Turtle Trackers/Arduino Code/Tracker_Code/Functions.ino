/*
  Turtle Tracker Arduino code used in the re-mote setup found at 
  https://gitlab.cas.mcmaster.ca/re-mote
*/

/* ------------------------ Tracker (general) ------------------------- */

// Initialize Serial, GPS, and LoRa.
void initilaize() {
  Serial.println("Initializing...");
  
  pinMode(GPS_EN, OUTPUT);

  LoRa.setTxPower(LORA_TX_POWER);
  Serial.begin(9600);
  while (!Serial);

  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);

  gpsPort.begin(9600);

  Serial.println("Initialize Successfully.");
}

// Send registration message
void sendRegistration() {
  Serial.println("Trying to send registration...");
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
//  unsigned long unixTime = 3392475485; // 32 bits i.e 4 bytes
//  long latitude = 123456789; // 32 bits i.e 4 bytes
//  long longitude = 987654321; // 32 bits i.e 4 bytes

  // send data
  Serial.println("Trying to send sensor data...");
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
  DEBUG_SERIAL.println("Reading GPS...");
  while (millis() - start < GPS_TIMEOUT && (!fix.valid.location || !fix.valid.time || fix.dateTime == lastTime)) {
    readGPS();
  }
  if (fix.valid.time)
    lastTime = fix.dateTime; // Save the timestanp for the last GPS fix data
}

// Check if the fix variable is vaild
bool ifVaildFix(){
  return (fix.valid.location || fix.valid.time);
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
void enterLowPowerMode(){
  DEBUG_SERIAL.println("lowPowerMode Start.");
  disableGPS(); // Make sure GPS is off before sleep
  LoRa.end(); // Ture off LoRa SPI before sleep
  resetLoRa(); // Ture off LoRa hardware before sleep
  delay(100);
//  for (int i = 0; i < sleep_cycles; i++) {
  for (int i = 0; i < 3; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); // Sleeps the tracker for ~SLEEP_CYCLES*8 mins
  }
  DEBUG_SERIAL.println("lowPowerMode End.");
}

/* ------------------------ Helper for Debug ------------------------- */

// Prints a byte as hex to the Serial port
void printByte(uint8_t b) {
  Serial.print(F(" 0x"));
  if (b <= 0xF)
    Serial.print(F("0"));
  Serial.print(b, HEX);
}

// Pause the program for debuging
void pause(){
  DEBUG_SERIAL.println("Wait input (G): ");
  while(true){
    if(Serial.available() > 0)
      if(Serial.read() == 'G')
        break;
  }
}

// Prints various GPS data
void printLocationData(){
  DEBUG_SERIAL.print("Status: ");
  DEBUG_SERIAL.println(fix.status);
  delay(100);
  DEBUG_SERIAL.print("valid.location: ");
  DEBUG_SERIAL.println(fix.valid.location);
  delay(100);
  DEBUG_SERIAL.print("latitudeL: ");
  DEBUG_SERIAL.println(fix.latitudeL());
  delay(100);
  DEBUG_SERIAL.print("longitudeL: ");
  DEBUG_SERIAL.println(fix.longitudeL());
  delay(100);
  DEBUG_SERIAL.print("dateTime: ");
  DEBUG_SERIAL.println(fix.dateTime);
  delay(100);
}
