# Bedienung

Status: aus vorhandener Projektdokumentation uebernommen; konkrete Bedienung am Aufbau zu pruefen.

## Normalbetrieb

- Projekt mit PlatformIO bauen: `pio run`
- Upload bei Hardwaretest: `pio run -t upload`
- Seriellen Monitor mit 115200 Baud verwenden: `pio device monitor`
- Status ueber Serial-Ausgaben und WS2812-Status-LEDs beobachten

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

## LED-Status

Aus `CODING_RULES.md` belegt:

| LED | Bedeutung |
| --- | --- |
| LED0/LED1/LED2 | BME680 Plausibilitaetsstatus |
| LED3 | frei / derzeit unbenutzt |
| LED4 | obere Temperaturzone |
| LED5 | untere Temperaturzone |
| LED6 | Heartbeat-Watchdog |
| LED7 | WLAN/NTP-Status |
| LED8 | Lampenmodus 12h/18h |

Aktuelle Farblogik:

- LED3 bleibt frei und wird nicht als Statusanzeige verwendet.
- LED7 rot: WLAN nicht verbunden und NTP nicht synchronisiert.
- LED7 blau: WLAN nicht verbunden, NTP-Zeit war bereits synchronisiert.
- LED7 orange: WLAN verbunden, NTP noch nicht synchronisiert.
- LED7 gruen: WLAN verbunden und NTP synchronisiert.
- LED8 gruen: 12h-Modus.
- LED8 orange: 18h-Modus.

Zu pruefen:

- Tatsaechliche LED-Reihenfolge am montierten Aufbau.
- Farbwirkung am Geraet, besonders bei Warn- und Fehlerzustaenden.

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
