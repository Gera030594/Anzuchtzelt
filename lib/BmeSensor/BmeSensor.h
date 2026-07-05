#pragma once

#include <Arduino.h>

extern float T;
extern float RH;
extern bool bmeOK;
extern unsigned long lastBMEGood_ms;

void bmeInit();
void bmeInitialCheck();
void bmeConfigure();
void bmeTryInitNow();
void bmeService(unsigned long now);
void bme680Task(unsigned long now);
bool bmeHealthy(unsigned long now);
bool hasBmeDisplayError();
bool hasValidHumidityForDisplay();
