#include <SPI.h>
#include <LoRa.h>
#include <NeoSWSerial.h>

#define SIM3G_EN A1
#define SIM3G_TX 4
#define SIM3G_RX 5

#define POWER_ON_TIME 180
#define POWER_OFF_TIME 4000

#define SIM3G_TIMEOUT 4000

#define DEBUG true // Set to true for debug output, false for no output
#define DEBUG_SERIAL if(DEBUG)Serial

NeoSWSerial ss(SIM3G_TX, SIM3G_RX);

String host = "turtletracker.cas.mcmaster.ca"; // Change this to the public IP address/ domain of the pi server's network
String endpoint = "/data";
String altHost = ""; // Alternate server address, currently not working
String altEndpoint = "";

int port = 80;

void setup() {
  pinMode(SIM3G_EN, OUTPUT);

  digitalWrite(SIM3G_EN, HIGH);
  delay(POWER_ON_TIME);
  digitalWrite(SIM3G_EN, LOW);

  delay(1000);

  digitalWrite(SIM3G_EN, HIGH);
  delay(POWER_OFF_TIME);
  digitalWrite(SIM3G_EN, LOW);
  
  while (!Serial);

  DEBUG_SERIAL.begin(115200);
  DEBUG_SERIAL.println("Serial started.");

  Serial.println("LoRa started.");
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.enableCrc(); // Enables the LoRa module's built in error checking
  
  ss.begin(9600);
  DEBUG_SERIAL.println("TinySine serial started.");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  String result;
  if (packetSize) {
    // 77 is the turtle tracker
    // This number can temporary avoid sending invaild messages to server
    if (!String(LoRa.read()).equals("77")) {
      #if DEBUG == true
      DEBUG_SERIAL.println("Received invaild LoRa Message");
      while (LoRa.available()) {
        result += (char)LoRa.read();
      }
      DEBUG_SERIAL.println(result);
      #endif  //DEBUG == true
      return;
    }
    byte sender = LoRa.read();
    byte msgNum = LoRa.read();
    byte msgLen = LoRa.read();

    while (LoRa.available()) {
      result += (char)LoRa.read();
    }

    DEBUG_SERIAL.println("Received Turtle Tracker LoRa Message");
    DEBUG_SERIAL.print("Node: ");
    DEBUG_SERIAL.println(sender);
    DEBUG_SERIAL.print("Count: ");
    DEBUG_SERIAL.println(msgNum);
    DEBUG_SERIAL.println(result);

    start3G();
    if (!postToServer(host, port, endpoint, sender, msgNum, result)){
      // postToServer(altHost, port, altEndpoint, sender, msgNum, result);
    }
    DEBUG_SERIAL.println("Posted to Server");
    stop3G();
  }
}

bool postToServer(String host, int port, String endpoint, byte id, byte count, String message) {
  
  char sessCommand[64];
  sprintf(sessCommand, "AT+CHTTPSOPSE=\"%s\",%d,1", host.c_str(), port);

  long start = millis();
  while (sendCommand(sessCommand, 10000).equals("ERROR")){
    if (millis() - start < SIM3G_TIMEOUT){
      return false;
    }
  }

  char msgBody[64];
  sprintf(msgBody, "id=%d&count=%d&msg=%s", id, count, message.c_str());
  
  char command[255];
  sprintf(command, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s", endpoint.c_str(), host.c_str(), strlen(msgBody), msgBody);

  char preCommand[64];
  sprintf(preCommand, "AT+CHTTPSSEND=%d", strlen(command));
  sendCommand(preCommand, 250);
  
  sendCommand(command, 8000);
  sendCommand("AT+CHTTPSRECV?", 8000);
  sendCommand("AT+CHTTPSRECV=1000", 8000);
  sendCommand("AT+CHTTPSCLSE", 8000);
  return true;
}

void start3G() {
  digitalWrite(SIM3G_EN, HIGH);
  delay(POWER_ON_TIME);
  digitalWrite(SIM3G_EN, LOW);
  
  delay(15000);

  sendCommand("AT", 250);
  sendCommand("ATE0", 250);
  sendCommand("AT+CMEE=2", 250);
  sendCommand("AT+CGDCONT=1,\"IP\",\"pda.bell.ca\"", 250);
  sendCommand("AT+CGSOCKCONT=1,\"IP\",\"pda.bell.ca\"", 250);
  sendCommand("AT+CSOCKSETPN=1", 250);
  
  while (sendCommand("AT+CHTTPSSTART", 10000).equals("ERROR")) {
    sendCommand("AT+CHTTPSCLSE", 10000);
    sendCommand("AT+CHTTPSSTOP", 10000);
  }

  delay(500);
}

void stop3G() {
  sendCommand("AT+CHTTPSSTOP", 2000);
  
  digitalWrite(SIM3G_EN, HIGH);
  delay(4000);
  digitalWrite(SIM3G_EN, LOW);
}

String sendCommand(const char *command, unsigned long timeout) {
  String result;
  DEBUG_SERIAL.print(">> ");
  DEBUG_SERIAL.println(command);
  ss.println(command);
  unsigned long start =  millis();
  DEBUG_SERIAL.print("<< ");
  while(millis() - start < timeout) {
    if (ss.available()) {
      char c = ss.read();
      if (c == 0x0A || c == 0x0D);
      else {
        DEBUG_SERIAL.write(c);
        result += c;
      }
    }
  }
  DEBUG_SERIAL.println();
  return result;
}
