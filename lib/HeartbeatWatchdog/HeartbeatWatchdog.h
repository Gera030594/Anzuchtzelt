#pragma once

#include <Arduino.h>

extern volatile unsigned long lastHB_RX_ms;

enum class HeartbeatStatus {
  Grace,
  Ok,
  Timeout
};

void initRelay();
void initHeartbeat();
void heartbeatTask(unsigned long now);
HeartbeatStatus getHeartbeatStatus(unsigned long now);
