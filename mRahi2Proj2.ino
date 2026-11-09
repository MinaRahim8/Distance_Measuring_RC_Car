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

This the the code for the remote controlled car
*/

#include <SPI.h>
#include <RF24.h>
RF24 radio(9, 10); // CE, CSN
const uint64_t pipeOut = 0xE8E8F0F0E1LL; // Set the pipe address for outgoing data
const uint64_t pipeIn = 0xE8E8F0F0E2LL; // Set the pipe address for incoming data
// Motor pins

const int in1 = 14; // Analog pin A0 - Green
const int in2 = 15; // Analog pin A1 - Blue
const int in3 = 4; // Purple
const int in4 = 7; // Grey
const int enA = 5; // Yellow
const int enB = 6; // White

int left_intr = 0; // interupt counter for left wheel
int right_intr = 0; // interrupt counter for right wheel 
const double radius = 0.03175; // radius of wheel
double distance = 0; // distance traveled
int DEBOUNCE_DELAY = 10; // how much time should pass before the next input read
volatile unsigned long lastLeftInterrupt = 0; // when the last interrupt was
volatile unsigned long lastRightInterrupt = 0; // same here

bool connectionEstablished = false;  // flag to check if data has been received

void setup() {
  radio.begin();
  radio.setPALevel(RF24_PA_MIN); // set to lowest setting
  radio.setDataRate(RF24_2MBPS); // set to 2 mbps
  radio.setChannel(124); // set to unique channel
  radio.openWritingPipe(pipeIn); // sends info
  radio.openReadingPipe(1, pipeOut); // gets info
  radio.startListening(); // make sure we ready for info 
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  // Serial.begin(9600); // Initialize serial communication
  // Serial.println("Waiting for data...");
  attachInterrupt(digitalPinToInterrupt(3), Left_ISR, CHANGE); //Left_ISR is called when left wheel sensor is triggered
  attachInterrupt(digitalPinToInterrupt(2), Right_ISR, CHANGE);//Right_ISR is called when right wheel sensor is triggered
}

void loop() {
  if (radio.available()) { //if we've made a connection to the remote control

    int data[2]; // array to hold joystick values
    radio.read(&data, sizeof(data));
    int x = data[0];
    int y = data[1];
    int motorSpeedA = map(y, 0, 1023, -255, 255); // map the value for easy convertion to the directions
    int motorSpeedLeft = motorSpeedA + (x - 512) / 2; // find the left speed value  
    int motorSpeedRight = motorSpeedA - (x - 512) / 2; // find the right speed value
    motorSpeedLeft = constrain(motorSpeedLeft, -255, 255); // make sure these values dont go over the -255/255 line
    motorSpeedRight = constrain(motorSpeedRight, -255, 255);

    // Set motor directions
    if (motorSpeedLeft > 0) { // if positive, go clockwise (forward)
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
    } else { // if negative, go counterclockwise (backwards)
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      motorSpeedLeft = -motorSpeedLeft; // make sure speed is a positive value if negative
    }
    if (motorSpeedRight > 0) { // same for the right tire
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
    } else {
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
      motorSpeedRight = -motorSpeedRight;
    }

    // Set motor speeds
    analogWrite(enA, motorSpeedLeft); // set the speed of the left 
    analogWrite(enB, motorSpeedRight); // and right tire


    unsigned long totalInterrupts = left_intr + right_intr; // add the two inturupt values
    if (totalInterrupts > 0) { // if the car has moved, calculate the distance
      double wheelCircumference = 2 * 3.1415 * radius; // calulate wheelCircumference
      double rotations = (left_intr + right_intr) / 2.0 / 40.0; // Assuming 40 pulses per revolution, also average the inturrpts for accurate distance
      distance = wheelCircumference * rotations; // calulate the distance
      // Serial.print(distance);
      // Serial.print("\n");
      radio.stopListening(); // stop listening, otherwise we can't send the distance back to the controller
      double dist = distance;
      radio.write(&dist, sizeof(double)); // send to controller
      radio.startListening(); // start listening again for cordinates
    }
    
    //Serial print for debugging
    // Serial.print("X: ");
    // Serial.print(x);
    // Serial.print(" - Y: ");
    // Serial.print(y);
    // Serial.print(" - Left: ");
    // Serial.print(motorSpeedLeft);
    // Serial.print(" - Right: ");
    // Serial.println(motorSpeedRight);
  }
}

void Left_ISR()
{
  unsigned long currentTime = millis(); // start timer
  if (currentTime - lastLeftInterrupt > DEBOUNCE_DELAY) { // see if 10 ms has passed
    left_intr++; // increment counter 
    lastLeftInterrupt = currentTime; // reset time 
  }
}


void Right_ISR()
{
  unsigned long currentTime = millis(); // start timer
  if (currentTime - lastRightInterrupt > DEBOUNCE_DELAY) { // see if 10 ms has passed
    right_intr++; // increment counter 
    lastRightInterrupt = currentTime; // reset time
  }
}