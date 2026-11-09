/*

Name: Measurement vehicle
Emails: mito3@uic.edu mRahi2@uic.edu 
Team: Marco Tinoco, Minahil Rahimullah 
netID: mtino3, mRahi2 
Group Number: 69 Project 

Our project is a remote-controlled vehicle, allowing it to measure distance. We will use this feature to measure distance, 
allowing the user to pick between length, width, and distance modes. The first two modes measure the height and width of a given space, 
and distance mode allows the user to measure the distance traveled by the vehicle. All of this information will be displayed on the LCD on 
the controller.

This is the code for the remote.
*/

#include <SPI.h>
#include <RF24.h>
#include <LiquidCrystal.h>

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //initalize the lcd

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
double length, startingL, endingL, width, startingW, endingW = 0.00;
int areaCounter = 3; // make it four so that when the button is pressed and it increments to zero

void setup() {
  radio.begin(); // initailize the radio
  radio.setPALevel(RF24_PA_MIN); // set to lowest setting
  radio.setDataRate(RF24_2MBPS); // set to 2 mbps
  radio.setChannel(124); // set to frequenzy that won't be use by other devices for maximum efficiancy 
  radio.openWritingPipe(pipeOut); // sending data pipe
  radio.openReadingPipe(1, pipeIn); // getting data pipe
  lcd.begin(16, 2); // Inilitalize all variable to zero
  lcd.print("Welcome"); // Intro Message line 1
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
  int x = analogRead(joystickX); //read in x value
  int y = analogRead(joystickY); // read in y value
  int data[] = {x, y}; // put both value in array for easy sending

  if (!radio.write(&data, sizeof(data))) { //check if the array did not go through
    Serial.println("Failed to send data"); // if not, output error message
  }
  
  radio.startListening(); // Start listening after sending
  unsigned long started_waiting_at = millis(); // start timer for when we start listening
  bool timeout = false; // keeps track if we have reached the time limit or not. 

  while (!radio.available() && !timeout) { 
    if (millis() - started_waiting_at > 200) { // Wait for 200ms
      timeout = true;
    }
  }

  if (timeout) { // if we timeout, print error statement
    Serial.println("Failed to receive response.");
  } else { //if we don't timeout, get the value sent
    double distance = 0.0;
    radio.read(&distance, sizeof(distance)); //read the value sent from the car
    // Display the received distance on the LCD or handle it otherwise
    processDistance(distance);
  }
    
  radio.stopListening(); // make sure we not listening anymore so we can x + y again

  checkButtons(); //check if any buttons were pressed or not and update the mode/areabutton

}

void processDistance(double distance) { // this function checks what the current Mode is (distance/Area), and prints to the lcd accordingly
  lcd.clear();
  if (currentMode == 1 || currentMode == 0) { // if distance mode on, then print the  
    lcd.print("Distance: ");
    lcd.print(distance);
    lcd.print("m");
  } else if (currentMode == 2) { // if area mode, then go to updateArea function
    updateAreaMeasurement(distance);
  }
}

void updateAreaMeasurement(double distance) { //
  switch (areaCounter) { // switch cases for the different value of the area counter
    case 0: // means that we have just started the areaMode
      startingL = distance; // Put down the starting distance 
      lcd.setCursor(0, 0); 
      lcd.print("Starting point"); // Display to the user that Area mode is starting
      break;
    case 1: // means the user is pressing the button a second time to get the final value of length
      endingL = distance; // gets the final distance
      length = endingL - startingL; // caluculate the length from the two distances saved
      lcd.setCursor(0, 0);
      lcd.print("Length: "); // display the length in meters
      lcd.print(length);
      lcd.print("m");
      startingW = distance; // also save the starting distance of the width
      break;
    case 2: // means that user has pressed the button three times 
      endingW = distance; // save the ending distance for the width
      width = endingW - startingW; // calulate the width
      lcd.setCursor(0, 0);
      lcd.print("Width: "); // display the width in meters
      lcd.print(width);
      lcd.print("m");
      break;
    case 3:
      double area = length * width; // calulate the area and display it on the lcd
      lcd.setCursor(0, 1);
      lcd.print("Area: ");
      lcd.print(area);
      lcd.print("m2");
      break;
  }
}

void checkButtons() {
 static unsigned long lastButtonPress = 0;
  if (millis() - lastButtonPress > 200) { // Simple debouncing
    if (digitalRead(distButton)) { // was distanceMode button pressed?
      lastButtonPress = millis(); // if so, save the time
      currentMode = 1; // change mo0de variable
      areaCounter = 0; // Reset area counter on mode change
    } else if (digitalRead(areaButton)) { // was areMode button pressed?
      lastButtonPress = millis(); // save time
      currentMode = 2; // change variable
      areaCounter = (areaCounter + 1) % 4; // Increment and wrap around the area counter safely
      // Serial.print(areaCounter);
      // Serial.print("\n");

    }
  }
}
