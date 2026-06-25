#pragma once

#include <Arduino.h>

void initHardwareLampensteuerung();
void handleLamp(unsigned long now);
void checkLampState();
void updateModeLed();