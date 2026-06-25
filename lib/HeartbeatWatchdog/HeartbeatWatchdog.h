#pragma once

#include <Arduino.h>

extern volatile unsigned long lastHB_RX_ms;

void IRAM_ATTR hbISR();
void initRelay();
void initHeartbeat();
void heartbeatTask(unsigned long now);