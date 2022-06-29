
/* ------------------------ Server Post ------------------------- */

// Post ToSend.csv data to server
// return true : Sent
// return false : Empty
bool postToServerCSV(){
  int lineCount = 2; // skip the first two lines from csv (Gateway ID, Table Head)
  while (true){
    String line = getPostBufferFromSD(lineCount); // Read line from ToSend.csv
    line.trim(); // Delete the trailing character
    if (line=="END") // If EOF
      break;
    else if (line=="SKIP") // If Blank line
      continue;
    else if (line=="ETY") // If Empty File
      return false;
    lineCount++; // Next line
    
    DEBUG_SERIAL.print("Sending... : ");
    DEBUG_SERIAL.println(line);

    // Open HTTP Session
    char sessCommand[CHTTPSOPSE_LENGTH]; // 24+29=53
    sprintf(sessCommand, "AT+CHTTPSOPSE=\"%s\",%d,1", host, port);
    long start = millis();
    while (sendCommand(sessCommand, 10000).equals("ERROR"))
      if (millis() - start < SIM3G_HTTP_TIMEOUT)
        return false;
  
    // Payload
    char msgBody[msgBody_LENGTH]; // "line" is 38 max
    sprintf(msgBody, "data=%s", line.c_str());
  
    // POST Command
    char command[POST_COMMAND_LENGTH]; // 110 with current "host" and "endpoint"
    sprintf(command, "POST %s?%s HTTP/1.1\r\nHost: %s\r\n\r\n", endpoint, msgBody, host);

    // Send HTTP Request
    char preCommand[64];
    sprintf(preCommand, "AT+CHTTPSSEND=%d", strlen(command));
    sendCommand(preCommand, 250);

    // Receive HTTP Response
    sendCommand(command, 8000);
    sendCommand("AT+CHTTPSRECV?", 8000);
    sendCommand("AT+CHTTPSRECV=1000", 8000);
    sendCommand("AT+CHTTPSCLSE", 3000);
  }
  return true;
}

//bool postToServer(String id, String count, String message) {
//  char sessCommand[64];
//  sprintf(sessCommand, "AT+CHTTPSOPSE=\"%s\",%d,1", host.c_str(), port);
//
//  long start = millis();
//  while (sendCommand(sessCommand, 10000).equals("ERROR")){
//    if (millis() - start < SIM3G_HTTP_TIMEOUT){
//      return false;
//    }
//  }
//
//  // Payload
//  char msgBody[64];
//  sprintf(msgBody, "id=%d&count=%d&msg=%s", id.c_str(), count.c_str(), message.c_str());
//
//  // POST Command
//  char command[255];
//  sprintf(command, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s", endpoint.c_str(), host.c_str(), strlen(msgBody), msgBody);
//
//  char preCommand[64];
//  sprintf(preCommand, "AT+CHTTPSSEND=%d", strlen(command));
//  sendCommand(preCommand, 250);
//  
//  sendCommand(command, 8000);
//  sendCommand("AT+CHTTPSRECV?", 8000);
//  String result = sendCommand("AT+CHTTPSRECV=1000", 8000);
//  sendCommand("AT+CHTTPSCLSE", 3000);
//
//  // Server should return "Message Received Successfully"
//  if(result.indexOf(F("Success")) > 0){
//    DEBUG_SERIAL.println(F("Post Success"));
//    return true;
//  }else{
//    DEBUG_SERIAL.println(F("Post Failed"));
//    DEBUG_SERIAL.println(result);
//    return false;
//  }
//}

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
  delay(4500);
  long start = millis();
  String result;
  while (!result.equals(F("ATOK"))&&!result.equals(F("OK"))){
    if (millis() - start > SIM3G_POWER_ON_TIMEOUT)
      break;
    result = sendCommand("AT", 250);
  }
  sendCommand("ATE0", 250);
  delay(1000);
  sendCommand("AT+CMEE=2", 250);
  sendCommand("AT+CGDCONT=1,\"IP\",\"pda.bell.ca\"", 250);
  sendCommand("AT+CGSOCKCONT=1,\"IP\",\"pda.bell.ca\"", 250);
  sendCommand("AT+CSOCKSETPN=1", 250);
  
  while (sendCommand("AT+CHTTPSSTART", 10000).equals(F("ERROR"))) {
    sendCommand("AT+CHTTPSCLSE", 10000);
    sendCommand("AT+CHTTPSSTOP", 10000);
  }
  delay(500);
}

// Stop 3G Shield HTTP Request
void stop3GHTTP() {
  sendCommand("AT+CHTTPSSTOP", 2000);
  powerOff3G();
}

// Send Command via Serial
String sendCommand(const char *command, unsigned long timeout) {
  String result;
  DEBUG_SERIAL.print(F(">> "));
  DEBUG_SERIAL.println(command);
  ss.println(command);
  unsigned long start =  millis();
  DEBUG_SERIAL.print(F("<< "));
  int count = 0;
  while(millis() - start < timeout) {
    if (ss.available()) {
      char c = ss.read();
      if (c == 0x0A || c == 0x0D);
      else {
        DEBUG_SERIAL.write(c);
        result += c;
        count++;
//        if (count>=400){ // The string is too long, needs to get the end part
//          result = "";
//          count = 0;
//        }
      }
    }
  }
  DEBUG_SERIAL.println();
  result.trim();
  return result;
}

/* ------------------------ SD Card ------------------------- */

// Create a ToSend.csv file in SD Card
void createToSendFile(){
  SdFat sd;
  if (!sd.begin(SD_CONFIG)) {
    DEBUG_SERIAL.println(F("SD Error."));
    return;
  }
  if (!sd.exists("ToSend.csv")) {
    File file;
    if (!file.open("ToSend.csv", FILE_WRITE)) {
      DEBUG_SERIAL.println(F("Open CSV Failed"));
      return;
    }
    file.print(GATEWAY_ID); // Write ID in first line
    file.print(F("\r\nID,Count,Time,Lat,Lon")); // Write table header
    file.close();
  }else{
    DEBUG_SERIAL.println(F("ToSend.csv Existed"));
  }
  DEBUG_SERIAL.println(F("SD Ok"));
}

// Add Tracker data into SD card
// return true : success
// return false : failed
bool addData(char id, String count, String timestamp, String lat, String lon) {
  SdFat sd;
  if (!sd.begin(SD_CONFIG)) {
    DEBUG_SERIAL.println(F("SD Error"));
    return false;
  }
  
  // Get file name for tracker csv log
  char* fileName = getFileName(id);
  if (fileName == NULL)
    return false;

  // Save data to tracker csv
  if (sd.exists(fileName)) {
    // Create csv file
    File file;
    free(fileName);
    char* fileName = getFileName(id); // Get name again
    if (fileName == NULL)
      return false;
    if (!file.open(fileName, FILE_WRITE)) { // Open Tracker data CSV
      DEBUG_SERIAL.print(F("Open CSV Failed: "));
      DEBUG_SERIAL.println(fileName);
      free(fileName);
      return false;
    }
    
    file.print("\r\n"+count+","+timestamp+","+lat+","+lon); // Data added to CSV
    file.close();
    
    DEBUG_SERIAL.print(F("Data Added: "));
    DEBUG_SERIAL.println(fileName);
    
    free(fileName);
  }else{
    DEBUG_SERIAL.println(F("Node not registered"));
    return false;
  }

  // Save into posting buffer
  if (sd.exists("ToSend.csv")) {
    // Create csv file
    File file;
    if (!file.open("ToSend.csv", FILE_WRITE)) {  // Create ToSend.csv
      DEBUG_SERIAL.print(F("Open CSV Failed: "));
      DEBUG_SERIAL.println("ToSend.csv");
      return false;
    }

    char idString[5];
    sprintf(idString, "%d", id);
    
    file.print("\r\n"+String(idString)+","+count+","+timestamp+","+lat+","+lon);
    file.close();
    DEBUG_SERIAL.println(F("Buffer Added"));
  }else{
    DEBUG_SERIAL.println(F("ToSend.csv not existed"));
    return false;
  }
}

// Get file name for tracker csv log
// remember to free(fileName) after called
char* getFileName(char id){
  // Set file name
  char* fileName = (char*) malloc(sizeof(char) * 7);
  if (fileName == NULL) {
    DEBUG_SERIAL.println(F("Ran out of memory making file name"));
    free(fileName);
    return NULL;
  }
  sprintf(fileName, "t%u.csv", id);
  return fileName;
}

// Register Tracker into SD card
// return true : success reg
// return false : failed reg
bool registerTracker(char id){
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

  // If csv not existed, create with name "tracker<id>.csv"
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

// Remove ToSend.csv from SD
void resetToSend(){
  SdFat sd;
  if (!sd.begin(SD_CONFIG)) {
    DEBUG_SERIAL.println(F("SD Error."));
    return;
  }
  if (sd.exists("ToSend.csv")) {
    sd.remove("ToSend.csv"); // Delete ToSend.csv
  }
  if (!sd.exists("ToSend.csv")) {
    File file;
    if (!file.open("ToSend.csv", FILE_WRITE)) {  // Create ToSend.csv
      DEBUG_SERIAL.println(F("Open CSV Failed"));
      return;
    }
    file.print(GATEWAY_ID); // Write ID in first line
    file.print(F("\r\nID,Count,Time,Lat,Lon")); // Write table header
    file.close();
  }else{
    DEBUG_SERIAL.println(F("ToSend.csv Still Exist"));
  }
  DEBUG_SERIAL.println(F("ToSend.csv Reseted"));
}

// Get lines from ToSend.csv
// return END : End of file
// return SKIP : Blank line
// return ETY : Empty File
String getPostBufferFromSD(int count){
  SdFat sd;
  if (!sd.begin(SD_CONFIG)) {
    DEBUG_SERIAL.println(F("SD Error."));
    return "END";
  }
  if (sd.exists("ToSend.csv")) {  // Create ToSend.csv
    File file;
    if (!file.open("ToSend.csv", FILE_READ)) {
      DEBUG_SERIAL.println(F("Open CSV Failed"));
      return "END";
    }
    char line[CSV_LINE_LENGTH]; // Store a line in csv here (35 char if node id is 1 character)
    for (int i = 0; i < count; i++) 
      file.fgets(line, sizeof(line)); // Skip lines that already read / ignore
    if (file.available()) {
      int n = file.fgets(line, sizeof(line)); // Read line
      if (n <= 0 || (line[n-1] != '\n' && n == (sizeof(line) - 1))) {
        DEBUG_SERIAL.println("fgets failed or line too long");
        return "SKIP";
      }
      if (String(line).indexOf(",")<=0) // If no "," in this line
        return "SKIP";
      file.close();
      return line; // Return the current line
    }else{
      file.close();
    }
  }else{
    DEBUG_SERIAL.println(F("ToSend.csv Not Exist"));
  }
  if (count == 2) // If no data is in the file
    return "ETY";
  return "END";
}

/* ------------------------ Helper for Debug ------------------------- */

void printByte(uint8_t b) {
    DEBUG_SERIAL.print(F(" 0x"));
    if (b <= 0xF)
        DEBUG_SERIAL.print(F("0"));
    DEBUG_SERIAL.print(b, HEX);
}
