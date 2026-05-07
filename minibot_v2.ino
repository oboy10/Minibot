// RWD 2-Wheel Bot — Tank Style
// Left stick UP/DOWN  -> Left motor
// Right stick UP/DOWN -> Right motor
//
// Inverted driving mode using channel 4:
// Normal mode:   left stick -> left motor,  right stick -> right motor
// Invert mode:   left stick -> right motor, right stick -> left motor
//
// NOTE:
// This version SWAPS the controls in invert mode,
// but does NOT invert the pulse direction.

#include <Servo.h>

#define LEFT_CH      8
#define RIGHT_CH     9
#define MODE_CH      4

#define MOTOR_L_PIN  13
#define MOTOR_R_PIN  11

#define NEUTRAL      1500
#define DEADBAND     40

Servo motorL, motorR;

int applyDeadband(int val) {
  return (abs(val - NEUTRAL) < DEADBAND) ? NEUTRAL : val;
}

void setup() {
  Serial.begin(115200);

  pinMode(LEFT_CH, INPUT);
  pinMode(RIGHT_CH, INPUT);
  pinMode(MODE_CH, INPUT);

  motorL.attach(MOTOR_L_PIN);
  motorR.attach(MOTOR_R_PIN);

  motorL.writeMicroseconds(NEUTRAL);
  motorR.writeMicroseconds(NEUTRAL);

  delay(2000); // SparkMAX arm time
  Serial.println("Bot Ready.");
}

void loop() {
  int leftRaw  = pulseIn(LEFT_CH, HIGH, 25000);
  int rightRaw = pulseIn(RIGHT_CH, HIGH, 25000);
  int modeRaw  = pulseIn(MODE_CH, HIGH, 25000);

  if (leftRaw == 0)  leftRaw = NEUTRAL;
  if (rightRaw == 0) rightRaw = NEUTRAL;
  if (modeRaw == 0)  modeRaw = NEUTRAL;

  int leftCmd  = applyDeadband(leftRaw);
  int rightCmd = applyDeadband(rightRaw);

  int leftOut, rightOut;

  // Step 3 / high position = swapped drive mode
  bool reverseMode = (modeRaw > 1700);

  if (!reverseMode) {
    leftOut  = leftCmd;
    rightOut = rightCmd;
  } else {
    // Swap only
    leftOut  = rightCmd;
    rightOut = leftCmd;
  }

  leftOut  = constrain(leftOut, 1000, 2000);
  rightOut = constrain(rightOut, 1000, 2000);

  motorL.writeMicroseconds(leftOut);
  motorR.writeMicroseconds(rightOut);

  Serial.print("ModeRaw:");
  Serial.print(modeRaw);
  Serial.print("  Mode:");
  Serial.print(reverseMode ? "SWAPPED" : "NORMAL");
  Serial.print("  Lraw:");
  Serial.print(leftRaw);
  Serial.print("  Rraw:");
  Serial.print(rightRaw);
  Serial.print("  Lout:");
  Serial.print(leftOut);
  Serial.print("  Rout:");
  Serial.println(rightOut);

  delay(20);
}

