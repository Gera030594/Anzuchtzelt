#pragma once

#include <Arduino.h>

enum class GrowPhase {
  Vegetation,
  Flowering
};

GrowPhase getGrowPhase();
bool isLampRelayOn();
void updateGrowPhase(unsigned long now);
void initHardwareLampensteuerung();
void handleLamp(unsigned long now);
void checkLampState();
