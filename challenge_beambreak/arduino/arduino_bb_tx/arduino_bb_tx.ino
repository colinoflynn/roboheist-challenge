#include <SoftwareSerial.h>
#include <stdlib.h>

const byte rxPin = 2;
const byte txPin = 5;

// Set up a new SoftwareSerial object
SoftwareSerial mySerial (rxPin, txPin);

void setup() {
    // Set pin 9 (OC1A) as output
    pinMode(9, OUTPUT);

    // Configure Timer1
    TCCR1A = 0; // Clear Timer1 Control Register A
    TCCR1B = 0; // Clear Timer1 Control Register B

    // Set CTC mode (Clear Timer on Compare Match)
    TCCR1B |= (1 << WGM12);

    // Set prescaler to 1 (no division)
    TCCR1B |= (1 << CS10);

    // Set compare match value for 38.0 kHz
    OCR1A = 211;

    // Enable toggle mode on pin 9
    TCCR1A |= (1 << COM1A0);


    pinMode(11, OUTPUT); // Using Pin 11 for Timer2 on Arduino Uno
    
    TCCR2A = 0; // Clear Timer2 Control Register A
    TCCR2A |= (1 << WGM21); // Set to CTC Mode
    TCCR2A |= (1 << COM2A0); // Toggle OC2A on Compare Match
    
    TCCR2B = 0; // Clear Timer2 Control Register B
    TCCR2B |= (1 << CS20); // Set prescaler to 1 (No prescaling)
    
    // Calculate OCR2A for approximately 59 kHz
    OCR2A = (16000000 / (2 * 1 * 59000)) - 1;

    pinMode(rxPin, INPUT_PULLUP);
    pinMode(txPin, OUTPUT);

    mySerial.begin(300);
    Serial.begin(600);
}

uint8_t chksum(char * data, unsigned char len){
  uint8_t sum = 0;

  while(len){
    sum ^= *data++;
    len--;
  }
  
  return sum; 
}

char temp[20];
int rint = 0;
int counttime = 0;

void loop() {
  // put your main code here, to run repeatedly:
  rint = random() & 0x1FFF;
  sprintf(temp, "ACHY %04d ", rint);
  mySerial.write(temp);
  sprintf(temp, "%02x\n", chksum(temp, 9));
  mySerial.write(temp);
  
  _delay_ms(150);

  counttime++;
  if(counttime > 2){
    Serial.write("BRKA 1A\n");
    counttime = 0;
  }
  
}
