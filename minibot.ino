// FourWheelBot_PWM
// RC Controller to 4-Wheel Drive Motor Control
// Left stick (Elevator) = Forward/Backward
// Right stick (Aileron) = Left/Right turning (tank-style differential)

/*
PWM Pulse Value Limits for REV SparkMAX:
- Full Reverse:  1000 microseconds
- Neutral:       1500 microseconds
- Full Forward:  2000 microseconds

Wiring:
  RC Receiver → Arduino
  Aile  → Pin 0  (Right/Left steering axis)
  Elev  → Pin 1  (Forward/Backward throttle axis)
  Thro  → Pin 2  (unused / available)
  Rudd  → Pin 3  (unused / available)

  SparkMAX PWM signals:
  Front Left  Motor → Pin 10
  Front Right Motor → Pin 11
  Rear Left   Motor → Pin 12
  Rear Right  Motor → Pin 13

Motor orientation note:
  Left-side motors are physically mirrored from right-side.
  Set INVERT_LEFT true if your left motors spin opposite direction.
*/

#include <Servo.h>

// RC Receiver Input Pins
#define AILE_PIN   0   // Steering (left/right)
#define ELEV_PIN   1   // Throttle (forward/back)
#define THRO_PIN   2
#define RUDD_PIN   3
#define RIGHT_PIN  4
#define BUTTON_PIN 5
#define LEFT_PIN   6
#define SCROLL_PIN 7

// Motor Output Pins (PWM to SparkMAX)
#define MOTOR_FL_PIN 10   // Front Left
#define MOTOR_FR_PIN 11   // Front Right
#define MOTOR_RL_PIN 12   // Rear Left
#define MOTOR_RR_PIN 13   // Rear Right

// Tuning
#define DEADBAND       30    // Ignore stick inputs within ±30 of 1500 (center)
#define INVERT_LEFT    true  // Set true if left motors need to spin reversed vs right
#define INVERT_RIGHT   false

// SparkMAX neutral
#define NEUTRAL 1500

Servo motorFL, motorFR, motorRL, motorRR;

// RC channel values
int aileValue, elevValue, throValue, ruddValue;

// Helper: apply deadband around center (1500)
int applyDeadband(int val, int deadband) {
  int offset = val - NEUTRAL;
  if (abs(offset) < deadband) return NEUTRAL;
  return val;
}

// Helper: invert a PWM value around neutral (1500 → 1500, 2000 → 1000, 1000 → 2000)
int invertPWM(int val) {
  return NEUTRAL - (val - NEUTRAL);
}

void setMotor(Servo &motor, int pwmValue, bool invert) {
  pwmValue = constrain(pwmValue, 1000, 2000);
  if (invert) pwmValue = invertPWM(pwmValue);
  motor.writeMicroseconds(pwmValue);
}

void stopAll() {
  motorFL.writeMicroseconds(NEUTRAL);
  motorFR.writeMicroseconds(NEUTRAL);
  motorRL.writeMicroseconds(NEUTRAL);
  motorRR.writeMicroseconds(NEUTRAL);
}

void setup() {
  Serial.begin(9600);

  // Set RC pins as inputs
  pinMode(AILE_PIN,   INPUT);
  pinMode(ELEV_PIN,   INPUT);
  pinMode(THRO_PIN,   INPUT);
  pinMode(RUDD_PIN,   INPUT);
  pinMode(RIGHT_PIN,  INPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LEFT_PIN,   INPUT);
  pinMode(SCROLL_PIN, INPUT);

  // Attach motors
  motorFL.attach(MOTOR_FL_PIN);
  motorFR.attach(MOTOR_FR_PIN);
  motorRL.attach(MOTOR_RL_PIN);
  motorRR.attach(MOTOR_RR_PIN);

  // Initialize all motors to neutral
  stopAll();
  delay(2000); // SparkMAX initialization window

  Serial.println("4-Wheel Bot Ready.");
  Serial.println("Elev = Throttle (Fwd/Rev) | Aile = Steer (Left/Right)");
}

void loop() {
  // Read RC channels
  aileValue  = pulseIn(AILE_PIN,   HIGH, 25000); // 25ms timeout
  elevValue  = pulseIn(ELEV_PIN,   HIGH, 25000);
  throValue  = pulseIn(THRO_PIN,   HIGH, 25000);
  ruddValue  = pulseIn(RUDD_PIN,   HIGH, 25000);

  // Apply deadband to avoid drift at center
  int throttle = applyDeadband(elevValue, DEADBAND);
  int steering = applyDeadband(aileValue, DEADBAND);

  // If either channel has no signal, stop everything
  if (elevValue == 0 || aileValue == 0) {
    stopAll();
    Serial.println("No RC signal — motors stopped.");
    return;
  }

  // Convert to offsets from neutral (-500 to +500)
  int throttleOffset = throttle - NEUTRAL;  // + = forward, - = reverse
  int steerOffset    = steering - NEUTRAL;  // + = right,   - = left

  // Tank-style differential mixing:
  //   Left side  = throttle + steering
  //   Right side = throttle - steering
  int leftSpeed  = NEUTRAL + throttleOffset + steerOffset;
  int rightSpeed = NEUTRAL + throttleOffset - steerOffset;

  // Clamp to valid SparkMAX range
  leftSpeed  = constrain(leftSpeed,  1000, 2000);
  rightSpeed = constrain(rightSpeed, 1000, 2000);

  // Write to motors (left side may need inversion depending on motor orientation)
  setMotor(motorFL, leftSpeed,  INVERT_LEFT);
  setMotor(motorRL, leftSpeed,  INVERT_LEFT);
  setMotor(motorFR, rightSpeed, INVERT_RIGHT);
  setMotor(motorRR, rightSpeed, INVERT_RIGHT);

  // Debug output
  Serial.print("Thro(raw): "); Serial.print(elevValue);
  Serial.print(" | Steer(raw): "); Serial.print(aileValue);
  Serial.print(" | L_PWM: "); Serial.print(leftSpeed);
  Serial.print(" | R_PWM: "); Serial.println(rightSpeed);

  delay(20);
}
