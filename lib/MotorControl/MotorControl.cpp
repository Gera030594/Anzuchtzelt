#include "MotorControl.h"

#include <stdlib.h>

#include "Calibration.h"
#include "Config.h"
#include "Pins.h"
#include "PotiFeedback.h"

static int targetPct = 25;                   // Soll-Position in %
int activeTargetPct = 25;                    // Zielwert der aktuell laufenden Bewegung
int moveDir = 0;                             // +1 = hoch, -1 = runter, 0 = keine Bewegung
static MotorFault motorFault = MOTOR_FAULT_NONE;  // Motorfehlerstatus
bool moveActive = false;                     // Bewegung aktiv?
unsigned long moveStart_ms = 0;              // Startzeit aktive Bewegung

MotorFault getMotorFault() {
  return motorFault;
}

void clearMotorFault() {
  motorFault = MOTOR_FAULT_NONE;
}

int getMotorTargetPct() {
  return targetPct;
}

bool tryGetMotorPositionPct(int& value) {
  if (!potiValid) return false;

  const int raw = potiRawNow;
  const int pct = rawToPercent(raw);
  if (pct < 0 || pct > 100) return false;

  value = pct;
  return true;
}

bool isPotiFeedbackValid() {
  return potiValid;
}

void setMotorTargetPct(int pct) {
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  targetPct = pct;
}

/* int tempToSetpoint(float temp) {
  if (temp < 22.0f) return 10;      // <22°C -> 10%
  if (temp >= 32.0f) return 100;    // ab 32°C -> 100%

  // 22..32°C -> linear 10..100%
  float rel = (temp - 22.0f) / 10.0f;  // 0..1
  float pct = 10.0f + rel * 90.0f;     // 10..100

  if (pct < 10.0f) pct = 10.0f;
  if (pct > 100.0f) pct = 100.0f;

  return (int)(pct + 0.5f);            // gerundet
}
*/

int tempToSetpoint(float temp) {
  if (temp < 22.0f) return 10;     // unter 22°C -> 10%
  if (temp >= 32.0f) return 60;    // ab 32°C -> 60%

  // 22..32°C -> linear 10..60%
  float rel = (temp - 22.0f) / 10.0f;  // 0..1
  float pct = 10.0f + rel * 50.0f;     // 10..60

  if (pct < 10.0f) pct = 10.0f;
  if (pct > 60.0f) pct = 60.0f;

  return (int)(pct + 0.5f);            // gerundet
}

void motorDriveToward(int curPct, int tgtPct) {
  int err = tgtPct - curPct;  // Fehler (Ziel - Ist)
  if (err == 0) {
    motorStop();
    return;
  }

  // Proportionale PWM (einfach): größerer Fehler -> mehr PWM
  int duty = (int)(abs(err) * (MAX_DUTY - MIN_DUTY) / 100) + MIN_DUTY;  // 2..100% -> MIN..MAX
  if (duty > MAX_DUTY) duty = MAX_DUTY;                                 // Obergrenze
  if (duty < MIN_DUTY) duty = MIN_DUTY;                                 // Untergrenze
  if (err > 0) {                                                        // Ziel größer als Ist -> vorwärts
    digitalWrite(L298N_IN1, HIGH);                                      // Richtung setzen
    digitalWrite(L298N_IN2, LOW);                                       // Gegenrichtung aus
  } else {                                                              // Ziel kleiner -> rückwärts
    digitalWrite(L298N_IN1, LOW);                                       // Richtung setzen
    digitalWrite(L298N_IN2, HIGH);                                      // Gegenrichtung aus
  }
  ledcWrite(PWM_CHANNEL, duty);  // PWM ausgeben (0..1023)
}

void motorStop() {
  ledcWrite(PWM_CHANNEL, 0);     // PWM=0 (Motor stromlos)
  digitalWrite(L298N_IN1, LOW);  // beide Richtungen LOW
  digitalWrite(L298N_IN2, LOW);  // (H-Brücke frei)
}

void initMotorControl() {
  pinMode(L298N_IN1, OUTPUT);
  digitalWrite(L298N_IN1, LOW);  // Richtungspin 1
  pinMode(L298N_IN2, OUTPUT);
  digitalWrite(L298N_IN2, LOW);                                       // Richtungspin 2
  ledcAttachChannel(L298N_ENA, PWM_FREQ, PWM_RES_BITS, PWM_CHANNEL);  // LEDC v3: Pin an Kanal binden
  ledcWrite(PWM_CHANNEL, 0);                                          // PWM = 0 (Motor aus)
  Serial.print(F("[MOTOR] PWM "));
  Serial.print(PWM_FREQ);
  Serial.print(F(" Hz, "));
  Serial.print(PWM_RES_BITS);
  Serial.println(F(" Bit konfiguriert."));
}

void motorControlTask(unsigned long now) {
  /***** 4) Motor-Regelung (non-blocking) *****/
  if (motorFault == MOTOR_FAULT_TIMEOUT) {
    motorStop();
    moveActive = false;
    moveDir = 0;
    return;
  }

  if (!potiValid) {
    motorFault = MOTOR_FAULT_POTI_INVALID;
    motorStop();
    moveActive = false;
    moveDir = 0;
    return;
  }

  // Aktuelle Poti-Position in % bestimmen
  // ADC lesen
  int curPct = rawToPercent(potiRawNow);  // in %

  if (!moveActive) {
    // Totband entscheidet nur, ob eine neue Bewegung gestartet wird.
    if (abs(curPct - targetPct) <= DEAD_BAND_PCT) {
      return;
    }

    activeTargetPct = targetPct;
    if (activeTargetPct > curPct) {
      moveDir = 1;
    } else if (activeTargetPct < curPct) {
      moveDir = -1;
    } else {
      moveDir = 0;
    }

    moveActive = true;
    moveStart_ms = now;
  }

  bool targetReached = (moveDir > 0 && curPct >= activeTargetPct) ||
                       (moveDir < 0 && curPct <= activeTargetPct);

  if (moveDir == 0 || targetReached) {
    motorStop();
    moveActive = false;
    moveDir = 0;
    return;
  }

  motorDriveToward(curPct, activeTargetPct);  // Richtung + PWM setzen
  // Timeout prüfen
  if (now - moveStart_ms > MOVE_TIMEOUT) {  // >10 s ohne Zielerreichung?
    motorStop();                            // Motor aus
    motorFault = MOTOR_FAULT_TIMEOUT;
    Serial.print(F("[MOTOR] ERROR: MOVE_TIMEOUT cur="));
    Serial.print(curPct);
    Serial.print(F("% target="));
    Serial.print(activeTargetPct);
    Serial.print(F("% dir="));
    Serial.print(moveDir);
    Serial.print(F(" elapsed="));
    Serial.print(now - moveStart_ms);
    Serial.println(F("ms"));
    moveActive = false;                     // Bewegung abbrechen
    moveDir = 0;
  }
}
