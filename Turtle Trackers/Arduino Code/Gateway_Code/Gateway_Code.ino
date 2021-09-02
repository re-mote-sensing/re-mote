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

  digitalWrite(SIM3G_EN, HIGH);
  delay(POWER_ON_TIME);
  digitalWrite(SIM3G_EN, LOW);

  delay(1000);

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

  LoRa.enableCrc();
  
  ss.begin(9600);
  Serial.println("TinySine serial started.");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  String result;
  if (packetSize) {
    byte sender = LoRa.read();
    byte msgNum = LoRa.read();
    byte msgLen = LoRa.read();

    while (LoRa.available()) {
      result += (char)LoRa.read();
    }

    Serial.println(result);

    start3G();
    postToServer(altHost, 8080, sender, msgNum, result);
    stop3G();
  } else {
    Serial.flush();
    LoRa.sleep();
    for (int i = 0; i < 2; i++) {
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
  }
}

void postToServer(String host, int port, byte id, byte count, String message) {
  char sessCommand[255];
  sprintf(sessCommand, "AT+CHTTPSOPSE=\"%s\",%d,1", host.c_str(), port);
  
  while (!sendCommand(sessCommand, 10000).equals("OK"));
  
  char command[255];
  sprintf(command, "POST /data HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 30\r\n\r\nid=%d&count=%d&msg=%s", host.c_str(), id, count, message.c_str());
  
  sendCommand("AT+CHTTPSSEND=143", 250);
  
  sendCommand(command, 8000);
  sendCommand("AT+CHTTPSRECV?", 8000);
  sendCommand("AT+CHTTPSRECV=2000", 8000);
  sendCommand("AT+CHTTPSCLSE", 8000);
}

void start3G() {
  digitalWrite(SIM3G_EN, HIGH);
  delay(POWER_ON_TIME);
  digitalWrite(SIM3G_EN, LOW);
  
  delay(15000);

  sendCommand("AT", 250);
  sendCommand("ATE0", 250);
  sendCommand("AT+CMEE=2", 250);
  sendCommand("AT+CGDCONT=1,\"IP\",\"rogers-core-appl1.apn\"", 250);
  sendCommand("AT+CGSOCKCONT=1,\"IP\",\"rogers-core-appl1.apn\"", 250);
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