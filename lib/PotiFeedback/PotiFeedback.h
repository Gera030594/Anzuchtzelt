#pragma once

#include <Arduino.h>

extern int potiRawNow;
extern bool potiValid;

void initPoti();
int readPotiRaw();
int readPotiRawInstant();
void readPotiTask(unsigned long now);