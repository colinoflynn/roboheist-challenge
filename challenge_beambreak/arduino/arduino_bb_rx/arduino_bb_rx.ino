#include <SoftwareSerial.h>
#include <stdlib.h>

const byte rxPin = 2;
const byte txPin = 5;

const byte rxPin2 = 3;
const byte txPin2 = 4;

const int LED_RED = 8;
const int LED_GREEN = 7;

const int PIN_STATUS = 13;

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

    pinMode(rxPin2, INPUT_PULLUP);
    pinMode(txPin2, OUTPUT);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_RED, OUTPUT);

    pinMode(PIN_STATUS, OUTPUT);
    digitalWrite(PIN_STATUS, 0);

    breakSerial.begin(300);
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
char c;
char strindex = 0;
bool strvalid = false;

int last_valid_time = 0;
int sum;
int counttime = 0;
int alarms = 0;
char tempbeacon[20];

void loop() {  
  while (breakSerial.available() > 0) {
        c = breakSerial.read();

        if (c == '\n'){
          temp[strindex] = 0;
          strindex = 0;

          //process string
          if (strvalid == true){

            //starts with ACHY
            if (temp[0] != 'A'){
              goto fail;
            }
            if (temp[1] != 'C'){
              goto fail;
            }
            if (temp[2] != 'H'){
              goto fail;
            }
            if (temp[3] != 'Y'){
              goto fail;
            }
            if (temp[4] != ' '){
              goto fail;
            }
            //Next four digits are random
            //We don't care...

            //Next is space
            if (temp[9] != ' '){
              goto fail;
            }  

            //Finally - we have checksum in HEX
            if (sscanf(&temp[10], "%02x", &sum) != 1){
              goto fail;
            }

            if(chksum(temp, 9) != sum){
              goto fail;
            }

            //Made it this far - message received!!!!            
            last_valid_time = 0;
          }

          fail:
          
          //OK - next string might be valid!
          strvalid = true;
        } else {
          temp[strindex++] = c;
        }

        // String is too long. Just reset index, this one 
        // won't be processed and hope next one works.
        if (strindex > 17){
          strvalid = false;
          strindex = 0;
        }
    }
  
  _delay_ms(50);
  last_valid_time++;

  if (last_valid_time > 5){
    digitalWrite(LED_BUILTIN, 1);
    digitalWrite(LED_RED, 0);
    digitalWrite(LED_GREEN, 1);
    digitalWrite(PIN_STATUS, 1);
  } else {
    digitalWrite(LED_BUILTIN, 0);
    digitalWrite(LED_GREEN, 0);
    digitalWrite(LED_RED, 1);
    digitalWrite(PIN_STATUS, 0);
  }

  if(last_valid_time == 10){
    alarms++;
  }

  counttime++;
  if(counttime > 6){
    sprintf(tempbeacon, "BRKB %04d ", alarms);
    Serial.write(tempbeacon);
    sprintf(tempbeacon, "%02x\n", chksum(tempbeacon, 10));
    Serial.write(tempbeacon);
    counttime = 0;
  }
}
