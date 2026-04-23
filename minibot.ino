// RWD 2-Wheel Bot — Tank Style
// Left stick UP/DOWN  → Left motor  (D2)
// Right stick UP/DOWN → Right motor (change RIGHT_CH to your pin)




#include <Servo.h>




#define LEFT_CH   3    // ✅ confirmed
#define RIGHT_CH  2    // ← change this to whichever pin right stick lands on




#define MOTOR_L_PIN  11
#define MOTOR_R_PIN  13




#define NEUTRAL   1500
#define DEADBAND  40




Servo motorL, motorR;




int applyDeadband(int val) {
  return (abs(val - NEUTRAL) < DEADBAND) ? NEUTRAL : val;
}




void setup() {
  Serial.begin(115200);
  pinMode(LEFT_CH,  INPUT);
  pinMode(RIGHT_CH, INPUT);




  motorL.attach(MOTOR_L_PIN);
  motorR.attach(MOTOR_R_PIN);
  motorL.writeMicroseconds(NEUTRAL);
  motorR.writeMicroseconds(NEUTRAL);
  delay(2000); // SparkMAX arm time
  Serial.println("Bot Ready.");
}




void loop() {
  int leftRaw  = pulseIn(LEFT_CH,  HIGH, 25000);
  int rightRaw = pulseIn(RIGHT_CH, HIGH, 25000);




  // No signal = stop
  if (leftRaw  == 0) leftRaw  = NEUTRAL;
  if (rightRaw == 0) rightRaw = NEUTRAL;




  int leftOut  = applyDeadband(constrain(leftRaw,  1000, 2000));
  int rightOut = applyDeadband(constrain(rightRaw, 1000, 2000));




  motorL.writeMicroseconds(leftOut);
  motorR.writeMicroseconds(rightOut);




  Serial.print("L:"); Serial.print(leftOut);
  Serial.print("  R:"); Serial.println(rightOut);




  delay(20);
}









