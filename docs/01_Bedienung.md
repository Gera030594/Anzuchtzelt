# Bedienung

Die aktuelle Firmware stellt ihre Messwerte und Zustände über MQTT in Home Assistant bereit. Home Assistant dient als zentrale Statusanzeige, die Steuerung auf dem ESP32 arbeitet jedoch unabhängig davon weiter.

## Normalbetrieb

- Projekt mit PlatformIO bauen: `pio run`
- Upload bei Hardwaretest: `pio run -t upload`
- Seriellen Monitor mit 115200 Baud verwenden: `pio device monitor`
- Status über Home Assistant beobachten und den seriellen Monitor für zusätzliche Diagnose verwenden

## Serielle Befehle

Aus `CODING_RULES.md` belegt:

| Befehl | Bedeutung |
| --- | --- |
| `r` | Statusdump |
| `K` | Kalibriermodus starten |
| `1..5` | Kalibrierpunkt erfassen |
| `L` | Punkte anzeigen |
| `S` | Speichern in NVS |
| `V` | Reload aus NVS |
| `Q` | Verlassen |
| `D` | NVS-Diagnose |

Motor-Timeouts werden im seriellen Monitor in diesem Format gemeldet:

```text
[MOTOR] ERROR: MOVE_TIMEOUT cur=... target=... dir=... elapsed=...
```

Zu pruefen:

- Exakte Serial-Ausgaben und Menuefuehrung am aktuellen Firmwarestand.
- Ob alle Befehle am Geraet wie dokumentiert reagieren.
- Es gibt aktuell keinen Serial-Befehl zum Loeschen eines MotorFault.

## Statusprüfung über Home Assistant und MQTT

Das Projekt besitzt keine WS2812-Statusanzeige mehr. Zustände und Messwerte werden ausschließlich lesend über MQTT an Home Assistant übertragen.

Sinnvolle Prüfreihenfolge:

1. `ESP32-Verbindung` prüfen. Diese Entität zeigt die MQTT-Erreichbarkeit über Online-Publish und Last Will an.
2. `Systemstatus` prüfen.
3. Bei einem auffälligen Systemstatus die betroffene Detailentität prüfen, insbesondere `BME680-Fehler`, `Heartbeat`, `Motorfehler`, `Poti-Rückmeldung` und `Zeitsynchronisation`.
4. Den seriellen Monitor mit 115200 Baud für zusätzliche Diagnose verwenden.

Mögliche Werte des zusammengefassten Systemstatus:

| Payload | Bedeutung |
| --- | --- |
| `ok` | Der zusammengefasste Status ist in Ordnung. |
| `starting` | Das System befindet sich noch in der Startphase. |
| `calibration` | Der Kalibriermodus ist aktiv. |
| `fault` | Mindestens ein berücksichtigter Fehlerzustand liegt vor. |
| `unknown` | Der Zustand kann derzeit nicht eindeutig bestimmt werden. |

Der Systemstatus ist nur eine Zusammenfassung. Die Detailentitäten liefern die genaue Ursache. Die vollständige Liste der 16 automatisch angelegten Entitäten und ihrer State-Topics steht im `README.md`.

Wenn `ESP32-Verbindung` als offline angezeigt wird, zuerst die Erreichbarkeit von WLAN und MQTT-Broker prüfen und anschließend den seriellen Monitor verwenden. Der Verbindungsstatus enthält keine Aussage über Internetzugang oder WLAN-Signalstärke.

Home Assistant kann im aktuellen Stand keine Fehler quittieren oder zurücksetzen und keine Motor-, Lampen-, Kalibrier- oder sonstigen Steuerbefehle an den ESP32 senden. Es gibt keine steuerbaren MQTT-Entitäten oder Steuer-Subscriptions.

## Motorbedienung

- Ungueltiges Poti stoppt den Motor und setzt `MOTOR_FAULT_POTI_INVALID`.
- Move-Timeout stoppt den Motor und setzt `MOTOR_FAULT_TIMEOUT`.
- Nach `MOTOR_FAULT_TIMEOUT` startet die Motorsteuerung keine neue Bewegung mehr.
- Die Sperre bleibt bis Neustart oder internem `clearMotorFault()` aktiv.
- Ein Bedienbefehl zum Loeschen per Serial ist aktuell nicht vorhanden.

## Kalibrierung

Aus aktuellem Code und `CODING_RULES.md` belegt:

- Kalibrierung nutzt technisch mindestens 2 gueltige Punkte.
- Slots sind 0 %, 25 %, 50 %, 75 %, 100 %.
- Kalibrierwerte werden in NVS gespeichert.
- Vorhandene Kalibrierwerte duerfen nicht ungefragt geloescht werden.

Zu pruefen:

- Aktuelle Kalibrierwerte am Geraet vor Motor- oder Poti-Arbeiten.

## Lampenmodus

Aus `CODING_RULES.md` belegt:

- Lampensteuerung hat einen 12h/18h-Modus.
- 12h-Modus: 08:00 bis 20:00.
- 18h-Modus: 04:00 bis 22:00.
- WLAN/NTP-Status ist getrennt von der Lampenlogik zu behandeln.

Zu pruefen:

- Konkrete Schalterstellung am Aufbau.
