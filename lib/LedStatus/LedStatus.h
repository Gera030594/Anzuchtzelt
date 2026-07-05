#pragma once

#include <Arduino.h>
#include <NeoPixelBus.h>

RgbColor C(uint8_t r, uint8_t g, uint8_t b);
RgbColor withBri(const RgbColor& c);
void ledSet(uint8_t i, const RgbColor& color);
void initLEDs();
void updateZoneLEDs(float temp);
void updateModeLed();
void ledUpdateTask();
