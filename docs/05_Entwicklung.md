# Entwicklung

Status: aktueller Entwicklungs- und Build-Ablauf fuer Hauptprogramm_V18.

## Werkzeuge

- VS Code
- PlatformIO Extension
- Git
- GitHub-Konto
- Codex
- Arduino IDE als Reserve
- USB-Treiber fuer ESP32

## Build-Umgebung

Der aktuelle Stand in `platformio.ini` verwendet:

- Default-Environment `esp32dev`
- Board `esp32dev` beziehungsweise ESP32 Dev Module
- Arduino Framework
- PlatformIO mit der im Projekt festgelegten ESP32-Plattform
- Monitor-Geschwindigkeit 115200 Baud

Aktuelle `lib_deps`:

- `adafruit/Adafruit BME680 Library`
- `adafruit/Adafruit Unified Sensor`
- `adafruit/Adafruit BusIO`
- `bertmelis/espMqttClient @ 1.7.3`

NeoPixelBus ist nicht mehr in `lib_deps` eingetragen. Das entfernte LedStatus-Modul und der fruehere lokale SPI-Shim sind nicht mehr Teil des Builds; LED-spezifische NeoPixelBus- oder RMT-Buildmassnahmen sind daher nicht erforderlich. ESP32-LEDC bleibt davon unabhaengig fuer die Motor-PWM in Verwendung.

## Normaler Ablauf

```powershell
git status
git diff
pio run
git add <konkrete Dateien>
git commit -m "type: kurze beschreibung"
git push
```

`pio run` ist die verbindliche Build-Pruefung nach Codeaenderungen. Dateien sollen gezielt hinzugefuegt werden; `git add .` ist fuer den normalen Ablauf nicht vorgesehen.

## Nach Codeaenderungen

Aus `AGENTS.md` und `CODING_RULES.md` folgt:

- `pio run` ausfuehren oder melden, falls der Build nicht moeglich ist.
- Bestehende Architektur und Task-Struktur erhalten.
- Non-blocking bleiben.
- Keine Pins aendern, ausser es wurde ausdruecklich verlangt.
- Keine Secrets oder Secret-Beispieldateien aendern, ausser es wurde ausdruecklich verlangt.
- BME680-Recovery, Heartbeat-Watchdog, Motor-Failsafe und Kalibrierung/NVS nicht entfernen.

## Secrets

Lokale Zugangsdaten bleiben ausschliesslich in den dafuer vorgesehenen lokalen Secret-Dateien, insbesondere:

- `include/wifi_secrets.h`
- `include/mqtt_secrets.h`

Diese lokalen Dateien duerfen nicht committed werden. Versionierte Beispieldateien enthalten nur Platzhalter. Vor jedem Commit ist zu pruefen, dass keine Zugangsdaten, Passwoerter oder andere lokale Secret-Inhalte im Diff enthalten sind.

## Commit-Hinweise

- `feat`: neue Funktion
- `fix`: Fehlerbehebung
- `refactor`: interne Umstrukturierung
- `docs`: Dokumentation
- `chore`: Projektpflege
- `test`: Tests

Vor `git push` ist sicherzustellen, dass keine Secrets, `.pio`-Dateien oder unnoetigen generierten Dateien enthalten sind.

## Dokumentationsaenderungen

- README kurz halten.
- Doppelte Inhalte vermeiden.
- Aktive Dokumentation gegen den aktuellen Code und `platformio.ini` pruefen.
- Alte oder unsichere Hinweise eindeutig in `docs/99_Alt_und_Unsicher.md` als historisch kennzeichnen.
- Historische Hinweise nicht als aktuelle Build-, Upload- oder Bedienungsanleitung verwenden.
