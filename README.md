# Anzuchtzelt Hauptprogramm V17

ESP32-PlatformIO-Projekt fuer das Anzuchtzelt.

Die verbindlichen technischen Regeln stehen in `CODING_RULES.md`.
Agenten-Anweisungen stehen in `AGENTS.md`.

## Kurzübersicht

`src/main.cpp` orchestriert die zyklischen Tasks und Handler. Die eigentliche Logik liegt in eigenen Modulen unter `lib/`.

Aktuelle Module:

- `lib/BmeSensor`: BME680-Messung, Plausibilität, Reinitialisierung und Failsafe-Zielwert
- `lib/Calibration`: Poti-Kalibrierung und NVS-Speicherung
- `lib/Config`: Pins und zentrale Konstanten
- `lib/HeartbeatWatchdog`: Heartbeat-Senden, RX-Überwachung und C3-Reset-Relaispuls
- `lib/LampControl`: Lampenrelais und 12h/18h-Modus
- `lib/LedStatus`: WS2812-Statusanzeige; LED3 bleibt frei
- `lib/MotorControl`: Motorregelung, Deadband-Startlogik, MotorFault und Timeout-Sperre
- `lib/PotiFeedback`: Poti-Lesen, Filterung und Plausibilitätsstatus
- `lib/SerialCommands`: Statusausgaben und Kalibrierbefehle
- `lib/WifiTime`: WLAN, NTP, Sommerzeit und Zeitstatus

Die lokale WLAN-Secret-Datei ist `include/wifi_secrets.h`. Als Vorlage ist `include/wifi_secrets.example.h` im Repository.

Wichtige Safety-Funktionen:

- BME680-Autodetect 0x76/0x77, Fehlerzaehler und nicht-blockierende Reinitialisierung
- Motorstopp bei ungueltigem Poti und MotorFault-Status fuer Poti/Timeout
- Timeout-Verriegelung der Motorsteuerung bis Neustart oder internem `clearMotorFault()`
- Heartbeat-Watchdog mit TX nur bei gesundem BME, RX-Timeout und C3-Reset-Relaispuls
- Poti-Kalibrierung mit NVS-Speicherung und Fallback

Hinweis: Die Detaildokumentation kann teilweise noch Platzhalter oder Abschnitte mit Pruefstatus enthalten.

## Dokumentation

- Bedienung: `docs/01_Bedienung.md`
- Hardware: `docs/02_Hardware.md`
- Softwarestruktur: `docs/03_Softwarestruktur.md`
- Fehlersuche: `docs/04_Fehlersuche.md`
- Entwicklung: `docs/05_Entwicklung.md`
- Alte oder unsichere Hinweise: `docs/99_Alt_und_Unsicher.md`

## Kurzbefehle

```powershell
pio run
pio run -t upload
pio device monitor
```
