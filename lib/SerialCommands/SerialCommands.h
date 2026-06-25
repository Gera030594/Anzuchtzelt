#pragma once

#include <Arduino.h>

void changeLogMaybe(unsigned long now);
void printStatus();
void serialCommandTask(unsigned long now);