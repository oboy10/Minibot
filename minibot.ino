// TwoWheelBot_PWM - STRIPPED FINAL
#include <Servo.h>




#define LEFT_MOTOR_CH   2
#define RIGHT_MOTOR_CH  3
#define MOTOR_LEFT_PIN  11
#define MOTOR_RIGHT_PIN 13




// ── TUNE THESE ───────────────────────────────
#define LEFT_NEUTRAL    1488
#define RIGHT_NEUTRAL   1500  // Set to R_Raw value when stick is released




#define DEADBAND_LEFT   60    // Increase if left motor moves on L/R stick movement
#define DEADBAND_RIGHT  120    // Increase if right motor spins with no input
#define SMOOTH_FACTOR   8     // Higher = slower stop (2=fast, 10=slow coast)
// ─────────────────────────────────────────────




#define SIGNAL_TIMEOUT_MS 100




Servo motorLeft, motorRight;




volatile unsigned long elevRiseTime = 0, aileRiseTime = 0;
volatile int elevValue  = LEFT_NEUTRAL;
volatile int aileValue  = RIGHT_NEUTRAL;
volatile unsigned long elevLastTime = 0, aileLastTime = 0;




void elevISR() {
  if (digitalRead(LEFT_MOTOR_CH) == HIGH) elevRiseTime = micros();
  else {
    int p = (int)(micros() - elevRiseTime);
    if (p > 800 && p < 2200) { elevValue = p; elevLastTime = millis(); }
  }
}




void aileISR() {
  if (digitalRead(RIGHT_MOTOR_CH) == HIGH) aileRiseTime = micros();
  else {
    int p = (int)(micros() - aileRiseTime);
    if (p > 800 && p < 2200) { aileValue = p; aileLastTime = millis(); }
  }
}




// LEFT:  UP=1000→2000(fwd), DOWN=1976→1000(rev)
int remapLeft(int val)  { return map(val, 1000, 1976, 2000, 1000); }
// RIGHT: UP=1990→2000(fwd), DOWN=1000→1000(rev)
int remapRight(int val) { return map(val, 1990, 1000, 2000, 1000); }




void setMotor(Servo &motor, int pwm) {
  motor.writeMicroseconds(constrain(pwm, 1000, 2000));
}




void stopAll() {
  motorLeft.writeMicroseconds(LEFT_NEUTRAL);
  motorRight.writeMicroseconds(RIGHT_NEUTRAL);
}




void setup() {
  Serial.begin(115200);
  motorLeft.attach(MOTOR_LEFT_PIN);
  motorRight.attach(MOTOR_RIGHT_PIN);
  stopAll();




  Serial.println("Arming SparkMAX...");
  unsigned long s = millis();
  while (millis() - s < 3000) { stopAll(); delay(20); }




  pinMode(LEFT_MOTOR_CH,  INPUT_PULLUP);
  pinMode(RIGHT_MOTOR_CH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(LEFT_MOTOR_CH),  elevISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_MOTOR_CH), aileISR, CHANGE);




  Serial.println("=== Bot Ready ===");
}




float smoothL = LEFT_NEUTRAL, smoothR = RIGHT_NEUTRAL;




void loop() {
  noInterrupts();
  int lr = elevValue, rr = aileValue;
  unsigned long la = millis() - elevLastTime;
  unsigned long ra = millis() - aileLastTime;
  interrupts();




  if (la > SIGNAL_TIMEOUT_MS) lr = LEFT_NEUTRAL;
  if (ra > SIGNAL_TIMEOUT_MS) rr = RIGHT_NEUTRAL;
  if (lr < 800 || lr > 2200) lr = LEFT_NEUTRAL;
  if (rr < 800 || rr > 2200) rr = RIGHT_NEUTRAL;




  int lRemapped = remapLeft(lr);
  int rRemapped = remapRight(rr);




  // Deadbands — snap to neutral if within range
  int lt = (abs(lRemapped - LEFT_NEUTRAL)  < DEADBAND_LEFT)  ? LEFT_NEUTRAL  : lRemapped;
  int rt = (abs(rRemapped - RIGHT_NEUTRAL) < DEADBAND_RIGHT) ? RIGHT_NEUTRAL : rRemapped;




  // Smooth toward target
  smoothL += (lt - smoothL) / SMOOTH_FACTOR;
  smoothR += (rt - smoothR) / SMOOTH_FACTOR;




  // Hard snap to neutral when very close
  if (abs(smoothL - LEFT_NEUTRAL)  < 5) smoothL = LEFT_NEUTRAL;
  if (abs(smoothR - RIGHT_NEUTRAL) < 5) smoothR = RIGHT_NEUTRAL;




  setMotor(motorLeft,  (int)smoothL);
  setMotor(motorRight, (int)smoothR);




  Serial.print("L_Raw:"); Serial.print(lr);
  Serial.print(" L_Out:"); Serial.print((int)smoothL);
  Serial.print(" | R_Raw:"); Serial.print(rr);
  Serial.print(" R_Out:"); Serial.print((int)smoothR);
  Serial.print(" [L:");
  if      (smoothL > LEFT_NEUTRAL  + 5) Serial.print("FWD");
  else if (smoothL < LEFT_NEUTRAL  - 5) Serial.print("REV");
  else                                   Serial.print("STP");
  Serial.print(" R:");
  if      (smoothR > RIGHT_NEUTRAL + 5) Serial.print("FWD");
  else if (smoothR < RIGHT_NEUTRAL - 5) Serial.print("REV");
  else                                   Serial.print("STP");
  Serial.println("]");




  delay(30);
}









