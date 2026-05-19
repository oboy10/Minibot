#include <Servo.h>

#define THROTTLE_CH   3
#define TURN_CH       9

#define MOTOR_L_PIN   13
#define MOTOR_R_PIN   11

int THROTTLE_CENTER = 1500;
int TURN_CENTER     = 1500;

#define DEADBAND_THROTTLE 120
#define DEADBAND_TURN     60
#define TURN_TRIM         0
#define TURN_SCALE        0.60

Servo motorL, motorR;

int applyDeadband(int val, int center, int deadband) {
  return (abs(val - center) < deadband) ? center : val;
}

void setup() {
  Serial.begin(115200);

  pinMode(THROTTLE_CH, INPUT);
  pinMode(TURN_CH, INPUT);

  motorL.attach(MOTOR_L_PIN);
  motorR.attach(MOTOR_R_PIN);

  motorL.writeMicroseconds(1500);
  motorR.writeMicroseconds(1500);

  delay(2000);
  Serial.println("Arcade Drive Calibrated.");
}

void loop() {
  int throttleRaw = pulseIn(THROTTLE_CH, HIGH, 25000);
  int turnRaw     = pulseIn(TURN_CH, HIGH, 25000);

  if (throttleRaw == 0) throttleRaw = THROTTLE_CENTER;
  if (turnRaw == 0)     turnRaw     = TURN_CENTER;

  int throttleCmd = applyDeadband(throttleRaw, THROTTLE_CENTER, DEADBAND_THROTTLE);
  int turnCmd     = applyDeadband(turnRaw + TURN_TRIM, TURN_CENTER, DEADBAND_TURN);

  int throttle = throttleCmd - THROTTLE_CENTER;
  int turn     = (turnCmd - TURN_CENTER) * TURN_SCALE;

  int leftOut  = 1500 + throttle + turn;
  int rightOut = 1500 + throttle - turn;

  leftOut  = 3000 - leftOut;
  rightOut = 3000 - rightOut;

  leftOut  = constrain(leftOut, 1000, 2000);
  rightOut = constrain(rightOut, 1000, 2000);

  motorL.writeMicroseconds(leftOut);
  motorR.writeMicroseconds(rightOut);

  Serial.print("ThrottleRaw:");
  Serial.print(throttleRaw);
  Serial.print("  TurnRaw:");
  Serial.print(turnRaw);
  Serial.print("  Throttle:");
  Serial.print(throttle);
  Serial.print("  Turn:");
  Serial.print(turn);
  Serial.print("  Lout:");
  Serial.print(leftOut);
  Serial.print("  Rout:");
  Serial.println(rightOut);

  delay(20);
}


