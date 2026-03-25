#include <Motoron.h>
#include "Arduino_LED_Matrix.h"
#include <WiFiS3.h>
#include <Wire.h>
#include <LSM6.h>
#include <Modulino.h>

//WiFi set up
char ssid[] = "zilin";
char pass[] = "watermelon";

WiFiSSLClient client;
char HOST_NAME[] = "api.pushcut.io";
const int port = 443;

// Store all notification paths in an array
String notifications[] = {
  "/XqclO3hK_CIauvfDO63QQ/notifications/CHEESE",
  "/XqclO3hK_CIauvfDO63QQ/notifications/Easy-Peasy",
  "/XqclO3hK_CIauvfDO63QQ/notifications/Frenchie",
  "/XqclO3hK_CIauvfDO63QQ/notifications/Good-God-Man",
  "/XqclO3hK_CIauvfDO63QQ/notifications/Hey-Baby",
  "/XqclO3hK_CIauvfDO63QQ/notifications/Hmm-Cheese",
  "/XqclO3hK_CIauvfDO63QQ/notifications/Holy-Macaroni",
  "/XqclO3hK_CIauvfDO63QQ/notifications/I-Feel-Grate",
  "/XqclO3hK_CIauvfDO63QQ/notifications/I-Forgot",
  "/XqclO3hK_CIauvfDO63QQ/notifications/I-Gorgonzola",
  "/XqclO3hK_CIauvfDO63QQ/notifications/Ain't-Easy",
  "/XqclO3hK_CIauvfDO63QQ/notifications/Feta-Up",
  "/XqclO3hK_CIauvfDO63QQ/notifications/Moo-FAHH",
  "/XqclO3hK_CIauvfDO63QQ/notifications/Mr-Bombastic",
  "/XqclO3hK_CIauvfDO63QQ/notifications/Cheese-Touch",
  "/XqclO3hK_CIauvfDO63QQ/notifications/HELP"
};
// Motor Setup
MotoronI2C mc(0x10);  

const int motorLeft = 1;
const int motorRight = 2;

int speed = 0;
const int maxSpeed = 800;
const int minStartSpeed = 500;

// Sensors
LSM6 imu;
ArduinoLEDMatrix matrix;

// PINS
const int distancePin = A2;
const int ldrPin = A0;
const int buttonPin = 2;   
const int ledPin = 9;   

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

// STATES
bool ledState = false;        
bool lastButtonState = LOW;   
bool on = false; 

// THRESHOLDS
const float obstacleThreshold = 4.0;
const float lightThreshold = 25.0; // needs to tuned when testing

// TIMERS
unsigned long lastNotification = 0;
unsigned long notificationInterval = 5000;
unsigned long lastMoveTime = 0;

// IMU Struct
struct IMUData {
  double accMag;
  double tiltMag;
};

void setup() {
  Wire.begin();
  Serial.begin(115200);

  matrix.begin();
  imu.init();
  imu.enableDefault();

  mc.reinitialize();    
  mc.disableCrc();      
  mc.clearResetFlag();  

  mc.setMaxAcceleration(motorLeft, 200);
  mc.setMaxDeceleration(motorLeft, 300);
  mc.setMaxAcceleration(motorRight, 200);
  mc.setMaxDeceleration(motorRight, 300);

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  randomSeed(analogRead(A3));

  WiFi.begin(ssid, pass);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected!");
  Serial.println(WiFi.localIP());
  Serial.begin(115200);

  // if (!imu.init())
  // {
  //   Serial.println("Failed to detect and initialize IMU!");
  //   while (1);
  // }
  // imu.enableDefault();
}

void loop() {
  // double mag_acc, mag_grav;

  // readIMU(mag_acc, mag_grav);

  // Serial.print("Acceleration Magnitude: ");
  // Serial.println(mag_acc);
  // Serial.print("Tilt Magnitude: ");
  // Serial.println(mag_grav);

  IMUData imuData = getIMUData();

  if (isHelpTriggered(imuData)) {
    handleHelp();
  }
  
  // if (checkForHelp(mag_acc, mag_grav)) {
  //   return;
  // }

  if (millis() - lastNotification >= notificationInterval) {
    playRandomNotification();
  }

  //on = button_on(); // uncomment if button is wanted
  on = true;
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
  else {
    if (light < lightThreshold) {
      searchForLight();
    } 
    else {
      moveTowardLight(light);
    }

    ensureMovement();
  }
  updateMovementTimer();
  delay(50);
}

// MOVEMENT

void MoveForward() {
  if (speed > 0 && speed < minStartSpeed) speed = minStartSpeed;
  // kickstart
  mc.setSpeed(motorLeft, 700);
  mc.setSpeed(motorRight, 700);
  delay(50);

  mc.setSpeed(motorLeft, speed);
  mc.setSpeed(motorRight, speed)
}

void MoveBackwards() {
  mc.setSpeed(motorLeft, -700);
  mc.setSpeed(motorRight, -700);
}

void TurnLeft() {
  mc.setSpeed(motorLeft, -600);
  mc.setSpeed(motorRight, 600);
}

void TurnRight() {
  mc.setSpeed(motorLeft, 600);
  mc.setSpeed(motorRight, -600);
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

// Movement Logic

int motor_control(float lightValue) {
  if (lightValue >= 47) return 0;
  int computed = map(lightValue, 0, 47, maxSpeed, minStartSpeed);
  if (computed > 0 && computed < minStartSpeed) {
    computed = minStartSpeed;
  }
  return constrain(computed, 0, maxSpeed);
}

// int motor_control(float voltage) {
//   if (voltage >= 47) {
//     return 0; 
//   } 
//   else if (voltage <= 0.1) { 
//     return 600;
//   } 
//   else {
//     float computed = maxSpeed / (voltage / 60.0);
//     return constrain((int)computed, 0, maxSpeed);
//   }
// }

void moveTowardLight(float lightValue) {
  speed = motor_control(lightValue);
  //speed = constrain(speed, 0, maxSpeed);
  MoveForward();
}

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
int bestIndex = 0;

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
      bestIndex = i;
    }
  }

  for (int j = 0; j < (8 - bestIndex); j++) {
    TurnRight();
    delay(150);
  }
  // After scanning, move forward
  MoveForward();
}

// Movement Saftey
void ensureMovement() {
  if (speed > 0 && speed < minStartSpeed) {
    speed = minStartSpeed;
  }

  if (speed > 0) {
    mc.setSpeed(motorLeft, 700);
    mc.setSpeed(motorRight, 700);
    delay(30);
  }
}

void updateMovementTimer() {

  if (speed > 0) {
    lastMoveTime = millis();
  }

  if (millis() - lastMoveTime > 3000) {

    MoveBackwards();
    delay(300);

    TurnLeft();
    delay(400);

    lastMoveTime = millis();
  }
}

// SENSORS

// DianalogRead(distancePin);stance

// Distance sensor
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

// IMU
IMUData getIMUData() {
  IMUData data;
  imu.read();

  data.accMag = sqrt(pow(imu.a.x,2) + pow(imu.a.y,2) + pow(imu.a.z,2));
  data.tiltMag = sqrt(pow(imu.g.y,2) + pow(imu.g.z,2));

  return data;
}

bool isHelpTriggered(IMUData data) {
  return (data.accMag > 33000 && data.tiltMag > 40000);
}

void handleHelp() {

  sendNotification(notifications[15]);

  mc.setSpeed(motorLeft, 700);
  mc.setSpeed(motorRight, 400);
  delay(2000);

  lastNotification = millis();
  notificationInterval = random(5000, 7000);
}

// Notifications

void playRandomNotification() {
  int index = random(0, 15);

  Serial.print("Sending sound: ");
  Serial.println(index);

  sendNotification(notifications[index]);

  lastNotification = millis();
  notificationInterval = random(5000, 7000);
}

void sendNotification(String path) {
  if (!client.connect(HOST_NAME, port)) {
    Serial.println("Connection failed");
    return;
  }
  client.println("GET " + path + " HTTP/1.1");
  client.println("Host: " + String(HOST_NAME));
  client.println("Connection: close");
  client.println();

  delay(50);
  client.stop();
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
