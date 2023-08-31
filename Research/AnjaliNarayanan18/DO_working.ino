
#include <SoftwareSerial.h>                           //we have to include the SoftwareSerial library, or else we can't use it
//#define rx 7                                          //define what pin rx is going to be
//#define tx 8                                          //define what pin tx is going to be

SoftwareSerial dosensor(8, 7); //tx is connected to pin 7 and rx is connected to 8         //define how the soft serial port is going to work



void setup() {
  Serial.begin(9600);                                 //set baud rate for the hardware serial port_0 to 9600
  dosensor.begin(9600);

}

void loop() {
  dosensor.print('R');                      //send that string to the Atlas Scientific product
  dosensor.print('\r');
  char c = '\0';

  while (c != '\r') {
    if (c == '\0') {
      Serial.println("null");
    }
    while (dosensor.available() <= 0);
    c = char(dosensor.read());
    Serial.print(c);
  }
  delay(2000);
}
