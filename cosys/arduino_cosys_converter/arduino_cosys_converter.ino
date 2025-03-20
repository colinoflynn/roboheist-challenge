#include <SoftwareSerial.h>
#include <stdlib.h>

const byte rxPin = 2;
const byte txPin = 3;


// Set up a new SoftwareSerial object
SoftwareSerial breakSerial (rxPin, txPin);

void setup() {
    pinMode(rxPin, INPUT_PULLUP);
    pinMode(txPin, OUTPUT);

    pinMode(LED_BUILTIN, OUTPUT);
    breakSerial.begin(600);
    Serial.begin(9600);

    digitalWrite(LED_BUILTIN, 1);
}

void loop() {
  //Serial.write("h");
  //delay(500);
  while (breakSerial.available() > 0) {
    char inByte = breakSerial.read();
    Serial.write(inByte);
    digitalWrite(LED_BUILTIN, 0);
  }
  digitalWrite(LED_BUILTIN, 1);
}
