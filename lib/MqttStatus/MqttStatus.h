#pragma once

void initMqttStatus();
void mqttStatusTask(unsigned long now);
bool isMqttConnected();
