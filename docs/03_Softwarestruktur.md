# Softwarestruktur

Status: aktueller Stand von Hauptprogramm_V18, abgeglichen mit `platformio.ini`, `src/main.cpp` und den Modul-Schnittstellen.

## Build-System

Der aktuelle Build verwendet:

- PlatformIO
- Default-Environment und Board `esp32dev`
- Arduino Framework
- Adafruit BME680 Library
- Adafruit Unified Sensor
- Adafruit BusIO
- `bertmelis/espMqttClient @ 1.7.3`

NeoPixelBus ist keine Abhaengigkeit mehr. Der fruehere lokale SPI-Shim wurde ebenfalls entfernt.

## Grundprinzip

- Der Ablauf bleibt non-blocking.
- `loop()` bleibt schlank.
- Zyklische Aufgaben laufen als Tasks oder Handler.
- Zeitsteuerung erfolgt ueber `millis()`.
- Sicherheitslogik fuer BME680, Heartbeat-Watchdog, Motor-Failsafe und Kalibrierung bleibt lokal auf dem ESP32 erhalten.
- Fachmodule stellen Statuswerte ueber oeffentliche, nur lesende Getter bereit.
- `MqttStatus` liest diese Schnittstellen und veroeffentlicht die Werte, ohne Fachlogik zu steuern oder interne Variablen zu veraendern.
- Motor-, Lampen-, Sensor- und Watchdog-Logik funktionieren unabhaengig davon, ob Home Assistant geoeffnet oder erreichbar ist.

Der aktuelle MQTT-Stand ist nur lesend. Es gibt keine `command_topic`s, keine steuerbaren MQTT-Switches oder MQTT-Number-Entitaeten und keine Subscriptions fuer Steuerbefehle.

## Aktuelle Projektmodule

- `lib/BmeSensor`: BME680-Messung, Cachewerte und Recovery
- `lib/Calibration`: Kalibrierkennlinie und NVS-Verwaltung
- `lib/Config`: Konfiguration und Pinbelegung
- `lib/HeartbeatWatchdog`: Heartbeat-Ueberwachung und Reset-Failsafe
- `lib/LampControl`: Grow-Phase, Lampenmodus und Relaissteuerung
- `lib/MotorControl`: Stellmotor, Zielwert und Motor-Failsafe
- `lib/MqttStatus`: MQTT-Verbindung, Status-Publishes und Home-Assistant-Discovery
- `lib/PotiFeedback`: Potentiometer-Erfassung und Plausibilitaet
- `lib/SerialCommands`: serielle Diagnose- und Kalibrierbefehle
- `lib/WifiTime`: WLAN-Verbindung und NTP-Synchronisierung
- `src/main.cpp`: Initialisierung und Aufrufreihenfolge der Tasks

Das fruehere Modul `lib/LedStatus` und die WS2812-Statusanzeige wurden vollstaendig entfernt. GPIO16 ist derzeit frei und nicht reserviert. Die zentrale Statusanzeige erfolgt ueber MQTT in Home Assistant.

## Setup-Reihenfolge

Die Reihenfolge entspricht `src/main.cpp`:

1. `initGeneral()`
2. `initRelay()`
3. `initPoti()`
4. `initMotorControl()`
5. `initHeartbeat()`
6. `bmeInit()`
7. `bmeInitialCheck()`
8. `openNVS()`
9. `calInitLoad()`
10. `initHardwareLampensteuerung()`
11. `connectWiFi()`
12. `initMqttStatus()`
13. `checkLampState()`

Danach wird der Abschluss des Setups seriell gemeldet.

## Loop-Reihenfolge

Zu Beginn jeder Runde wird `now` einmal mit `millis()` ermittelt. Danach werden die Tasks und Handler in dieser Reihenfolge aufgerufen:

1. `readPotiTask(now)`
2. `bme680Task(now)`
3. `heartbeatTask(now)`
4. `motorControlTask(now)`
5. `handleWiFi(now)`
6. `handleNTP(now)`
7. `handleLamp(now)`
8. `mqttStatusTask(now)`
9. `serialCommandTask(now)`

## MqttStatus und oeffentliche Statusschnittstellen

`MqttStatus` verwendet die folgenden oeffentlichen Lesezugriffe:

- `BmeSensor`: `tryGetBmeTemperatureC()`, `tryGetBmeHumidityPct()`, `hasBmeDisplayError()`
- `Calibration`: `getCalibrationPointCount()`, `isCalibrationModeActive()`
- `HeartbeatWatchdog`: `getHeartbeatStatus(now)`
- `LampControl`: `getGrowPhase()`, `isLampRelayOn()`
- `MotorControl`: `getMotorFault()`, `getMotorTargetPct()`, `tryGetMotorPositionPct()`, `isPotiFeedbackValid()`
- `WifiTime`: `isWifiConnected()`, `isTimeSynced()`

Der Lampenmodus `18h` beziehungsweise `12h` wird in `MqttStatus` aus der Grow-Phase abgeleitet. Dafuer gibt es keinen separaten Lampenmodus-Getter. Die Poti-Rueckmeldung wird ueber die oeffentliche Schnittstelle von `MotorControl` gelesen; `MqttStatus` inkludiert `PotiFeedback` nicht direkt.

`MqttStatus` stellt mit `isMqttConnected()` selbst einen oeffentlichen Verbindungsstatus bereit. Bei einer Verbindung zum Broker veroeffentlicht das Modul retained Availability, die Home-Assistant-MQTT-Discovery und die aktuellen Statuswerte. Danach werden Zustandswerte anhand ihrer Aenderungen und Messwerte in den vorgesehenen Intervallen erneut publiziert. Nach einem Reconnect werden die Zustaende erneut bereitgestellt.

Home Assistant legt ueber die Discovery-Nachrichten 16 Statusentitaeten an. Der zusammengefasste Systemstatus ergaenzt die Detailentitaeten; die genaue Fehlerursache bleibt in den jeweiligen Detailwerten sichtbar.

## Weitere Modulabhaengigkeiten

- `BmeSensor` setzt Motorziele ueber `setMotorTargetPct()` und nutzt Motorstatuswerte fuer Failsafe-Bewegungen.
- `HeartbeatWatchdog` haengt von `BmeSensor` ab, weil Heartbeat-TX nur bei `bmeHealthy(now)` erfolgt.
- `WifiTime` ruft `checkLampState()` aus `LampControl` auf, wenn NTP synchronisiert wurde.
- `SerialCommands` liest Status aus BME, Poti, Heartbeat, Motor und Kalibrierung.

## Safety-Fluss

- Die Poti-Plausibilitaet wird in `PotiFeedback` ermittelt.
- `MotorControl` stoppt bei ungueltiger Poti-Rueckmeldung und setzt `MOTOR_FAULT_POTI_INVALID`.
- `MotorControl` setzt bei Move-Timeout `MOTOR_FAULT_TIMEOUT` und verriegelt neue Bewegungen bis Neustart oder internem `clearMotorFault()`.
- Motorfehler werden als MQTT-Detailstatus veroeffentlicht; der zusammengefasste Systemstatus kann zusaetzlich `fault` melden.
- `BmeSensor` setzt bei Sensorfehlern einen sicheren Motorzielwert und versucht die non-blocking Reinitialisierung.
- `HeartbeatWatchdog` sendet nur bei gesundem BME und loest bei RX-Timeout einen C3-Reset-Relaispuls aus.
- `Calibration` laedt Kalibrierwerte aus NVS und nutzt bei ungueltigen Daten einen Fallback.

## Verbindliche Regeln

Technische Details und Aenderungsregeln stehen in `CODING_RULES.md`. Diese Datei beschreibt die aktuelle Struktur in lesbarer Form.
