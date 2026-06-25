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

Zu pruefen:

- Exakte Serial-Ausgaben und Menuefuehrung am aktuellen Firmwarestand.
- Ob alle Befehle am Geraet wie dokumentiert reagieren.

## LED-Status

Aus `CODING_RULES.md` belegt:

| LED | Bedeutung |
| --- | --- |
| LED0/LED1/LED2 | BME680 Plausibilitaetsstatus |
| LED3 | Motor/Poti/Failsafe |
| LED4 | obere Temperaturzone |
| LED5 | untere Temperaturzone |
| LED6 | Heartbeat-Watchdog |
| LED7 | WLAN/NTP-Status |
| LED8 | Lampenmodus 12h/18h |

Zu pruefen:

- Tatsaechliche LED-Reihenfolge am montierten Aufbau.
- Farbwirkung am Geraet, besonders bei Warn- und Fehlerzustaenden.

## Kalibrierung

Aus `CODING_RULES.md` belegt:

- Kalibrierung nutzt 3 bis 5 Kalibrierpunkte.
- Slots sind 0 %, 25 %, 50 %, 75 %, 100 %.
- Kalibrierwerte werden in NVS gespeichert.
- Vorhandene Kalibrierwerte duerfen nicht ungefragt geloescht werden.

Zu pruefen:

- Aktuelle Kalibrierwerte am Geraet vor Motor- oder Poti-Arbeiten.

## Lampenmodus

Aus `CODING_RULES.md` belegt:

- Lampensteuerung hat einen 12h/18h-Modus.
- WLAN/NTP-Status ist getrennt von der Lampenlogik zu behandeln.

Zu pruefen:

- Konkrete Schalterstellung und reale Lampenzeiten am Aufbau.
