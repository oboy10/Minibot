// TwoWheelBot_PWM - FINAL
#include <Servo.h>


#define LEFT_MOTOR_CH   3
#define RIGHT_MOTOR_CH  2
#define MOTOR_LEFT_PIN  11
#define MOTOR_RIGHT_PIN 12


#define LEFT_NEUTRAL  1488
#define RIGHT_NEUTRAL 1600


#define DEADBAND_LEFT  50
#define DEADBAND_RIGHT 30   // Small deadband just to prevent drift
#define SMOOTH_FACTOR  4
#define KICK_AMOUNT    150  // Only used for LEFT motor now
#define KICK_DURATION  200
#define SIGNAL_TIMEOUT_MS 100


Servo motorLeft, motorRight;


volatile unsigned long elevRiseTime = 0, aileRiseTime = 0;
volatile int elevValue = LEFT_NEUTRAL, aileValue = RIGHT_NEUTRAL;
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


int remapLeft(int val) {
  return map(val, 1000, 1976, 2000, 1000);
}


int remapRight(int val) {
  return map(val, 1990, 1000, 2000, 1000);
}


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
bool lwas = true;
unsigned long lkick = 0;


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


  // Deadbands
  int lt = (abs(lRemapped - LEFT_NEUTRAL)  < DEADBAND_LEFT)  ? LEFT_NEUTRAL  : lRemapped;
  int rt = (abs(rRemapped - RIGHT_NEUTRAL) < DEADBAND_RIGHT) ? RIGHT_NEUTRAL : rRemapped;


  // Kick-start LEFT only
  if (lt != LEFT_NEUTRAL && lwas) { lkick = millis(); lwas = false; }
  if (lt == LEFT_NEUTRAL) lwas = true;
  if (lt != LEFT_NEUTRAL && (millis() - lkick) < KICK_DURATION)
    lt = constrain(lt + ((lt > LEFT_NEUTRAL) ? 1 : -1) * KICK_AMOUNT, 1000, 2000);


  // RIGHT motor: no kick, direct smooth
  smoothL += (lt - smoothL) / SMOOTH_FACTOR;
  smoothR += (rt - smoothR) / SMOOTH_FACTOR;


  if (abs(smoothL - LEFT_NEUTRAL)  < 5) smoothL = LEFT_NEUTRAL;
  if (abs(smoothR - RIGHT_NEUTRAL) < 5) smoothR = RIGHT_NEUTRAL;


  setMotor(motorLeft,  (int)smoothL);
  setMotor(motorRight, (int)smoothR);


  Serial.print("L_Raw:"); Serial.print(lr);
  Serial.print(" L_Out:"); Serial.print((int)smoothL);
  Serial.print(" | R_Raw:"); Serial.print(rr);
  Serial.print(" R_Mapped:"); Serial.print(rRemapped);
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






