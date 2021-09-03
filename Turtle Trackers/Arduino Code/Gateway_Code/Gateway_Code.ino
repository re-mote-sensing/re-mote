#include <SPI.h>
#include <LoRa.h>
#include <NeoSWSerial.h>

#define SIM3G_EN A1
#define SIM3G_TX 4
#define SIM3G_RX 5

#define POWER_ON_TIME 180

NeoSWSerial ss(SIM3G_TX, SIM3G_RX);

String host = "turtletracker.ddns.net";
String altHost = "65.93.37.85";
String endpoint = "/data";

void setup() {
  pinMode(SIM3G_EN, OUTPUT);

  // Power cycle 3G module to reset it
  digitalWrite(SIM3G_EN, HIGH);
  delay(POWER_ON_TIME);
  digitalWrite(SIM3G_EN, LOW);

  delay(1000);

  // Turn it off to save power
  digitalWrite(SIM3G_EN, HIGH);
  delay(4000);
  digitalWrite(SIM3G_EN, LOW);
  
  while (!Serial);

  Serial.begin(115200);
  Serial.println("Serial started.");

  Serial.println("LoRa started.");
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Enabled LoRa module's built in error checking
  LoRa.enableCrc();
  
  ss.begin(9600);
  Serial.println("TinySine serial started.");
}

void loop() {
  // Listen for packets
  int packetSize = LoRa.parsePacket();
  String result;
  if (packetSize) {
	// Parse header values
    byte sender = LoRa.read();
    byte msgNum = LoRa.read();
    byte msgLen = LoRa.read();

	// Read packet message into String
    while (LoRa.available()) {
      result += (char)LoRa.read();
    }

    Serial.println(result);

    start3G();
    postToServer(altHost, 8080, sender, msgNum, result);
    stop3G();
	
	// TODO: Save packet if failed to send
  } else {
	// Enter Low Power Mode
	// This currently causes the gateway to function incorrectly, exact cause is not determinable
    // Serial.flush();
    // LoRa.sleep();
    // for (int i = 0; i < 2; i++) {
    //   LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    // }
  }
}

// Creates a POST request and sends it over 3G to the webserver
void postToServer(String host, int port, byte id, byte count, String message) {
  // Session Open Request
  char sessCommand[255];
  sprintf(sessCommand, "AT+CHTTPSOPSE=\"%s\",%d,1", host.c_str(), port);	
  
  // Try to open an HTTP session
  while (!sendCommand(sessCommand, 10000).equals("OK"));
  
  // Build POST request
  char command[255];
  sprintf(command, "POST /data HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 30\r\n\r\nid=%d&count=%d&msg=%s", host.c_str(), id, count, message.c_str());
  
  // TODO: Figure out exact length of the request
  sendCommand("AT+CHTTPSSEND=143", 250);
  
  // Send the request and wait for a response
  sendCommand(command, 8000);
  sendCommand("AT+CHTTPSRECV?", 8000);
  sendCommand("AT+CHTTPSRECV=2000", 8000);
  sendCommand("AT+CHTTPSCLSE", 8000);
}

void start3G() {
  // Turn on the 3G module
  digitalWrite(SIM3G_EN, HIGH);
  delay(POWER_ON_TIME);
  digitalWrite(SIM3G_EN, LOW);
  
  delay(15000);
  
  // Initialize the 3G module
  sendCommand("AT", 250);
  sendCommand("ATE0", 250);
  sendCommand("AT+CMEE=2", 250);
  sendCommand("AT+CGDCONT=1,\"IP\",\"rogers-core-appl1.apn\"", 250);
  sendCommand("AT+CGSOCKCONT=1,\"IP\",\"rogers-core-appl1.apn\"", 250);
  sendCommand("AT+CSOCKSETPN=1", 250);
  
  // Attempt to start the HTTPS service
  while (sendCommand("AT+CHTTPSSTART", 10000).equals("ERROR")) {
    sendCommand("AT+CHTTPSCLSE", 10000);
    sendCommand("AT+CHTTPSSTOP", 10000);
  }

  delay(500);
}

void stop3G() {
  // Stop the HTTPS service
  sendCommand("AT+CHTTPSSTOP", 2000);
  
  // Turn off the module
  digitalWrite(SIM3G_EN, HIGH);
  delay(4000);
  digitalWrite(SIM3G_EN, LOW);
}

// Helper function used to simplify sending AT commands and receiving responses
String sendCommand(const char *command, unsigned long timeout) {
  String result;
  Serial.print(">> ");
  Serial.println(command);
  ss.println(command);
  unsigned long start =  millis();
  Serial.print("<< ");
  while(millis() - start < timeout) {
    if (ss.available()) {
      char c = ss.read();
      if (c == 0x0A || c == 0x0D);
      else {
        Serial.write(c);
        result += c;
      }
    }
  }
  Serial.println();
  return result;
}