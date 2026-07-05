#pragma once

#include <Arduino.h>

enum class GrowPhase {
  Vegetation,
  Flowering
};

GrowPhase getGrowPhase();
void initHardwareLampensteuerung();
void handleLamp(unsigned long now);
void checkLampState();
