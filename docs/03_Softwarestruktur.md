# Softwarestruktur

Status: aus vorhandener Projektdokumentation, `platformio.ini` und Projektstruktur zusammengefasst.

## Build-System

Belegt durch `platformio.ini`:

- PlatformIO
- Default-Environment: `esp32dev`
- Board: `esp32dev`
- Framework: Arduino
- Libraries werden ueber `lib_deps` verwaltet

Aktueller `platformio.ini`-Stand:

- Adafruit BME680 Library
- Adafruit Unified Sensor
- Adafruit BusIO
- NeoPixelBus

Offener Punkt / zu pruefen:

- `ArduinoJson`, `OneWire` und `DHT sensor library` stehen im aktuell gelesenen `platformio.ini` nicht in `lib_deps`.
- Falls diese Libraries in einem anderen Arbeitsstand auftauchen, erst ihre Nutzung im Code pruefen und nicht automatisch entfernen.

## Grundprinzip

Aus `CODING_RULES.md` belegt:

- Der Ablauf bleibt non-blocking.
- `loop()` bleibt schlank.
- Zyklische Aufgaben laufen als Tasks oder Handler.
- Zeitsteuerung erfolgt ueber `millis()`.
- Sicherheitslogik fuer BME680, Heartbeat-Watchdog, Motor-Failsafe und Kalibrierung bleibt erhalten.

## Vorhandene Modulordner

Aus der vorhandenen Projektstruktur belegt:

- `lib/BmeSensor`
- `lib/Calibration`
- `lib/Config`
- `lib/HeartbeatWatchdog`
- `lib/LampControl`
- `lib/LedStatus`
- `lib/MotorControl`
- `lib/PotiFeedback`
- `lib/SerialCommands`
- `lib/WifiTime`

Zu pruefen:

- Ob alle in `CODING_RULES.md` beschriebenen Zielmodule vollstaendig dem aktuellen Code entsprechen.
- Ob alte Hinweise zu `main.cpp` noch den aktuellen Auslagerungsstand abbilden.

## Zentrale Aufgabenbereiche

Aus `CODING_RULES.md` belegt:

- Poti-Task
- BME680-Task
- Heartbeat-Task
- Motor-Task
- LED-Update-Task
- Serial-Command-Task
- WLAN-Handler
- NTP-Handler
- Lampen-Handler

## Setup-Reihenfolge

Aus `src/main.cpp` belegt:

1. `initGeneral()`
2. `initRelay()`
3. `initPoti()`
4. `initLEDs()`
5. `initMotorControl()`
6. `initHeartbeat()`
7. `bmeInit()`
8. `bmeInitialCheck()`
9. `openNVS()`
10. `calInitLoad()`
11. `initHardwareLampensteuerung()`
12. `updateModeLed()`
13. `connectWiFi()`
14. `checkLampState()`
15. `updateZoneLEDs(T)`
16. `updateStatusLed()`

## Loop-Reihenfolge

Aus `src/main.cpp` belegt:

1. `readPotiTask(now)`
2. `bme680Task(now)`
3. `heartbeatTask(now)`
4. `motorControlTask(now)`
5. `handleWiFi(now)`
6. `handleNTP(now)`
7. `handleLamp(now)`
8. `updateStatusLed()`
9. `updateModeLed()`
10. `ledUpdateTask()`
11. `serialCommandTask(now)`

## Modulabhaengigkeiten

Aus den Includes und Funktionsaufrufen im aktuellen Code belegt:

- `BmeSensor` setzt Motorziele ueber `setMotorTargetPct()` und nutzt Motorstatuswerte fuer Failsafe-Bewegungen.
- `HeartbeatWatchdog` haengt von `BmeSensor` ab, weil Heartbeat-TX nur bei `bmeHealthy(now)` erfolgt.
- `WifiTime` ruft `checkLampState()` aus `LampControl` auf, wenn NTP synchronisiert wurde.
- `SerialCommands` liest Status aus BME, Poti, Heartbeat, Motor und Kalibrierung.

## Safety-Fluss

- Poti-Plausibilitaet wird in `PotiFeedback` ueber `potiValid` bereitgestellt.
- `MotorControl` stoppt bei `potiValid == false` und setzt `MOTOR_FAULT_POTI_INVALID`.
- `MotorControl` setzt bei Move-Timeout `MOTOR_FAULT_TIMEOUT` und verriegelt neue Bewegungen bis Neustart oder internem `clearMotorFault()`.
- MotorFault-Zustaende bleiben Fachstatus und werden nicht auf LED3 angezeigt.
- `BmeSensor` setzt bei Sensorfehlern einen sicheren Motorzielwert und versucht die nicht-blockierende Reinitialisierung.
- `HeartbeatWatchdog` sendet nur bei gesundem BME und loest bei RX-Timeout einen C3-Reset-Relaispuls aus.
- `Calibration` laedt Kalibrierwerte aus NVS und nutzt bei ungueltigen Daten einen Fallback.

## Verbindliche Regeln

Technische Details und Aenderungsregeln stehen in `CODING_RULES.md`.
Diese Datei ist nur die lesbare Uebersicht.
