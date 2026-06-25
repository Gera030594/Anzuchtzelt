# Softwarestruktur

Status: aus vorhandener Projektdokumentation, `platformio.ini` und Projektstruktur zusammengefasst.

## Build-System

Belegt durch `platformio.ini`:

- PlatformIO
- Default-Environment: `esp32dev`
- Board: `esp32dev`
- Framework: Arduino
- Libraries werden ueber `lib_deps` verwaltet

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

## Verbindliche Regeln

Technische Details und Aenderungsregeln stehen in `CODING_RULES.md`.
Diese Datei ist nur die lesbare Uebersicht.
