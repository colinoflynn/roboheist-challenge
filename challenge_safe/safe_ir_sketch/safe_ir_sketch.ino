#include <SoftwareSerial.h>
#include <stdlib.h>

const byte rxPinIgnored = 10;
const byte txPinforLED = 7;

// Set up a new SoftwareSerial object
SoftwareSerial breakSerial (rxPinIgnored, txPinforLED);

/****************************

 IR Blaster for Safe

 Connect IR led as follows, cathode to Pin 7,
 anode to Pin 11:

            1K       C   A
 Pin 7 --/\/\/\/\-----|<|-----------+
                                    |
 Pin 11 ----------------------------+

 Connect safe serial output to RX input.
 
 ****************************/

void setup() {
    pinMode(11, OUTPUT); // Using Pin 11 for Timer2 on Arduino Uno
    
    TCCR2A = 0; // Clear Timer2 Control Register A
    TCCR2A |= (1 << WGM21); // Set to CTC Mode
    TCCR2A |= (1 << COM2A0); // Toggle OC2A on Compare Match
    
    TCCR2B = 0; // Clear Timer2 Control Register B
    TCCR2B |= (1 << CS20); // Set prescaler to 1 (No prescaling)
    
    // Calculate OCR2A for approximately 59 kHz
    OCR2A = (16000000 / (2 * 1 * 59000)) - 1;

    pinMode(rxPinIgnored, INPUT_PULLUP);
    pinMode(txPinforLED, OUTPUT);

    breakSerial.begin(600);
    Serial.begin(115200);
}

uint8_t chksum(char * data, unsigned char len){
  uint8_t sum = 0;

  while(len){
    sum ^= *data++;
    len--;
  }
  
  return sum; 
}

void serialFlush(){
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}

char tempsafe[20];
char tempbeacon[20];
int lenRead = 0;
char c;

char status = '?';
char digit = '?';

void loop() {
  while (Serial.available()) {
    c = Serial.read();

    if((c >= '0') && (c <= '9')){
      digit = c;
    }

    if( c=='U' || c=='L'){
      status = c;
    }
  }

  sprintf(tempbeacon, "SAF %c:%c", digit, status);
  breakSerial.write(tempbeacon);
  sprintf(tempbeacon, " %02x\n", chksum(tempbeacon, 7));
  breakSerial.write(tempbeacon);

  delay(400);
}
