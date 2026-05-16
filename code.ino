#include <Servo.h>
Servo doorServo;

// ---------------- PIN DEFINITIONS ----------------
#define BTN_MODE 5
#define BTN_DOOR 13
#define PIR1 2//for light and fan
#define PIR2 8// for intruder alarm
#define BUZZER 3
#define IN1 11
#define IN2 12
#define ENA 10
#define SERVO_PIN 9
#define LED_SECURITY 6
#define LED_INTRUDER 7
#define LED_LIGHT 4

#define THERMISTOR A1
#define MQ_SENSOR A2
#define LDR A0

// ---------------- VARIABLES ----------------
bool securityMode = false;
bool lastBtnState = HIGH;
bool doorState = false; // false = closed, true = open

// threshold values
int temp1 = 100;
int temp2 = 200;
int temp3 = 300;
int smokeThreshold = 150;
int lightThreshold = 100;

// ---------------- SETUP ----------------
void setup() {
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_DOOR, INPUT_PULLUP);
  pinMode(PIR1, INPUT);
  pinMode(PIR2, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(LED_SECURITY, OUTPUT);
  pinMode(LED_INTRUDER, OUTPUT);
  pinMode(LED_LIGHT, OUTPUT);

  doorServo.attach(SERVO_PIN);
  doorServo.write(0); // door closed

  Serial.begin(9600);
}

void loop() {

  // -------- MODULE 1: MODE TOGGLE --------
  bool btnState = digitalRead(BTN_MODE);
  if (btnState == LOW && lastBtnState == HIGH) {
    securityMode = !securityMode;
    delay(200); // debounce
  }
  lastBtnState = btnState;
  digitalWrite(LED_SECURITY, securityMode);

  // Read sensors
  int tempVal = analogRead(THERMISTOR);
  int smokeVal = analogRead(MQ_SENSOR);
  Serial.println(tempVal);
  int lightVal = analogRead(LDR);

  bool motion1 = digitalRead(PIR1);
  Serial.println(motion1);
  bool motion2 = digitalRead(PIR2);

  // -------- MODULE 2: TEMP + FAN --------
  if (motion1) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    if ((tempVal > temp1) && (tempVal < temp2)) {
      analogWrite(ENA, 100);
    } 
    else if ((tempVal > temp2) && (tempVal < temp3)) {
      analogWrite(ENA, 180);
    } 
    else if (tempVal > temp3) {
      analogWrite(ENA, 255);
    } 
    else {
      analogWrite(ENA, 0);
    }
  } else {
    analogWrite(ENA, 0);
  }

  // -------- MODULE 3: SMOKE ALERT --------
  if (smokeVal > smokeThreshold) {
    tone(BUZZER, 1000); // smoke sound

    if (!securityMode) {
      doorServo.write(90); // open
      doorState = true;

    } else {
      doorServo.write(0); // keep closed
      doorState = false;
    }doorServo.write(0);
  } else {
    noTone(BUZZER);
  }

  // -------- MODULE 4: DOOR BUTTON --------
  if (digitalRead(BTN_DOOR) == LOW) {
    delay(200);

    if (!securityMode) {
      doorState = !doorState;
      doorServo.write(doorState ? 90 : 0);
    } else {
      tone(BUZZER, 2000); // different alert
      delay(300);
      noTone(BUZZER);
    }
  }

  // -------- MODULE 5: INTRUDER ALERT --------
  if (motion2) {
    if (!securityMode) {
      digitalWrite(LED_INTRUDER, HIGH);
      delay(200);
      digitalWrite(LED_INTRUDER, LOW);
    } else {
      tone(BUZZER, 1500); // different tone
    }
  } else {
    digitalWrite(LED_INTRUDER, LOW);
  }

  // -------- MODULE 6: AUTO LIGHT --------
  if (motion1 && lightVal < lightThreshold) {
    digitalWrite(LED_LIGHT, HIGH);
  } else {
    digitalWrite(LED_LIGHT, LOW);
  }

}