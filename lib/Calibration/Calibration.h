#pragma once

#include <Arduino.h>
#include <Preferences.h>

extern Preferences prefs;

int getCalibrationPointCount();
bool isCalibrationModeActive();
void openNVS();
void calInitLoad();
void calSave();
void calList();
void calEnter();
void calExit();
void calCaptureSlot(uint8_t slotIdx);
void calRebuildSortedFromSlots();
int rawToPercent(int raw);
