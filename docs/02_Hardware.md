# Hardware

Status: aus vorhandener Projektdokumentation und Projektstruktur zusammengefasst; reale Verkabelung vor Aenderungen zu pruefen.

## Controller und Build-Ziel

Belegt durch `platformio.ini`:

- Board: `esp32dev`
- Framework: Arduino
- Monitor-Speed: 115200

Zu pruefen:

- Exakte ESP32-Boardvariante am realen Aufbau.

## Sensorik

Aus `CODING_RULES.md` belegt:

- BME680 ueber I2C
- Auto-Detect fuer Adressen 0x76/0x77
- Plausibilitaetspruefung fuer Temperatur und Feuchte
- Fehlerzaehler und Reinitialisierung bleiben erhalten
- Keine manuelle I2C-Bus-Recovery per SDA/SCL-Freitakten einfuehren

Zu pruefen:

- Reale I2C-Verkabelung und Sensoradresse am Aufbau.

## Motor und Positionsrueckmeldung

Aus `CODING_RULES.md` belegt:

- Motorsteuerung ueber L298N
- PWM ueber LEDC
- Poti als Positionsrueckmeldung
- Motor-Failsafe bei ungueltigem Poti
- Move-Timeout fuehrt zum Motorstopp
- BME-Fehler fuehrt zu definiertem sicherem Verhalten

Zu pruefen:

- Reale Motorverdrahtung.
- Mechanische Endlagen.
- Aktuelle Poti-Kalibrierung.

## Relais

Aus `CODING_RULES.md` belegt:

- `C3_WD_RELAIS_PIN`: Watchdog-Reset-Relais
- `relayPin`: Lampen-Relais
- Relaispulse duerfen nicht mit `delay()` umgesetzt werden

Zu pruefen:

- Aktive Relaislogik am Aufbau.
- Reale Zuordnung und Verdrahtung der Relais.

## Status-LEDs

Aus `CODING_RULES.md` belegt:

- WS2812-LEDs werden ueber `NeoPixelBus` gesteuert.
- LED-Bedeutungen stehen in `docs/01_Bedienung.md`.
- Direkte LED-Zugriffe ausserhalb der LED-Hilfsfunktionen sind zu vermeiden.

Zu pruefen:

- Anzahl und physische Reihenfolge der LEDs am Aufbau.

## Pinbelegung

Die Pinbelegung ist aus `lib/Config/Pins.h` belegt. Keine neuen Pinwerte erfinden.

| Funktion | Pin |
| --- | --- |
| I2C SDA | 21 |
| I2C SCL | 22 |
| WS2812 Datenpin | 16 |
| Motor L298N IN1 | 26 |
| Motor L298N IN2 | 27 |
| Motor L298N ENA/PWM | 25 |
| Poti ADC | 34 |
| Heartbeat Eingang | 32 |
| Heartbeat Ausgang | 33 |
| C3-Reset-Relais | 23 |
| Lampen-Relais | 19 |
| Modus-Schalter | 18 |

Belegt durch vorhandene Projektstruktur:

- Pin-/Konfigurationsbereich ist als `lib/Config/Pins.h` vorhanden.

Zu pruefen:

- Vor jeder Hardwareaenderung `lib/Config/Pins.h` und die reale Verkabelung abgleichen.
- Pins nicht aendern, ausser es wird ausdruecklich verlangt.
- `docs/pinout/` ist im aktuell gelesenen Repo nicht vorhanden.

## Weitere Hardware-Unterlagen

Aktueller Repo-Stand:

- `docs/datasheets/` ist im aktuell gelesenen Repo nicht vorhanden.
- `docs/hardware/` ist im aktuell gelesenen Repo nicht vorhanden.
- `docs/visio/` ist im aktuell gelesenen Repo nicht vorhanden.
- `docs/archiv/` ist im aktuell gelesenen Repo nicht vorhanden.
- Alte oder unsichere Hinweise sind in `docs/99_Alt_und_Unsicher.md` markiert.

Zu pruefen:

- Ob externe Hardware-Unterlagen ausserhalb dieses Repos existieren.
- Ob solche Unterlagen direkt zum aktuellen Aufbau gehoeren.
