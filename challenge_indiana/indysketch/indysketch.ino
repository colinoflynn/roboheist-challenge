#include <SoftwareSerial.h>

// These constants won't change. They're used to give names to the pins used:
const int sensorPins[6] = {A2,A3,A4,A5,A6,A7};

int sensorThresholds[6] = {200, 200, 200, 200, 200, 250};
int sensorNorms[6] = {0,1,1,1,1,1};
int sensorValues[6] = {0};
int sensorAlarms[6] = {0,0,0,0,0,0};

int sensorAlarmFlags[6] = {0,0,0,0,0,0};

char tempbeacon[20];

int laserPresent;

int alarmTriggered = 0;

int LED_RED = 3;
int LED_GREEN = 4;

const byte rxPin = 2;
const byte txPin = 5;

// Set up a new SoftwareSerial object
SoftwareSerial breakSerial (rxPin, txPin);

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

  pinMode(11, OUTPUT); // Using Pin 11 for Timer2 on Arduino Uno
  
  TCCR2A = 0; // Clear Timer2 Control Register A
  TCCR2A |= (1 << WGM21); // Set to CTC Mode
  TCCR2A |= (1 << COM2A0); // Toggle OC2A on Compare Match
  
  TCCR2B = 0; // Clear Timer2 Control Register B
  TCCR2B |= (1 << CS20); // Set prescaler to 1 (No prescaling)
  
  // Calculate OCR2A for approximately 59 kHz
  OCR2A = (16000000 / (2 * 1 * 59000)) - 1;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  breakSerial.begin(600);
}

uint8_t chksum(char * data, unsigned char len){
  uint8_t sum = 0;

  while(len){
    sum ^= *data++;
    len--;
  }
  
  return sum; 
}

void loop() {
  // read the analog in value:
  for(int i = 0; i < 6; i++){
    sensorValues[i] = analogRead(sensorPins[i]);
    _delay_ms(25);

    //low value means laser is present, as the resistance is low
    //so voltage was pulled down
    if (sensorValues[i] < sensorThresholds[i]) {
      laserPresent = 1;
    } else {
      laserPresent = 0;
    }

    // Increment the alarm count, maxing out at 1000
    // if laser is not matching expected value
    if (sensorNorms[i] != laserPresent) {
          sensorAlarms[i] += 1;
          if (sensorAlarms[i] > 1000){
            sensorAlarms[i] == 1000;
          }
    } else {
      //Reset alarm count on single normal cycle...
      sensorAlarms[i] = 0;      
    }
    Serial.print("sensor ");
    Serial.print(i);
    Serial.print("\t = ");
    Serial.println(sensorValues[i]);
  }

  //Check if ANY sensor sees 4 wrong readings in a row.
  //if so set a alarmTriggered flag
  for(int i = 0; i < 6; i++){
    if (sensorAlarms[i] > 3){
      alarmTriggered = 10;
      sensorAlarmFlags[i] = 1;
    } else {
      sensorAlarmFlags[i] = 0;
    }
  }

  // Check alarmTriggered flag. Keep doing stuff until we've alarmed
  // for that many cycles.
  if (alarmTriggered){
    alarmTriggered--;
    //Red LEDs on
    digitalWrite(LED_BUILTIN, 1);
    digitalWrite(LED_RED, 1);
    digitalWrite(LED_GREEN, 0);
  } else {
    //Green LEDs on
    digitalWrite(LED_BUILTIN, 0);
    digitalWrite(LED_RED, 0);
    digitalWrite(LED_GREEN, 1);
  }

  if (alarmTriggered){
    sprintf(tempbeacon, "IND %d%d%d%d%d%d AL ", sensorAlarmFlags[0], sensorAlarmFlags[1], sensorAlarmFlags[2],
                                                sensorAlarmFlags[3], sensorAlarmFlags[4], sensorAlarmFlags[5]);
  } else {
    sprintf(tempbeacon, "IND %d%d%d%d%d%d OK ", sensorAlarmFlags[0], sensorAlarmFlags[1], sensorAlarmFlags[2],
                                                sensorAlarmFlags[3], sensorAlarmFlags[4], sensorAlarmFlags[5]);
  }
  breakSerial.write(tempbeacon);
  sprintf(tempbeacon, "%02x\n", chksum(tempbeacon, 13));
  breakSerial.write(tempbeacon);

  // wait 10ms - due to serial being so slow this is actually longer!
  _delay_ms(10);
}
