
//Install 2.0.0 of "yet another Arduino Wiegand library"
#include <Wiegand.h>

//SoftwareSerial library included by default?
#include <SoftwareSerial.h>

// These are the pins connected to the Wiegand D0 and D1 signals.
// Ensure your board supports external Interruptions on these pins
#define PIN_D0 2
#define PIN_D1 3
#define PIN_BEEP 8
#define PIN_LED  7

#define PIN_MAGNET 13
#define PIN_STATUS 12

//IR LED
const byte dbrxPin = 4;
const byte dbtxPin = 5;

// Set up a new SoftwareSerial object
SoftwareSerial debugSerial (dbrxPin, dbtxPin);

// The object that handles the wiegand protocol
Wiegand wiegand;

volatile unsigned char statusLocked = 1;

int loopCount;

// Initialize Wiegand reader
void setup() {

  //NOTE: By default the HW UART is used for the IR led at 600 baud.
  // this is needed since the SW UART will mess up the wiegand library
  // if used for regular interval beacons

  //Software debug serial
  pinMode(dbrxPin, INPUT_PULLUP);
  pinMode(dbtxPin, OUTPUT);

  debugSerial.begin(9600);

  //IR LED Serial
  Serial.begin(600);

  //Install listeners and initialize Wiegand reader
  wiegand.onReceive(receivedData, "Card readed: ");
  wiegand.onReceiveError(receivedDataError, "Card read error: ");
  wiegand.onStateChange(stateChanged, "State changed: ");
  wiegand.begin(Wiegand::LENGTH_ANY, true);

  //initialize pins as INPUT and attaches interruptions
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);
  digitalWrite(PIN_BEEP, HIGH);
  digitalWrite(PIN_LED, HIGH);
  pinMode(PIN_BEEP, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_MAGNET, OUTPUT);
  pinMode(PIN_STATUS, OUTPUT);

  digitalWrite(PIN_MAGNET, HIGH);
  digitalWrite(PIN_STATUS, LOW);
  
  attachInterrupt(digitalPinToInterrupt(PIN_D0), pinStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_D1), pinStateChanged, CHANGE);

  //Sends the initial pin state to the Wiegand library
  pinStateChanged();

  //59 kHz for IR tx
  pinMode(11, OUTPUT); // Using Pin 11 for Timer2 on Arduino Uno
  
  TCCR2A = 0; // Clear Timer2 Control Register A
  TCCR2A |= (1 << WGM21); // Set to CTC Mode
  TCCR2A |= (1 << COM2A0); // Toggle OC2A on Compare Match
  
  TCCR2B = 0; // Clear Timer2 Control Register B
  TCCR2B |= (1 << CS20); // Set prescaler to 1 (No prescaling)
  
  // Calculate OCR2A for approximately 59 kHz
  OCR2A = (16000000 / (2 * 1 * 59000)) - 1;
}

// Every few milliseconds, check for pending messages on the wiegand reader
// This executes with interruptions disabled, since the Wiegand library is not thread-safe
void loop() {
  noInterrupts();
  wiegand.flush();
  interrupts();
  //Sleep a little -- this doesn't have to run very often.
  delay(100);

  if (loopCount > 5){
    if(statusLocked){
      Serial.write("RFID S:L 1C\n");
    } else {
      Serial.write("RFID S:U 05\n");
    }
    loopCount = 0;
  }

  loopCount++;
}

// When any of the pins have changed, update the state of the wiegand library
void pinStateChanged() {
  wiegand.setPin0State(digitalRead(PIN_D0));
  wiegand.setPin1State(digitalRead(PIN_D1));
}

// Notifies when a reader has been connected or disconnected.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onStateChange()`
void stateChanged(bool plugged, const char* message) {
    debugSerial.print(message);
    debugSerial.println(plugged ? "CONNECTED" : "DISCONNECTED");
}

// Notifies when a card was read.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onReceive()`
void receivedData(uint8_t* data, uint8_t bits, const char* message) {
    debugSerial.print(message);
    debugSerial.print(bits);
    debugSerial.print("bits / ");
    //Print value in HEX
    uint8_t bytes = (bits+7)/8;
    for (int i=0; i<bytes; i++) {
        debugSerial.print(data[i] >> 4, 16);
        debugSerial.print(data[i] & 0xF, 16);
    }
    debugSerial.println();
}

// Notifies when an invalid transmission is detected
void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
    //Serial.print(message);
    //Serial.print(Wiegand::DataErrorStr(error));
    debugSerial.print(" - Raw data: ");
    debugSerial.print(rawBits);
    debugSerial.print("bits / ");

    if (rawBits == 39){
      unsigned int cardid = rawData[4] >> 1;
      cardid |= rawData[3] << 7;
      cardid |= (rawData[2] & 0x01) << 15;

      unsigned char facility = rawData[2] >> 1;
      facility |= (rawData[1] & 0x01) << 7;

      debugSerial.print(facility);
      debugSerial.print(cardid);

      if ((cardid == 44701) && (facility == 0xb4)){
        statusLocked = 0;
        
        digitalWrite(PIN_LED, LOW);
        digitalWrite(PIN_BEEP, LOW);
        digitalWrite(PIN_STATUS, HIGH);
        digitalWrite(PIN_MAGNET, LOW);
        _delay_ms(150);
        digitalWrite(PIN_BEEP, HIGH);
        _delay_ms(150);
        digitalWrite(PIN_BEEP, LOW);
        _delay_ms(150);
        digitalWrite(PIN_BEEP, HIGH);
        _delay_ms(150);
        digitalWrite(PIN_BEEP, LOW);
        digitalWrite(PIN_MAGNET, HIGH);
        _delay_ms(150);
        digitalWrite(PIN_BEEP, HIGH);
        _delay_ms(150);
        digitalWrite(PIN_BEEP, LOW);
        _delay_ms(150);
        digitalWrite(PIN_BEEP, HIGH);
        _delay_ms(2000);
        digitalWrite(PIN_LED, HIGH);
        digitalWrite(PIN_STATUS, LOW);
      }

      /*
      //Print value in HEX
      uint8_t bytes = (rawBits+7)/8;
      for (int i=0; i<bytes; i++) {
          Serial.print(rawData[i] >> 4, 16);
          Serial.print(rawData[i] & 0xF, 16);
      }
      */
    }
    debugSerial.println();
}
