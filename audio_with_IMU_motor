#include <WiFiS3.h>
#include <Wire.h>
#include <LSM6.h>
#include <Motoron.h>
#include <Modulino.h>

LSM6 imu;


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

//Motor set up
MotoronI2C mc(0x10);  // motor shield address
const int motorLeft = 1;
const int motorRight = 2;
int speed = 400;       // default speed

//Motor functions
void TurnLeft() {
  mc.setSpeed(motorLeft, -abs(speed));
  mc.setSpeed(motorRight, abs(speed));
}

void TurnRight() {
  mc.setSpeed(motorLeft, abs(speed));
  mc.setSpeed(motorRight, -abs(speed));
}

void MoveForewards() {
  mc.setSpeed(motorLeft, abs(speed));
  mc.setSpeed(motorRight, abs(speed));
}

void MoveBackwards() {
  mc.setSpeed(motorLeft, -abs(speed));
  mc.setSpeed(motorRight, -abs(speed));
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

void setup() {
  Wire.begin();
  mc.reinitialize();
  mc.disableCrc();
  mc.clearResetFlag();
  mc.setMaxAcceleration(motorLeft, 200);
  mc.setMaxDeceleration(motorLeft, 300);
  mc.setMaxAcceleration(motorRight, 200);
  mc.setMaxDeceleration(motorRight, 300);

  Serial.begin(115200);

  randomSeed(analogRead(A5));

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

  Wire.begin();

  if (!imu.init())
  {
    Serial.println("Failed to detect and initialize IMU!");
    while (1);
  }
  imu.enableDefault();
}

void sendNotification(String path)
{
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
unsigned long lastNotification = 0;
unsigned long notificationInterval = 5000;

void loop() {

  imu.read();

  double mag_acc = sqrt(pow(imu.a.x, 2) + pow(imu.a.y, 2) + pow(imu.a.z, 2));

  double mag_grav =  sqrt(pow(imu.g.y, 2) + pow(imu.g.z, 2));
  Serial.print("Acceleration Magnitude: ");
  Serial.println(mag_acc);
  Serial.print("Tilt Magnitude: ");
  Serial.println(mag_grav);

  if (mag_acc > 33000 & mag_grav > 40000) {
    Serial.println("HELP TRIGGERED");

    sendNotification(notifications[15]); // HELP notification
    Swerve();
    delay(500); // small debounce so it doesn't spam
    lastNotification = millis();
    notificationInterval = random(5000,7000);
    return;
  }

  if (millis() - lastNotification >= notificationInterval) {

    int index = random(0, 15);

    Serial.print("Sending sound: ");
    Serial.println(index);

    sendNotification(notifications[index]);

    lastNotification = millis();

    notificationInterval = random(5000, 7000);
  }
  delay(50);
}
