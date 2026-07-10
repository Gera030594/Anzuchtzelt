#include "MqttStatus.h"

#include <Arduino.h>
#include <espMqttClient.h>
#include <stdio.h>

#include "BmeSensor.h"
#include "WifiTime.h"
#include "../../include/mqtt_secrets.h"

static espMqttClient mqttClient;
static constexpr char MQTT_CLIENT_ID[] = "anzuchtzelt-esp32";
static constexpr unsigned long MQTT_RECONNECT_INTERVAL_MS = 5000;
static constexpr char MQTT_AVAILABILITY_TOPIC[] = "anzuchtzelt/status";
static constexpr char MQTT_PAYLOAD_ONLINE[] = "online";
static constexpr char MQTT_PAYLOAD_OFFLINE[] = "offline";
static constexpr uint8_t MQTT_AVAILABILITY_QOS = 1;
static constexpr char MQTT_TEMPERATURE_TOPIC[] =
    "anzuchtzelt/sensor/temperature";
static constexpr char MQTT_HUMIDITY_TOPIC[] =
    "anzuchtzelt/sensor/humidity";
static constexpr char MQTT_BME_OK_TOPIC[] =
    "anzuchtzelt/status/bme_ok";
static constexpr char MQTT_PAYLOAD_TRUE[] = "true";
static constexpr char MQTT_PAYLOAD_FALSE[] = "false";
static constexpr uint8_t MQTT_SENSOR_QOS = 1;
static constexpr unsigned long MQTT_SENSOR_PUBLISH_INTERVAL_MS = 10000;
static unsigned long lastMqttConnectAttempt_ms = 0;
static bool mqttConnectAttempted = false;
static bool mqttWasConnected = false;
static unsigned long lastMqttSensorPublish_ms = 0;

static void publishBmeStatus() {
  float temperature = 0.0f;
  float humidity = 0.0f;
  const bool temperatureValid = tryGetBmeTemperatureC(temperature);
  const bool humidityValid = tryGetBmeHumidityPct(humidity);
  const bool bmeOk =
      !hasBmeDisplayError() && temperatureValid && humidityValid;

  const char* bmeStatusPayload = bmeOk ? MQTT_PAYLOAD_TRUE : MQTT_PAYLOAD_FALSE;
  if (mqttClient.publish(
          MQTT_BME_OK_TOPIC,
          MQTT_SENSOR_QOS,
          true,
          bmeStatusPayload) == 0) {
    Serial.println(F("[MQTT] BME-Status konnte nicht gesendet werden."));
  }

  if (!bmeOk) return;

  char temperaturePayload[16];
  char humidityPayload[16];
  snprintf(temperaturePayload, sizeof(temperaturePayload), "%.2f", temperature);
  snprintf(humidityPayload, sizeof(humidityPayload), "%.2f", humidity);

  if (mqttClient.publish(
          MQTT_TEMPERATURE_TOPIC,
          MQTT_SENSOR_QOS,
          true,
          temperaturePayload) == 0) {
    Serial.println(F("[MQTT] Temperatur konnte nicht gesendet werden."));
  }

  if (mqttClient.publish(
          MQTT_HUMIDITY_TOPIC,
          MQTT_SENSOR_QOS,
          true,
          humidityPayload) == 0) {
    Serial.println(F("[MQTT] Luftfeuchtigkeit konnte nicht gesendet werden."));
  }
}

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
  if (!isWifiConnected()) {
    mqttWasConnected = false;
    return;
  }

  if (mqttClient.connected()) {
    const bool newConnection = !mqttWasConnected;
    mqttWasConnected = true;
    const bool publishIntervalElapsed =
        now - lastMqttSensorPublish_ms >= MQTT_SENSOR_PUBLISH_INTERVAL_MS;
    if (newConnection || publishIntervalElapsed) {
      lastMqttSensorPublish_ms = now;
      publishBmeStatus();
    }
    return;
  }

  mqttWasConnected = false;
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
