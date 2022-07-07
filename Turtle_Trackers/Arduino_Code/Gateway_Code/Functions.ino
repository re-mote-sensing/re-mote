/*
  Turtle Tracker Arduino code used in the re-mote setup found at 
  https://gitlab.cas.mcmaster.ca/re-mote
*/

/* ------------------------ Server Post ------------------------- */

// Post ToSend.bin data to server
// return true : Sent
// return false : Empty / Error
bool postToServerWithBuffer(){
  int lineCount = 1; // skip the first lines from csv (Gateway ID)
  while (true){
    String line = getPostBufferFromSdBin(lineCount, POST_AMOUNT); // Read line from ToSend.bin
    line.trim(); // Delete the trailing character
    if (line=="END") // If EOF
      break;
    else if (line=="SKIP"){ // If Blank line / error
      lineCount++;
      continue;
    }
    else if (line=="ETY") // If Empty File
      return false;

    if (lineCount == 1) // Only start on the first loop
      start3GHTTP(); // Start 3G HTTP
    
    lineCount+=POST_AMOUNT; // Add the lines that already read
    
    DEBUG_SERIAL.print("Sending... : ");
    DEBUG_SERIAL.println(line);

    // Open HTTP Session
    char sessCommand[CHTTPSOPSE_LENGTH];
    sprintf(sessCommand, "AT+CHTTPSOPSE=\"%s\",%d,1", HOST, SERVER_PORT);
    long start = millis();
    while (sendCommand(sessCommand, 5000, true).equals("ERROR"))
      if (millis() - start < SIM3G_HTTP_TIMEOUT){
        stop3GHTTP();
        return false;
      }
      
    // Payload
    char msgBody[msgBody_LENGTH];
    sprintf(msgBody, "b=%s", line.c_str());
  
    // POST Command
    char command[POST_COMMAND_LENGTH];
    sprintf(command, "POST %s?%s HTTP/1.1\r\nHost: %s\r\n\r\n", ENDPOINT, msgBody, HOST);

    // Send HTTP Request
    char preCommand[64];
    sprintf(preCommand, "AT+CHTTPSSEND=%d", strlen(command));
    sendCommand(preCommand, 250, true);

    // Receive HTTP Response
    if (sendCommand(command, 5000, true).equals("NO CARRIER")){
      DEBUG_SERIAL.println(F("HTTP NO CARRIER"));
      sendCommand("AT+CHTTPSCLSE", 2000, false);
      stop3GHTTP();
      return false;
    }
    sendCommand("AT+CHTTPSRECV?", 2000, true);
    sendCommand("AT+CHTTPSRECV=1000", 3000, true);
    sendCommand("AT+CHTTPSCLSE", 2000, false);

    DEBUG_SERIAL.println(F("HTTP END"));
  }
  stop3GHTTP();
  DEBUG_SERIAL.println(F("Done Posting to Server"));
  return true;
}

// Get $postAmount lines from ToSend.bin begin with line $lineCount
// return END : End of file
// return SKIP : Blank line
// return ETY : Empty File
String getPostBufferFromSdBin(int lineCount, int postAmount){
  SdFat sd;
  if (!sd.begin(SD_CONFIG)) {
    DEBUG_SERIAL.println(F("SD Error."));
    return "END";
  }
  if (sd.exists("ToSend.bin")) {
    File file;
    if (!file.open("ToSend.bin", FILE_READ)) { // Open ToSend.bin
      DEBUG_SERIAL.println(F("Open BIN Failed"));
      return "END";
    }
    char line[BIN_LINE_LENGTH]; // Store a line of bin here
    for (int i = 0; i < lineCount; i++) 
      if (file.available())
        file.fgets(line, sizeof(line)); // Skip lines that already read / ignore

    int readCounter = 0; // Track how many line has read
    String dataString=""; // Used for building the return string
    while (file.available()) {
      int n = file.fgets(line, sizeof(line)); // Read line
      if (n <= 0 || (line[n-1] != '\n' && n == (sizeof(line) - 1))) {
        DEBUG_SERIAL.println(F("fgets failed or line too long"));
        return "SKIP";
      }
      readCounter++; // Add counter
      dataString += line; // Combine line with previous read
      dataString.trim();
      if (readCounter>=postAmount) // If reach defined amount, return dataString
        break;
    }
    file.close();
    if (readCounter >= 1) // If read atleast 1 line, return the result
      return dataString;
    
  }else{
    DEBUG_SERIAL.println(F("ToSend.bin Not Exist in getPostBufferFromSdBin"));
    return "ETY";
  }
  if (lineCount == 1) // If no data is in the file
    return "ETY";
  return "END";
}

/* ------------------------ 3G ------------------------- */

// Power On 3G Shield
void powerOn3G(){
  digitalWrite(SIM3G_EN, HIGH);
  delay(SIM3G_POWER_ON_TIME);
  digitalWrite(SIM3G_EN, LOW);
}

// Power Off 3G Shield
void powerOff3G(){
  digitalWrite(SIM3G_EN, HIGH);
  delay(SIM3G_POWER_OFF_TIME);
  digitalWrite(SIM3G_EN, LOW);
}

// Start 3G Shield HTTP Request
void start3GHTTP() {
  powerOn3G();
  delay(7000);
  sendCommand("AT", 250, true);
  sendCommand("ATE0", 250, true);
  delay(1000);
  sendCommand("AT+CMEE=2", 250, true);
  sendCommand("AT+CGDCONT=1,\"IP\",\"pda.bell.ca\"", 250, true);
  sendCommand("AT+CGSOCKCONT=1,\"IP\",\"pda.bell.ca\"", 250, true);
  sendCommand("AT+CSOCKSETPN=1", 250, true);
  
  while (sendCommand("AT+CHTTPSSTART", 10000, true).equals(F("ERROR"))) {
    sendCommand("AT+CHTTPSCLSE", 10000, false);
    sendCommand("AT+CHTTPSSTOP", 10000, false);
    sendCommand("AT", 250, true);
    sendCommand("ATE0", 250, true);
    delay(1000);
    sendCommand("AT+CMEE=2", 250, true);
    sendCommand("AT+CGDCONT=1,\"IP\",\"pda.bell.ca\"", 250, true);
    sendCommand("AT+CGSOCKCONT=1,\"IP\",\"pda.bell.ca\"", 250, true);
    sendCommand("AT+CSOCKSETPN=1", 250, true);
  }
  delay(500);
}

// Stop 3G Shield HTTP Request
void stop3GHTTP() {
  sendCommand("AT+CHTTPSSTOP", 2000, false);
  powerOff3G();
}

// Send Command via Serial
String sendCommand(const char *command, unsigned long timeout, bool multiLine) {
  String result;
  DEBUG_SERIAL.print(F(">> "));
  DEBUG_SERIAL.println(command);
  ss.println(command);
  unsigned long start =  millis();
  DEBUG_SERIAL.print(F("<< "));
  int count = 0;
  bool firstLineRead = false;
  while(millis() - start < timeout) {
    if (ss.available()) {
      char c = ss.read();
      if (c==0x0D); // 0x0D = \r
      else if (c == 0x0A ){ // 0x0A = \n
        if (firstLineRead == false) // first 0x0A is ignored
          firstLineRead = true;
        else if (!multiLine) // If only needs one line, return before timeout
          break; // second 0x0A is the end of the line
      }
      else {
        DEBUG_SERIAL.write(c);
        result += c;
        count++;
      }
    }
  }
  DEBUG_SERIAL.println();
  result.trim();
  return result;
}

/* ------------------------ SD Card ------------------------- */

// Create a ToSend.bin file in SD Card
void createToSendFile(){
  SdFat sd;
  if (!sd.begin(SD_CONFIG)) {
    DEBUG_SERIAL.println(F("SD Error."));
    return;
  }
  if (!sd.exists("ToSend.bin")) {
    File file;
    if (!file.open("ToSend.bin", FILE_WRITE)) { // Create ToSend.bin
      DEBUG_SERIAL.println(F("Open CSV Failed"));
      return;
    }
    file.print(GATEWAY_ID); // Write ID in first line
    file.close();
  }else{
    DEBUG_SERIAL.println(F("ToSend.bin Existed"));
  }
  DEBUG_SERIAL.println(F("SD Ok"));
}

// Get a HEX string from uint8_t
String getByte(uint8_t b) {
  String s = "";
  if (b <= 0xF)
    s+="0";
  return s+=String(b,HEX);
}

// Add Tracker data into SD card
// return true : success
// return false : failed
bool addDataByte(String count, uint8_t type, uint8_t nodeID, uint8_t* unixTime, uint8_t* latitude, uint8_t* longitude) {
  SdFat sd;
  if (!sd.begin(SD_CONFIG)) {
    DEBUG_SERIAL.println(F("SD Error"));
    return false;
  }
  
  // Get file name for tracker csv log
  char* fileName = getFileName(nodeID);
  if (fileName == NULL)
    return false;

  // Save data to tracker csv t<NodeID>.csv
  if (sd.exists(fileName)) {
    // Create csv file
    File file;
//    free(fileName);
//    char* fileName = getFileName((char)nodeID); // Get name again
//    if (fileName == NULL)
//      return false;
    if (!file.open(fileName, FILE_WRITE)) { // Open Tracker data CSV
      DEBUG_SERIAL.print(F("Open CSV Failed: "));
      DEBUG_SERIAL.println(fileName);
      free(fileName);
      return false;
    }

    // Data added to CSV
    file.print("\r\n");
    file.print(count);
    file.print(",");
    file.print(String(* (unsigned long*) unixTime));
    file.print(",");
    file.print(String(* (long*) latitude));
    file.print(",");
    file.print(String(* (long*) longitude));
    
    file.close();
    
    DEBUG_SERIAL.print(F("Data Added: "));
    DEBUG_SERIAL.println(fileName);
    
    free(fileName);
  }else{
    DEBUG_SERIAL.println(F("Node not registered"));
    return false;
  }

  // Save into posting buffer
  if (sd.exists("ToSend.bin")) {
    // Create csv file
    File file;
    if (!file.open("ToSend.bin", FILE_WRITE)) {  // Open ToSend.bin
      DEBUG_SERIAL.print(F("Open CSV Failed: "));
      DEBUG_SERIAL.println("ToSend.bin");
      return false;
    }

    // Convert data to byte string
    // Example:
    // Type:  0x0D
    // nodeID:  0x06
    // unixTime:  0x5D 0x11 0x35 0xCA
    // latitude:  0x15 0xCD 0x5B 0x07
    // longitude:  0xB1 0x68 0xDE 0x3A
    // -->
    // 0d065d1135ca15cd5b07b168de3a
    file.print("\r\n");
    file.print(getByte(type));
    file.print(getByte(nodeID));
    file.print(getByte(unixTime[3]));
    file.print(getByte(unixTime[2]));
    file.print(getByte(unixTime[1]));
    file.print(getByte(unixTime[0]));
    file.print(getByte(latitude[3]));
    file.print(getByte(latitude[2]));
    file.print(getByte(latitude[1]));
    file.print(getByte(latitude[0]));
    file.print(getByte(longitude[3]));
    file.print(getByte(longitude[2]));
    file.print(getByte(longitude[1]));
    file.print(getByte(longitude[0]));

    file.close();
    DEBUG_SERIAL.println(F("Buffer Added"));
  }else{
    DEBUG_SERIAL.println(F("ToSend.bin not existed"));
    return false;
  }
}

// Get file name for tracker csv log
// remember to free(fileName) after called
char* getFileName(uint8_t id){
  // Set file name
  char* fileName = (char*) malloc(sizeof(char) * 7);
  if (fileName == NULL) {
    DEBUG_SERIAL.println(F("Ran out of memory making file name"));
    free(fileName);
    return NULL;
  }
  sprintf(fileName, "t%u.csv", (char*) id);
  return fileName;
}

// Register Tracker into SD card
// return true : success reg
// return false : failed reg
bool registerTracker(uint8_t id){
  SdFat sd;
  if (!sd.begin(SD_CONFIG)) {
    DEBUG_SERIAL.println(F("SD Error"));
    return false;
  }
  
  DEBUG_SERIAL.print(F("Register Tracker: "));
  // Get file name for tracker csv log
  char* fileName = getFileName(id);
  if (fileName == NULL)
    return false;
  DEBUG_SERIAL.println(fileName);

  // If csv not existed, create with name "t<id>.csv"
  if (!sd.exists(fileName)) {
    // Create csv file
    File file;
    if (!file.open(fileName, FILE_WRITE)) {
      DEBUG_SERIAL.print(F("Open CSV Failed: "));
      DEBUG_SERIAL.println(fileName);
      free(fileName);
      return false;
    }
    file.print(fileName); // Write ID in first line
    file.print(F("\nCount,Time,Lat,Lon")); // Write table header
    file.close();
  }else{
    DEBUG_SERIAL.println(F("CSV File Existed"));
  }
  DEBUG_SERIAL.print(F("Register Complete: "));
  DEBUG_SERIAL.println(fileName);
  free(fileName);
  return true;
}

// Reset ToSend.bin from SD
void resetToSend(){
  SdFat sd;
  if (!sd.begin(SD_CONFIG)) {
    DEBUG_SERIAL.println(F("SD Error."));
    return;
  }
  if (sd.exists("ToSend.bin")) {
    sd.remove("ToSend.bin"); // Delete ToSend.bin
  }
  if (!sd.exists("ToSend.bin")) {
    File file;
    if (!file.open("ToSend.bin", FILE_WRITE)) {  // Create ToSend.bin
      DEBUG_SERIAL.println(F("Open CSV Failed"));
      return;
    }
    file.print(GATEWAY_ID); // Write ID in first line
    file.close();
  }else{
    DEBUG_SERIAL.println(F("ToSend.bin Still Exist"));
  }
  DEBUG_SERIAL.println(F("ToSend.bin Reseted"));
}

/* ------------------------ Helper for Debug ------------------------- */

#ifdef DEBUG
void printByte(uint8_t b) {
    DEBUG_SERIAL.print(F(" 0x"));
    if (b <= 0xF)
        DEBUG_SERIAL.print(F("0"));
    DEBUG_SERIAL.print(b, HEX);
}
#endif
