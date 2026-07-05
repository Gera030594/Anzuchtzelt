#pragma once

#include <Arduino.h>
#include <time.h>

extern bool timeSynced;

void handleWiFi(unsigned long now);
void handleNTP(unsigned long now);
void connectWiFi();
void checkWiFiReconnect();
void synchronizeTime();
bool isSommerzeit(struct tm* timeinfo);
bool isWifiConnected();
bool isTimeSynced();
