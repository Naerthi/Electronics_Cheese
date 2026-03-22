#include <Motoron.h>
#include "Arduino_LED_Matrix.h"

MotoronI2C mc(0x10);  

const int motorLeft = 1;
const int motorRight = 2;

int speed = 0;
const int maxSpeed = 800;

ArduinoLEDMatrix matrix;

// SENSOR TABLES
const int numPoints = 16;

float voltageTable[numPoints] = {
  2.60, 2.30, 1.63, 1.40, 1.15, 0.97, 0.90, 0.80,
  0.75, 0.70, 0.68, 0.63, 0.60, 0.57, 0.50, 0.40
};

float distanceTable[numPoints] = {
  0,2,3,4,5,6,7,8,9,10,11,12,13,14,15,18
};

float filteredVoltage = 0;

// PINS
const int distancePin = A2;
const int ldrPin = A0;
const int buttonPin = 2;   
const int ledPin = 9;   

// STATES
bool ledState = false;        
bool lastButtonState = LOW;   
bool on = false; 

// THRESHOLDS
const float obstacleThreshold = 4.0;
const float lightThreshold = 25.0; // needs to tuned when testing

void setup() {
  Wire.begin();
  Serial.begin(115200);
  matrix.begin();

  mc.reinitialize();    
  mc.disableCrc();      
  mc.clearResetFlag();  

  mc.setMaxAcceleration(motorLeft, 200);
  mc.setMaxDeceleration(motorLeft, 300);
  mc.setMaxAcceleration(motorRight, 200);
  mc.setMaxDeceleration(motorRight, 300);

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  on = button_on();

  if (!on) return;

  float distance = getFilteredDistance();
  float light = readLight();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" | Light: ");
  Serial.println(light);

  
  if (distance <= obstacleThreshold) {
    avoidObstacle();
  } 
  else if (light < lightThreshold) {
    searchForLight();
  } 
  else {
    moveTowardLight(light);
  }

  delay(50);
}

// MOVEMENT

void MoveForward() {
  mc.setSpeed(motorLeft, speed);
  mc.setSpeed(motorRight, speed);
}

void MoveBackwards() {
  mc.setSpeed(motorLeft, -(speed));      // send command to motor (-800 to 800)
  mc.setSpeed(motorRight, -(speed));
}

void TurnLeft() {
  mc.setSpeed(motorLeft, -speed);
  mc.setSpeed(motorRight, speed);
}

void TurnRight() {
  mc.setSpeed(motorLeft, speed);
  mc.setSpeed(motorRight, -speed);
}

void StopMotors() {
  mc.setSpeed(motorLeft, 0);
  mc.setSpeed(motorRight, 0);
}

void Swerve() {
  unsigned long startTime = millis();

  // Swerve right for 2 seconds
  while (millis() - startTime < 2000) {
    mc.setSpeed(motorLeft, speed);      // left motor full speed
    mc.setSpeed(motorRight, speed / 2); // right motor slower to turn right
    delay(20);                           // small delay for I2C stability
  }

  // After 2 seconds, go straight at normal speed
  mc.setSpeed(motorLeft, speed);
  mc.setSpeed(motorRight, speed);
}

// BEHAVIOURS

void avoidObstacle() {
  StopMotors();
  delay(100);

  // Simple randomized turn (better than fixed direction)
  if (random(0, 2) == 0) {
    TurnLeft();
  } else {
    TurnRight();
  }

  delay(400);  // turn enough to clear obstacle
}

// for rotating the robot until stronger light is found
float bestLight = 0;

void searchForLight() {
  bestLight = 0;

  for (int i = 0; i < 8; i++) {
    TurnRight();
    delay(150);   // small rotation step

    StopMotors();
    delay(50);
    
    float currentLight = readLight();
    
    if (currentLight > bestLight) {
      bestLight = currentLight;
    }
  }
  // After scanning, move forward
  MoveForward();
}

void moveTowardLight(float lightValue) {

  speed = motor_control(lightValue);
  speed = constrain(speed, 150, maxSpeed);

  MoveForward();
}

// SENSORS

// Distance
float readVoltage() {
  const int samples = 20;
  long total = 0;

  for(int i = 0; i < samples; i++) {
    total += analogRead(distancePin);
    delay(2);
  }

  float avgReading = total / (float)samples;
  return avgReading * (5.0 / 1023.0);
}

float getDistance(float volts) {
  if(volts >= voltageTable[0]) return distanceTable[0];
  if(volts <= voltageTable[numPoints-1]) return distanceTable[numPoints-1];

  for(int i = 0; i < numPoints - 1; i++) {
    if(volts <= voltageTable[i] && volts >= voltageTable[i+1]) {
      float ratio = (volts - voltageTable[i+1]) /
                    (voltageTable[i] - voltageTable[i+1]);

      return distanceTable[i+1] +
             ratio * (distanceTable[i] - distanceTable[i+1]);
    }
  }
  return -1;
}

float getFilteredDistance() {
  float volts = readVoltage();
  filteredVoltage = 0.8 * filteredVoltage + 0.2 * volts;
  float distance = getDistance(filteredVoltage);
  return distance - 5.48;
}

// Light
float readLight() {
  int raw = analogRead(ldrPin);
  float voltage = raw * (5.0 / 1023.0);
  return voltage * 10;
}

// BUTTON - ON/OFF
bool button_on(){
  bool buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH && lastButtonState == LOW) {
      ledState = !ledState;
      on = !on;          
      digitalWrite(ledPin, ledState);
      delay(200);                    
  }

  lastButtonState = buttonState;
  return on;
}

// MOTOR CONTROLL

int motor_control(float voltage) {
  if (voltage >= 47) {
    return 0; 
  } 
  else if (voltage <= 0.1) { 
    return 600;
  } 
  else {
    float computed = maxSpeed / (voltage / 60.0);
    return constrain((int)computed, 0, maxSpeed);
  }
}
