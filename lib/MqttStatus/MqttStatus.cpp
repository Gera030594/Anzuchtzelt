#include "MqttStatus.h"

#include <Arduino.h>
#include <espMqttClient.h>

#include "WifiTime.h"
#include "../../include/mqtt_secrets.h"

static espMqttClient mqttClient;
static constexpr char MQTT_CLIENT_ID[] = "anzuchtzelt-esp32";
static constexpr unsigned long MQTT_RECONNECT_INTERVAL_MS = 5000;
static unsigned long lastMqttConnectAttempt_ms = 0;
static bool mqttConnectAttempted = false;

void initMqttStatus() {
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASSWORD);
  mqttClient.setClientId(MQTT_CLIENT_ID);
  Serial.println(F("[MQTT] Client konfiguriert."));
}

void mqttStatusTask(unsigned long now) {
  if (!isWifiConnected()) return;
  if (mqttClient.connected()) return;
  if (!mqttClient.disconnected()) return;
  const bool reconnectIntervalElapsed =
      now - lastMqttConnectAttempt_ms >= MQTT_RECONNECT_INTERVAL_MS;
  if (mqttConnectAttempted && !reconnectIntervalElapsed) return;

  mqttConnectAttempted = true;
  lastMqttConnectAttempt_ms = now;
  Serial.println(F("[MQTT] Verbindungsversuch."));
  mqttClient.connect();
}

bool isMqttConnected() {
  return mqttClient.connected();
}
