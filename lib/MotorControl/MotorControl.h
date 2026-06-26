#pragma once

#include <Arduino.h>

enum MotorFault {
  MOTOR_FAULT_NONE,
  MOTOR_FAULT_POTI_INVALID,
  MOTOR_FAULT_TIMEOUT
};

extern bool moveActive;
extern unsigned long moveStart_ms;

void initMotorControl();
MotorFault getMotorFault();
void clearMotorFault();
int getMotorTargetPct();
void setMotorTargetPct(int pct);
int tempToSetpoint(float temp);
void motorDriveToward(int curPct, int tgtPct);
void motorStop();
void motorControlTask(unsigned long now);
