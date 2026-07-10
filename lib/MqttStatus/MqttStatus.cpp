#include "MqttStatus.h"

#include <Arduino.h>
#include <espMqttClient.h>

#include "WifiTime.h"
#include "../../include/mqtt_secrets.h"

static espMqttClient mqttClient;
static constexpr char MQTT_CLIENT_ID[] = "anzuchtzelt-esp32";
static constexpr unsigned long MQTT_RECONNECT_INTERVAL_MS = 5000;
static constexpr char MQTT_AVAILABILITY_TOPIC[] = "anzuchtzelt/status";
static constexpr char MQTT_PAYLOAD_ONLINE[] = "online";
static constexpr char MQTT_PAYLOAD_OFFLINE[] = "offline";
static constexpr uint8_t MQTT_AVAILABILITY_QOS = 1;
static unsigned long lastMqttConnectAttempt_ms = 0;
static bool mqttConnectAttempted = false;

static void onMqttConnect(bool sessionPresent) {
  (void)sessionPresent;
  Serial.println(F("[MQTT] Verbunden."));

  const uint16_t packetId = mqttClient.publish(
      MQTT_AVAILABILITY_TOPIC,
      MQTT_AVAILABILITY_QOS,
      true,
      MQTT_PAYLOAD_ONLINE);
  if (packetId == 0) {
    Serial.println(F("[MQTT] Online-Status konnte nicht gesendet werden."));
  }
}

void initMqttStatus() {
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASSWORD);
  mqttClient.setClientId(MQTT_CLIENT_ID);
  mqttClient.setWill(
      MQTT_AVAILABILITY_TOPIC,
      MQTT_AVAILABILITY_QOS,
      true,
      MQTT_PAYLOAD_OFFLINE);
  mqttClient.onConnect(onMqttConnect);
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
