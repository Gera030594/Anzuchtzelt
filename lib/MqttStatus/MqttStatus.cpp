#include "MqttStatus.h"

#include <Arduino.h>
#include <espMqttClient.h>
#include <stdio.h>

#include "BmeSensor.h"
#include "HeartbeatWatchdog.h"
#include "LampControl.h"
#include "MotorControl.h"
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
static constexpr char MQTT_HEARTBEAT_TOPIC[] =
    "anzuchtzelt/status/heartbeat";
static constexpr char MQTT_MOTOR_FAULT_TOPIC[] =
    "anzuchtzelt/status/motor_fault";
static constexpr uint8_t MQTT_OPERATIONAL_STATUS_QOS = 1;
static constexpr char MQTT_GROW_PHASE_TOPIC[] =
    "anzuchtzelt/status/grow_phase";
static constexpr char MQTT_LAMP_MODE_TOPIC[] =
    "anzuchtzelt/status/lamp_mode";
static constexpr uint8_t MQTT_GROW_STATUS_QOS = 1;
static constexpr char MQTT_DISCOVERY_TEMPERATURE_TOPIC[] =
    "homeassistant/sensor/anzuchtzelt_temperature/config";
static constexpr char MQTT_DISCOVERY_HUMIDITY_TOPIC[] =
    "homeassistant/sensor/anzuchtzelt_humidity/config";
static constexpr char MQTT_DISCOVERY_BME_ERROR_TOPIC[] =
    "homeassistant/binary_sensor/anzuchtzelt_bme_error/config";
static constexpr char MQTT_DISCOVERY_HEARTBEAT_TOPIC[] =
    "homeassistant/sensor/anzuchtzelt_heartbeat/config";
static constexpr char MQTT_DISCOVERY_MOTOR_FAULT_TOPIC[] =
    "homeassistant/sensor/anzuchtzelt_motor_fault/config";
static constexpr uint8_t MQTT_DISCOVERY_QOS = 1;
static constexpr char MQTT_DISCOVERY_TEMPERATURE_PAYLOAD[] = R"json({
  "name": "Temperatur",
  "unique_id": "anzuchtzelt_temperature",
  "state_topic": "anzuchtzelt/sensor/temperature",
  "device_class": "temperature",
  "state_class": "measurement",
  "unit_of_measurement": "\u00B0C",
  "qos": 1,
  "availability": [
    {
      "topic": "anzuchtzelt/status",
      "payload_available": "online",
      "payload_not_available": "offline"
    },
    {
      "topic": "anzuchtzelt/status/bme_ok",
      "payload_available": "true",
      "payload_not_available": "false"
    }
  ],
  "availability_mode": "all",
  "device": {
    "identifiers": ["anzuchtzelt_esp32"],
    "name": "Anzuchtzelt",
    "manufacturer": "Eigenbau",
    "model": "ESP32 Zeltsteuerung",
    "sw_version": "Hauptprogramm_V18"
  },
  "origin": {
    "name": "Hauptprogramm_V18",
    "sw_version": "18"
  }
})json";
static constexpr char MQTT_DISCOVERY_HUMIDITY_PAYLOAD[] = R"json({
  "name": "Luftfeuchtigkeit",
  "unique_id": "anzuchtzelt_humidity",
  "state_topic": "anzuchtzelt/sensor/humidity",
  "device_class": "humidity",
  "state_class": "measurement",
  "unit_of_measurement": "%",
  "qos": 1,
  "availability": [
    {
      "topic": "anzuchtzelt/status",
      "payload_available": "online",
      "payload_not_available": "offline"
    },
    {
      "topic": "anzuchtzelt/status/bme_ok",
      "payload_available": "true",
      "payload_not_available": "false"
    }
  ],
  "availability_mode": "all",
  "device": {
    "identifiers": ["anzuchtzelt_esp32"],
    "name": "Anzuchtzelt",
    "manufacturer": "Eigenbau",
    "model": "ESP32 Zeltsteuerung",
    "sw_version": "Hauptprogramm_V18"
  },
  "origin": {
    "name": "Hauptprogramm_V18",
    "sw_version": "18"
  }
})json";
static constexpr char MQTT_DISCOVERY_BME_ERROR_PAYLOAD[] = R"json({
  "name": "BME680 Fehler",
  "unique_id": "anzuchtzelt_bme_error",
  "state_topic": "anzuchtzelt/status/bme_ok",
  "device_class": "problem",
  "entity_category": "diagnostic",
  "payload_on": "false",
  "payload_off": "true",
  "qos": 1,
  "availability_topic": "anzuchtzelt/status",
  "payload_available": "online",
  "payload_not_available": "offline",
  "device": {
    "identifiers": ["anzuchtzelt_esp32"],
    "name": "Anzuchtzelt",
    "manufacturer": "Eigenbau",
    "model": "ESP32 Zeltsteuerung",
    "sw_version": "Hauptprogramm_V18"
  },
  "origin": {
    "name": "Hauptprogramm_V18",
    "sw_version": "18"
  }
})json";
static constexpr char MQTT_DISCOVERY_HEARTBEAT_PAYLOAD[] = R"json({
  "name": "Heartbeat",
  "unique_id": "anzuchtzelt_heartbeat",
  "state_topic": "anzuchtzelt/status/heartbeat",
  "entity_category": "diagnostic",
  "icon": "mdi:heart-pulse",
  "qos": 1,
  "availability_topic": "anzuchtzelt/status",
  "payload_available": "online",
  "payload_not_available": "offline",
  "device": {
    "identifiers": ["anzuchtzelt_esp32"],
    "name": "Anzuchtzelt",
    "manufacturer": "Eigenbau",
    "model": "ESP32 Zeltsteuerung",
    "sw_version": "Hauptprogramm_V18"
  },
  "origin": {
    "name": "Hauptprogramm_V18",
    "sw_version": "18"
  }
})json";
static constexpr char MQTT_DISCOVERY_MOTOR_FAULT_PAYLOAD[] = R"json({
  "name": "Motorfehler",
  "unique_id": "anzuchtzelt_motor_fault",
  "state_topic": "anzuchtzelt/status/motor_fault",
  "entity_category": "diagnostic",
  "icon": "mdi:alert-circle-outline",
  "qos": 1,
  "availability_topic": "anzuchtzelt/status",
  "payload_available": "online",
  "payload_not_available": "offline",
  "device": {
    "identifiers": ["anzuchtzelt_esp32"],
    "name": "Anzuchtzelt",
    "manufacturer": "Eigenbau",
    "model": "ESP32 Zeltsteuerung",
    "sw_version": "Hauptprogramm_V18"
  },
  "origin": {
    "name": "Hauptprogramm_V18",
    "sw_version": "18"
  }
})json";
static unsigned long lastMqttConnectAttempt_ms = 0;
static bool mqttConnectAttempted = false;
static bool mqttWasConnected = false;
static unsigned long lastMqttSensorPublish_ms = 0;
static HeartbeatStatus lastPublishedHeartbeatStatus = HeartbeatStatus::Grace;
static bool heartbeatStatusPublished = false;
static MotorFault lastPublishedMotorFault = MOTOR_FAULT_NONE;
static bool motorFaultPublished = false;
static GrowPhase lastPublishedGrowPhase = GrowPhase::Vegetation;
static bool growPhasePublished = false;
static GrowPhase lastPublishedLampModePhase = GrowPhase::Vegetation;
static bool lampModePublished = false;

static const char* heartbeatStatusToPayload(HeartbeatStatus status) {
  switch (status) {
    case HeartbeatStatus::Grace:
      return "grace";
    case HeartbeatStatus::Ok:
      return "ok";
    case HeartbeatStatus::Timeout:
      return "timeout";
  }
  return "unknown";
}

static const char* motorFaultToPayload(MotorFault fault) {
  switch (fault) {
    case MOTOR_FAULT_NONE:
      return "none";
    case MOTOR_FAULT_POTI_INVALID:
      return "poti_invalid";
    case MOTOR_FAULT_TIMEOUT:
      return "timeout";
  }
  return "unknown";
}

static const char* growPhaseToPayload(GrowPhase phase) {
  switch (phase) {
    case GrowPhase::Vegetation:
      return "vegetation";
    case GrowPhase::Flowering:
      return "flowering";
  }
  return "unknown";
}

static const char* lampModeToPayload(GrowPhase phase) {
  switch (phase) {
    case GrowPhase::Vegetation:
      return "18h";
    case GrowPhase::Flowering:
      return "12h";
  }
  return "unknown";
}

static void publishOperationalStatus(unsigned long now, bool force) {
  const HeartbeatStatus heartbeatStatus = getHeartbeatStatus(now);
  const MotorFault motorFault = getMotorFault();

  const bool heartbeatStatusChanged =
      !heartbeatStatusPublished ||
      heartbeatStatus != lastPublishedHeartbeatStatus;
  if (force || heartbeatStatusChanged) {
    const uint16_t packetId = mqttClient.publish(
        MQTT_HEARTBEAT_TOPIC,
        MQTT_OPERATIONAL_STATUS_QOS,
        true,
        heartbeatStatusToPayload(heartbeatStatus));
    if (packetId == 0) {
      Serial.println(
          F("[MQTT] Heartbeat-Status konnte nicht gesendet werden."));
    } else {
      lastPublishedHeartbeatStatus = heartbeatStatus;
      heartbeatStatusPublished = true;
    }
  }

  const bool motorFaultChanged =
      !motorFaultPublished || motorFault != lastPublishedMotorFault;
  if (force || motorFaultChanged) {
    const uint16_t packetId = mqttClient.publish(
        MQTT_MOTOR_FAULT_TOPIC,
        MQTT_OPERATIONAL_STATUS_QOS,
        true,
        motorFaultToPayload(motorFault));
    if (packetId == 0) {
      Serial.println(
          F("[MQTT] Motorfehlerstatus konnte nicht gesendet werden."));
    } else {
      lastPublishedMotorFault = motorFault;
      motorFaultPublished = true;
    }
  }
}

static void publishGrowStatus(bool force) {
  const GrowPhase growPhase = getGrowPhase();

  const bool growPhaseChanged =
      !growPhasePublished || growPhase != lastPublishedGrowPhase;
  if (force || growPhaseChanged) {
    const uint16_t packetId = mqttClient.publish(
        MQTT_GROW_PHASE_TOPIC,
        MQTT_GROW_STATUS_QOS,
        true,
        growPhaseToPayload(growPhase));
    if (packetId == 0) {
      Serial.println(F("[MQTT] GrowPhase konnte nicht gesendet werden."));
    } else {
      lastPublishedGrowPhase = growPhase;
      growPhasePublished = true;
    }
  }

  const bool lampModeChanged =
      !lampModePublished || growPhase != lastPublishedLampModePhase;
  if (force || lampModeChanged) {
    const uint16_t packetId = mqttClient.publish(
        MQTT_LAMP_MODE_TOPIC,
        MQTT_GROW_STATUS_QOS,
        true,
        lampModeToPayload(growPhase));
    if (packetId == 0) {
      Serial.println(F("[MQTT] Lampenmodus konnte nicht gesendet werden."));
    } else {
      lastPublishedLampModePhase = growPhase;
      lampModePublished = true;
    }
  }
}

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

static void publishHomeAssistantDiscovery() {
  if (mqttClient.publish(
          MQTT_DISCOVERY_TEMPERATURE_TOPIC,
          MQTT_DISCOVERY_QOS,
          true,
          MQTT_DISCOVERY_TEMPERATURE_PAYLOAD) == 0) {
    Serial.println(
        F("[MQTT] Temperatur-Discovery konnte nicht gesendet werden."));
  }

  if (mqttClient.publish(
          MQTT_DISCOVERY_HUMIDITY_TOPIC,
          MQTT_DISCOVERY_QOS,
          true,
          MQTT_DISCOVERY_HUMIDITY_PAYLOAD) == 0) {
    Serial.println(
        F("[MQTT] Luftfeuchte-Discovery konnte nicht gesendet werden."));
  }

  if (mqttClient.publish(
          MQTT_DISCOVERY_BME_ERROR_TOPIC,
          MQTT_DISCOVERY_QOS,
          true,
          MQTT_DISCOVERY_BME_ERROR_PAYLOAD) == 0) {
    Serial.println(F("[MQTT] BME-Discovery konnte nicht gesendet werden."));
  }

  if (mqttClient.publish(
          MQTT_DISCOVERY_HEARTBEAT_TOPIC,
          MQTT_DISCOVERY_QOS,
          true,
          MQTT_DISCOVERY_HEARTBEAT_PAYLOAD) == 0) {
    Serial.println(
        F("[MQTT] Heartbeat-Discovery konnte nicht gesendet werden."));
  }

  if (mqttClient.publish(
          MQTT_DISCOVERY_MOTOR_FAULT_TOPIC,
          MQTT_DISCOVERY_QOS,
          true,
          MQTT_DISCOVERY_MOTOR_FAULT_PAYLOAD) == 0) {
    Serial.println(
        F("[MQTT] Motorfehler-Discovery konnte nicht gesendet werden."));
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

  publishHomeAssistantDiscovery();
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
    heartbeatStatusPublished = false;
    motorFaultPublished = false;
    growPhasePublished = false;
    lampModePublished = false;
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
    publishOperationalStatus(now, newConnection);
    publishGrowStatus(newConnection);
    return;
  }

  mqttWasConnected = false;
  heartbeatStatusPublished = false;
  motorFaultPublished = false;
  growPhasePublished = false;
  lampModePublished = false;
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
