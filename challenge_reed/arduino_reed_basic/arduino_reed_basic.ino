/*

This is used to sense the reed sensor.

Connect it to pinReed on one side, GND on the other.

Changes the state of a tri-colour LED. Will be used to do other stuff later,
but this is used for basic visual signalling.


*/

static int pinLEDRed = 4;
static int pinLEDGreen = 5;
static int pinReed = 3;

int reedState = 0;

void setup() {
  pinMode(pinLEDRed, OUTPUT);
  pinMode(pinLEDGreen, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(pinReed, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  reedState = digitalRead(pinReed);

  if (reedState){
    digitalWrite(pinLEDRed, 1);
    digitalWrite(pinLEDGreen, 0);
  } else {
     digitalWrite(pinLEDRed, 0);
    digitalWrite(pinLEDGreen, 1);   
  }
}
