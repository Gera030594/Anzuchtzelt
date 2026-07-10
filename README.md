# Anzuchtzelt Hauptprogramm_V18

ESP32-PlatformIO-Projekt fuer das Anzuchtzelt.

Die verbindlichen technischen Regeln stehen in `CODING_RULES.md`.
Agenten-Anweisungen stehen in `AGENTS.md`.

## Kurzübersicht

`src/main.cpp` orchestriert die Initialisierung sowie die zyklischen Tasks und Handler. Die Fachlogik liegt in eigenen Modulen unter `lib/`.

Aktuelle Module:

- `lib/BmeSensor`: BME680-Messung, Plausibilität, Reinitialisierung und Failsafe-Zielwert
- `lib/Calibration`: Poti-Kalibrierung und NVS-Speicherung
- `lib/Config`: zentrale Konstanten und Pinbelegung in `Config.h` und `Pins.h`
- `lib/HeartbeatWatchdog`: Heartbeat-Senden, RX-Überwachung und C3-Reset-Relaispuls
- `lib/LampControl`: Lampenrelais und 12h/18h-Modus
- `lib/MotorControl`: Motorregelung, Deadband-Startlogik, MotorFault und Timeout-Sperre
- `lib/MqttStatus`: lokale MQTT-Verbindung, Home-Assistant-Discovery und Veröffentlichung der Mess- und Statuswerte
- `lib/PotiFeedback`: Poti-Lesen, Filterung und Plausibilitätsstatus
- `lib/SerialCommands`: Statusausgaben und Kalibrierbefehle
- `lib/WifiTime`: WLAN, NTP, Sommerzeit und Zeitstatus
- `src/main.cpp`: Initialisierung und non-blocking Aufrufreihenfolge der Module

Lokale Zugangsdaten stehen ausschließlich in den ignorierten Dateien `include/wifi_secrets.h` und `include/mqtt_secrets.h`. Als versionierte Vorlagen liegen `include/wifi_secrets.example.h` und `include/mqtt_secrets.example.h` im Repository.

Wichtige Safety-Funktionen:

- BME680-Autodetect 0x76/0x77, Fehlerzaehler und nicht-blockierende Reinitialisierung
- Motorstopp bei ungueltigem Poti und MotorFault-Status fuer Poti/Timeout
- Timeout-Verriegelung der Motorsteuerung bis Neustart oder internem `clearMotorFault()`
- Heartbeat-Watchdog mit TX nur bei gesundem BME, RX-Timeout und C3-Reset-Relaispuls
- Poti-Kalibrierung mit NVS-Speicherung und Fallback

## Statusanzeige über MQTT und Home Assistant

Das Projekt verwendet keine WS2812-Status-LEDs mehr. Das frühere Modul `lib/LedStatus`, NeoPixelBus und der lokale SPI-Shim wurden entfernt. GPIO16 ist aktuell frei und nicht reserviert.

Messwerte und Zustände werden an einen lokalen MQTT-Broker übertragen und über Home Assistant dargestellt. Home Assistant ist damit die zentrale Statusanzeige. Die lokale Motor-, Lampen-, Sensor- und Watchdog-Logik arbeitet unabhängig davon weiter, ob Home Assistant geöffnet oder erreichbar ist.

`lib/MqttStatus` bietet:

- retained Availability über `anzuchtzelt/status`
- Home-Assistant-MQTT-Discovery
- erneute Veröffentlichung der Zustände nach einer MQTT-Wiederverbindung
- Änderungserkennung für Statuswerte
- ausschließlich lesende Statusübertragung

Der aktuelle MQTT-Stand besitzt keine `command_topic`s, keine MQTT-Switches, keine MQTT-Number-Entitäten und keine Subscriptions für Steuerbefehle. Home Assistant steuert den ESP32 daher nicht.

### Automatisch angelegte Home-Assistant-Entitäten

| Nr. | Entität | Typ | State-Topic |
| ---: | --- | --- | --- |
| 1 | Temperatur | Sensor | `anzuchtzelt/sensor/temperature` |
| 2 | Luftfeuchtigkeit | Sensor | `anzuchtzelt/sensor/humidity` |
| 3 | BME680-Fehler | Binary Sensor, aus `bme_ok` invertiert | `anzuchtzelt/status/bme_ok` |
| 4 | Heartbeat | Sensor | `anzuchtzelt/status/heartbeat` |
| 5 | Motorfehler | Sensor | `anzuchtzelt/status/motor_fault` |
| 6 | Grow-Phase | Sensor | `anzuchtzelt/status/grow_phase` |
| 7 | Lampenmodus | Sensor | `anzuchtzelt/status/lamp_mode` |
| 8 | Lampenrelais | Binary Sensor | `anzuchtzelt/status/lamp_relay` |
| 9 | Lüfter-Sollwert | Sensor | `anzuchtzelt/sensor/fan_target_pct` |
| 10 | Stellposition | Sensor | `anzuchtzelt/sensor/motor_position_pct` |
| 11 | Poti-Rückmeldung | Binary Sensor | `anzuchtzelt/status/poti_feedback_valid` |
| 12 | ESP32-Verbindung | Binary Sensor | `anzuchtzelt/status` |
| 13 | Zeitsynchronisation | Binary Sensor | `anzuchtzelt/status/time_synced` |
| 14 | Kalibrierpunkte | Sensor | `anzuchtzelt/sensor/calibration_points` |
| 15 | Kalibriermodus | Binary Sensor | `anzuchtzelt/status/calibration_mode_active` |
| 16 | Systemstatus | Sensor | `anzuchtzelt/status/system_state` |

Der zusammengefasste Systemstatus kann folgende Payloads liefern:

- `ok`
- `starting`
- `calibration`
- `fault`
- `unknown`

Der Systemstatus ist eine Zusammenfassung. Die Detailentitäten liefern weiterhin die genaue Ursache eines Zustands.

Fachliche Bedeutung ausgewählter Werte:

- Der Lüfter-Sollwert ist der berechnete Zielwert in Prozent und keine gemessene Lüfterdrehzahl.
- Die Stellposition ist die kalibrierte Potentiometer-/Stellmotorposition. Sie ist weder eine U/min-Messung noch ein gemessener Luftstrom oder eine direkte Lüfterrückmeldung.
- Das Lampenrelais zeigt den logischen, vom Programm gesetzten Relaiszustand. Es ist keine Strommessung und bestätigt nicht, dass die Lampe tatsächlich leuchtet.
- Die ESP32-Verbindung beschreibt die MQTT-Erreichbarkeit über Online-Publish und Last Will. Sie trifft keine Aussage über Internetzugang oder WLAN-Signalstärke.
- Die Kalibrierpunkte geben die Anzahl der aktuell im RAM verwendeten LUT-Punkte an. Zwischen einer Default- und einer aus NVS geladenen 2-Punkt-Kennlinie wird nicht unterschieden.

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
