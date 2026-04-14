// TwoWheelBot_PWM
// RC Controller to 2-Motor Drive Control (Tank-Style Independent)
// Left Stick  (Elevator) = Left  Motor (Back Left Wheel)
// Right Stick (Aileron)  = Right Motor (Back Right Wheel)
// Front roller is passive — no motor control needed

/*
PWM Pulse Value Limits for REV SparkMAX:
- Full Reverse:  1000 microseconds
- Neutral:       1500 microseconds
- Full Forward:  2000 microseconds

Wiring:
  RC Receiver → Arduino
  Elev  → Pin 1  (Left  stick up/down  → Left  motor)
  Aile  → Pin 0  (Right stick up/down  → Right motor)

  SparkMAX PWM signals:
  Left  Motor → Pin 10
  Right Motor → Pin 11

Motor inversion:
  If one side drives backward when it should go forward,
  flip its INVERT flag below.
*/

#include <Servo.h>

// RC Receiver Input Pins
#define AILE_PIN    0   // Right stick up/down
#define ELEV_PIN    1   // Left  stick up/down
#define THRO_PIN    2
#define RUDD_PIN    3
#define RIGHT_PIN   4
#define BUTTON_PIN  5
#define LEFT_PIN    6
#define SCROLL_PIN  7

// Motor Output Pins
#define MOTOR_LEFT_PIN  10
#define MOTOR_RIGHT_PIN 11

// Tuning
#define DEADBAND      30     // Ignore stick noise within ±30µs of center
#define INVERT_LEFT   false  // Flip if left  motor drives wrong direction
#define INVERT_RIGHT  true   // Flip if right motor drives wrong direction
                             // (right side is often physically mirrored)

#define NEUTRAL 1500

Servo motorLeft, motorRight;

// Apply deadband around 1500 center
int applyDeadband(int val, int deadband) {
  if (val == 0) return NEUTRAL;              // No signal guard
  int offset = val - NEUTRAL;
  if (abs(offset) < deadband) return NEUTRAL;
  return val;
}

// Invert a PWM value around neutral
int invertPWM(int val) {
  return NEUTRAL - (val - NEUTRAL);
}

void setMotor(Servo &motor, int pwmValue, bool invert) {
  pwmValue = constrain(pwmValue, 1000, 2000);
  if (invert) pwmValue = invertPWM(pwmValue);
  motor.writeMicroseconds(pwmValue);
}

void stopAll() {
  motorLeft.writeMicroseconds(NEUTRAL);
  motorRight.writeMicroseconds(NEUTRAL);
}

void setup() {
  Serial.begin(9600);

  // RC input pins
  pinMode(AILE_PIN,   INPUT);
  pinMode(ELEV_PIN,   INPUT);
  pinMode(THRO_PIN,   INPUT);
  pinMode(RUDD_PIN,   INPUT);
  pinMode(RIGHT_PIN,  INPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LEFT_PIN,   INPUT);
  pinMode(SCROLL_PIN, INPUT);

  // Attach motors to PWM pins
  motorLeft.attach(MOTOR_LEFT_PIN);
  motorRight.attach(MOTOR_RIGHT_PIN);

  // Neutral on startup — required for SparkMAX to arm
  stopAll();
  delay(2000);

  Serial.println("2-Wheel Bot Ready.");
  Serial.println("Left stick (Elev) → Left Motor | Right stick (Aile) → Right Motor");
}

void loop() {
  // Read RC channels (25ms timeout prevents blocking on lost signal)
  int elevValue = pulseIn(ELEV_PIN, HIGH, 25000);  // Left  stick vertical
  int aileValue = pulseIn(AILE_PIN, HIGH, 25000);  // Right stick vertical

  // Lost signal — stop motors
  if (elevValue == 0 && aileValue == 0) {
    stopAll();
    Serial.println("No RC signal — motors stopped.");
    delay(20);
    return;
  }

  // Apply deadband to remove center stick drift
  int leftSpeed  = applyDeadband(elevValue, DEADBAND);
  int rightSpeed = applyDeadband(aileValue, DEADBAND);

  // Drive each motor independently
  setMotor(motorLeft,  leftSpeed,  INVERT_LEFT);
  setMotor(motorRight, rightSpeed, INVERT_RIGHT);

  // Debug
  Serial.print("Left Stick: ");  Serial.print(elevValue);
  Serial.print(" → L_PWM: ");   Serial.print(leftSpeed);
  Serial.print(" | Right Stick: "); Serial.print(aileValue);
  Serial.print(" → R_PWM: ");   Serial.println(rightSpeed);

  delay(20);
}
