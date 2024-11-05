#include <SoftwareSerial.h>
SoftwareSerial module_bluetooth(0, 1); // pin RX | TX

// Pin definitions
#define LATCH 4
#define CLK 7
#define DATA 8
#define BUTTON1 A1
#define BUTTON2 A2
#define BUTTON3 A3
#define PROXIMITY_SENSOR 5

// Variables
char data = 0;
int LED1 = 13;
int LED2 = 12;
int LED3 = 11;
int LED4 = 10;
int buzzer = 3;
int potPin = A0;

bool buzzerOn = false;
int buttonState1 = 0;
int buttonState2 = 0;
int lastButton2State = HIGH;
int buttonState3 = 0;
int displayValue = 0;
unsigned long previousMillis = 0;
const long interval = 800; // Adjust this value for increment speed
bool beepActive = false;
unsigned long beepStartTime = 0;
const long beepDuration = 100; // Duration of each beep in milliseconds

// Byte for numbers 0 to 9 on seven-segment display
const byte SEGMENT_MAP[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
// Byte to select which LED to control
const byte SEGMENT_SELECT[] = {0xF1, 0xF2, 0xF4, 0xF8};

void setup() 
{
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);
  pinMode(PROXIMITY_SENSOR, INPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DATA, OUTPUT);

  digitalWrite(LED1, HIGH); // Set initial state to HIGH (off)
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(buzzer, HIGH); // Ensure the buzzer starts in the off state
}

void loop()
{
  // Read button states
  buttonState1 = digitalRead(BUTTON1);
  buttonState2 = digitalRead(BUTTON2);
  int button2Pressed = (buttonState2 == LOW && lastButton2State == HIGH); // Detect single press
  lastButton2State = buttonState2;
  buttonState3 = digitalRead(BUTTON3);
  int proximityState = digitalRead(PROXIMITY_SENSOR);

  // Check button A3 to turn off all if pressed
  if (buttonState3 == LOW) {
    turnOffAll();
    return;
  }

  // Bluetooth functionality
  if (Serial.available() > 0) 
  {
    data = Serial.read();
    handleBluetoothData(data);
  }

  // Handle buzzer tone based on potentiometer input
  if (buzzerOn) {
    int potValue = analogRead(potPin);
    int toneFrequency = map(potValue, 0, 1023, 100, 8000);
    tone(buzzer, toneFrequency);
  }

  // Handle button1 increment
  if (buttonState1 == LOW) {
    displayValue++;
    if (displayValue > 9999) displayValue = 9999;
    beep();
  }

  // Handle button2 multiplication
  if (button2Pressed) {
    displayValue *= 2;
    if (displayValue > 9999) displayValue = 9999;
    beep();
  }

  // Handle proximity sensor auto-increment and beep
  if (proximityState == LOW) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      displayValue++;
      if (displayValue > 9999) displayValue = 9999;
      beep();
    }
  } else {
    noTone(buzzer); // Ensure buzzer is off when proximity is inactive
    digitalWrite(buzzer, HIGH);
  }

  // Handle short beep duration
  if (beepActive && millis() - beepStartTime >= beepDuration) {
    noTone(buzzer); // Stop the beep
    beepActive = false;
    // delay(200);
  }

  // Display the value on the seven-segment display
  displayNumber(displayValue);
}

void handleBluetoothData(char data) {
  switch (data) {
    case '1': digitalWrite(LED1, HIGH); break;
    case '2': digitalWrite(LED1, LOW); break;
    case '3': digitalWrite(LED2, HIGH); break;
    case '4': digitalWrite(LED2, LOW); break;
    case '5': digitalWrite(LED3, HIGH); break;
    case '6': digitalWrite(LED3, LOW); break;
    case '7': digitalWrite(LED4, HIGH); break;
    case '8': digitalWrite(LED4, LOW); break;
    case '9': 
      buzzerOn = true; 
      digitalWrite(buzzer, LOW); // Activate the buzzer
      tone(buzzer, 1000); // Start the tone if not already playing
      break;
    case '0': 
      buzzerOn = false; 
      noTone(buzzer); // Stop the tone
      digitalWrite(buzzer, HIGH); // Ensure the buzzer is off
      break;
  }
}


void beep() {
  tone(buzzer, 1000); // Start the beep
  beepStartTime = millis();
  // delay(400);
  beepActive = true;
}

void displayNumber(int value) {
  int thousands = (value / 1000) % 10;
  int hundreds = (value / 100) % 10;
  int tens = (value / 10) % 10;
  int units = value % 10;

  if (value < 10) {
    WriteNumberToSegment(3, units);
  } else if (value < 100) {
    WriteNumberToSegment(2, tens);
    WriteNumberToSegment(3, units);
  } else if (value < 1000) {
    WriteNumberToSegment(1, hundreds);
    WriteNumberToSegment(2, tens);
    WriteNumberToSegment(3, units);
  } else {
    WriteNumberToSegment(0, thousands);
    WriteNumberToSegment(1, hundreds);
    WriteNumberToSegment(2, tens);
    WriteNumberToSegment(3, units);
  }
}

void WriteNumberToSegment(byte Segment, byte Value) {
  digitalWrite(LATCH, LOW);
  shiftOut(DATA, CLK, MSBFIRST, SEGMENT_MAP[Value]);
  shiftOut(DATA, CLK, MSBFIRST, SEGMENT_SELECT[Segment]);
  digitalWrite(LATCH, HIGH);
}

void turnOffAll() {
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  noTone(buzzer);
  buzzerOn = false;
  digitalWrite(buzzer, HIGH);
  displayValue = 0;
}
