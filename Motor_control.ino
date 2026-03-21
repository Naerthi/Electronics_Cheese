
#include <Motoron.h>
#include <Modulino.h>

// vvvv       ADD YOUR CODE HERE      vvvv
MotoronI2C mc(0x10);  // your motor shield's address goes here, '0xFF' format
// ^^^^       ADD YOUR CODE HERE      ^^^^
const int motorLeft = 1;
const int motorRight = 2;
int move = 0;
int speed = 0;

ModulinoKnob knob;    // initialise the Knob module

void setup()
{
  Wire.begin();
  Serial.begin(115200);

  

  // Reset the controller to its default settings, then disable
  // CRC.  The bytes for each of these commands are shown here
  // in case you want to implement them on your own without
  // using the library.
  mc.reinitialize();    // Bytes: 0x96 0x74
  mc.disableCrc();      // Bytes: 0x8B 0x04 0x7B 0x43

  // Clear the reset flag, which is set after the controller
  // reinitializes and counts as an error.
  mc.clearResetFlag();  // Bytes: 0xA9 0x00 0x04

  // By default, the Motoron is configured to stop the motors if
  // it does not get a motor control command for 1500 ms.  You
  // can uncomment a line below to adjust this time or disable
  // the timeout feature.
  // mc.setCommandTimeoutMilliseconds(1000);
  // mc.disableCommandTimeout();

  // Configure left motor
  mc.setMaxAcceleration(motorLeft, 200);
  mc.setMaxDeceleration(motorLeft, 300);
  // Configure right motor
  mc.setMaxAcceleration(motorRight, 200);
  mc.setMaxDeceleration(motorRight, 300);

  Modulino.begin();     // start Modulino libraries
  knob.begin();         // start Knob communication
}


void TurnLeft()
{
  mc.setSpeed(motorLeft, -abs(speed));      // send command to motor (-800 to 800)
  mc.setSpeed(motorRight, abs(speed));
}

void TurnRight()
{
  mc.setSpeed(motorLeft, abs(speed));      // send command to motor (-800 to 800)
  mc.setSpeed(motorRight, -abs(speed));
}

void MoveForewards()
{
  mc.setSpeed(motorLeft, abs(speed));      // send command to motor (-800 to 800)
  mc.setSpeed(motorRight, abs(speed));
}

void MoveBackwards()
{
  mc.setSpeed(motorLeft, -abs(speed));      // send command to motor (-800 to 800)
  mc.setSpeed(motorRight, -abs(speed));
}

void Swerve()
{
  
}
void loop()
{
  int position = knob.get();  // read knob position
  bool click = knob.isPressed();

  if (click){move++;}

  // vvvv       ADD YOUR CODE HERE      vvvv
  speed = position*50;
  
  Serial.print("Current position is: ");
  Serial.println(position);

  Serial.print("Current speed is: ");
  Serial.println(speed);

  if (move%4 == 0){MoveForewards(speed);}
  else if (move%4 == 1){TurnRight(speed);}
  else if (move%4 == 2){MoveBackwards(speed);}
  else if (move%4 == 3){TurnLeft(speed);}

// Full turn is just delay([2*pi (rad)]/[angular_velocity (rad/ms)])
  
  // ^^^^       ADD YOUR CODE HERE      ^^^^
  delay(50);
  position = position%801;
}
