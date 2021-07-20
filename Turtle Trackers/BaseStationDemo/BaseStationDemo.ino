#include <SPI.h>
#include <LoRa.h>
#include <NeoSWSerial.h>          // Overall a better Serial library

#define SIM3G_EN A1
#define SIM3G_TX 4
#define SIM3G_RX 5

#define POWER_ON_TIME 180

NeoSWSerial ss(SIM3G_TX, SIM3G_RX);

const char host[] = "65.93.37.163";
const char endpoint[] = "/data";
const int port = 8080;

int packet = 0;

void setup() {
  pinMode(SIM3G_EN, OUTPUT);

  digitalWrite(SIM3G_EN, HIGH);
  delay(POWER_ON_TIME);
  digitalWrite(SIM3G_EN, LOW);
  
  while (!Serial);

  Serial.begin(115200);
  Serial.println("Serial started.");

  Serial.println("LoRa started.");
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  ss.begin(9600);
  Serial.println("TinySine serial started.");

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

void loop() {
  int packetSize = LoRa.parsePacket();
  String result;
  if (packetSize) {
    Serial.print("Received packet with RSSI ");
    Serial.println(LoRa.packetRssi());

    while (LoRa.available()) {
      char c = (char)LoRa.read();
      result += c;
    }
    Serial.println(result);
    postToServer();
  }
}

void postToServer() {
  String sessStatus = sendCommand("AT+CHTTPSOPSE=\"65.93.37.163\",8080,1", 2000);

  if (sessStatus.equals("OK")) {
    sendCommand("AT+CHTTPSSEND=133", 250);
    sendCommand("POST /data HTTP/1.1\r\nHost: 65.93.37.163\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 11\r\n\r\ntype=1&id=1", 3000);
    sendCommand("AT+CHTTPSRECV?", 5000);
    sendCommand("AT+CHTTPSRECV=2000", 5000);
    sendCommand("AT+CHTTPSCLSE", 2000);
  } 
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

String buildRequest(int id, long latitude, long longitude) {
  String request = "POST ";
  request.concat(endpoint);
  request.concat(" HTTP/1.1\r\n");
  request.concat("Host: ");
  request.concat(host);
  request.concat("\r\nContent-Type: application/x-www-form-urlencoded\r\n");
}
