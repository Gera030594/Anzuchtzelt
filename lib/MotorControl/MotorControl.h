#pragma once

#include <Arduino.h>

extern int targetPct;
extern bool moveActive;
extern unsigned long moveStart_ms;

void initMotorControl();
int tempToSetpoint(float temp);
void motorDriveToward(int curPct, int tgtPct);
void motorStop();
void motorControlTask(unsigned long now);