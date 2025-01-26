

// These constants won't change. They're used to give names to the pins used:
const int sensorPins[6] = {A2,A3,A4,A5,A6,A7};

int sensorThresholds[6] = {200, 200, 200, 200, 200, 250};
int sensorNorms[6] = {0,1,1,1,1,1};
int sensorValues[6] = {0};
int sensorAlarms[6] = {0,0,0,0,0,0};

int laserPresent;

int alarmTriggered = 0;

int LED_RED = 3;
int LED_GREEN = 4;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
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

  // wait 100ms
  _delay_ms(100);
}
