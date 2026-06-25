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

Die konkrete Pinbelegung wird hier nicht dupliziert.

Belegt durch vorhandene Projektstruktur:

- Pin-/Konfigurationsbereich ist als `lib/Config/Pins.h` vorhanden.
- Pinout-Unterlagen liegen in `docs/pinout/`.

Zu pruefen:

- Vor jeder Hardwareaenderung `lib/Config/Pins.h` und die reale Verkabelung abgleichen.
- Pins nicht aendern, ausser es wird ausdruecklich verlangt.

## Weitere Hardware-Unterlagen

- Datenblaetter und Manuals liegen in `docs/datasheets/`.
- Weitere Hardware-/Komponentenunterlagen liegen in `docs/hardware/`.
- Visio-/Schaltplanmaterial liegt in `docs/visio/`.
- Alte oder unsichere Unterlagen liegen in `docs/archiv/` oder sind in `docs/99_Alt_und_Unsicher.md` markiert.

Zu pruefen:

- Ob die Unterlagen in `docs/hardware/` direkt zum aktuellen Aufbau gehoeren.
- Ob die Dateien in `docs/datasheets/` exakt den verbauten Komponenten entsprechen.
