//transmitter
#include <SPI.h>
#include <RF24.h>
#include <LiquidCrystal.h>

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

RF24 radio(8, 9); // CE, CSN
const uint64_t pipeOut = 0xE8E8F0F0E1LL; // Set the pipe address for outgoing data
const uint64_t pipeIn = 0xE8E8F0F0E2LL; // Set the pipe address for incoming data
// Joystick pins
const int joystickX = A0; // Brown 
const int joystickY = A1; // Brown

const int areaButton = 16; // A2 - Orange
const int distButton = 17; // A3 - Yellow

// Variable to hold the current display mode
int currentMode = 0; // 0 = no mode, 1 = Area Mode, 2 = Distance Mode
int length = 0;
int startingL = 0;
int endingL = 0;
int width = 0;
int startingW = 0;
int endingW = 0;
int areaCounter = 0;
int isDist = 0;

void setup() {
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_2MBPS);
  radio.setChannel(124);
  radio.openWritingPipe(pipeOut);
  radio.openReadingPipe(1, pipeIn);
  lcd.begin(16, 2); // Inilitalize all variable to zero
  lcd.print("Welcome");
  lcd.setCursor(0, 1);
  lcd.print("Starting up..."); // Intro Message line 2
  delay(1000);
  pinMode(areaButton, INPUT);
  pinMode(distButton, INPUT);
  pinMode(joystickX, INPUT);
  pinMode(joystickY, INPUT);
  Serial.begin(9600); // Initialize serial communication
}

void loop() {
  
  checkButton();
  processDistance(34.00);


}

void processDistance(double distance) {
  lcd.clear();
  if (currentMode == 1) {
    lcd.print("Distance: ");
    lcd.print(distance);
  } else if (currentMode == 2) {
    updateAreaMeasurement(distance);
  }
}

void updateAreaMeasurement(double distance) {
  switch (areaCounter) {
    case 0:
      startingL = distance;
      lcd.setCursor(0, 1);
      lcd.print("Start L: ");
      lcd.print(startingL);
      break;
    case 1:
      endingL = distance;
      length = endingL - startingL;
      lcd.setCursor(0, 1);
      lcd.print("Length: ");
      lcd.print(length);
      startingW = distance;
      break;
    case 2:
      endingW = distance;
      width = endingW - startingW;
      lcd.setCursor(0, 1);
      lcd.print("Width: ");
      lcd.print(width);
      break;
    case 3:
      int area = length * width;
      lcd.print("Area: ");
      lcd.print(area);
      break;
  }
}

void checkButtons() {
  static unsigned long lastButtonPress = 0;
  if (millis() - lastButtonPress > 200) { // Simple debouncing
    if (digitalRead(areaButton)) {
      lastButtonPress = millis();
      currentMode = 1;
      areaCounter = 0; // Reset area counter on mode change
      Serial.print(currentMode);
      Serial.print("")
    } else if (digitalRead(distButton)) {
      lastButtonPress = millis();
      currentMode = 2;
      areaCounter = (areaCounter + 1) % 4; // Increment and wrap around the area counter safely
    }
  }
}
