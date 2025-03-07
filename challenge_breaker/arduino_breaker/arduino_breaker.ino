#include <SoftwareSerial.h>
#include <stdlib.h>

const byte rxPin = 10;
const byte txPin = 7;

const byte switchPin = 2;

const int LED_RED = 3;
const int LED_GREEN = 5;

// Set up a new SoftwareSerial object
SoftwareSerial breakSerial (rxPin, txPin);

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

    pinMode(switchPin, INPUT_PULLUP);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_RED, OUTPUT);

    breakSerial.begin(600);

    digitalWrite(LED_BUILTIN, 1);
    digitalWrite(LED_RED, 0);
    digitalWrite(LED_GREEN, 1);
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
char c;
char strindex = 0;
bool strvalid = false;
unsigned int counttime = 0;

bool pwr = true;
int lastled = 1;
char tempbeacon[20];

void loop() {

  //PWR is OFF when high
  pwr = digitalRead(switchPin) ^ 1;

  _delay_ms(50);

  lastled ^= 1;

  if(counttime & 1){
    if(pwr){
      digitalWrite(LED_RED, lastled);
      digitalWrite(LED_GREEN, 1);
    } else {
      digitalWrite(LED_GREEN, 0);
      digitalWrite(LED_RED, 1);
    }
  }

  counttime++;
  if(counttime > 6){
    if(pwr){
      sprintf(tempbeacon, "PWR 1");
    } else {
      sprintf(tempbeacon, "PWR 0");
    }
    breakSerial.write(tempbeacon);
    sprintf(tempbeacon, " %02x\n", chksum(tempbeacon, 5));
    breakSerial.write(tempbeacon);
    counttime = 0;
  }
}
