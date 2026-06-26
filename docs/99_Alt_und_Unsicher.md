# Alte und unsichere Informationen

Status: Archiv. Inhalte hier nicht als aktuelle Anleitung verwenden, ohne sie zu pruefen.

## Alte README-Hinweise

Aus der bisherigen `README.md` uebernommen:

```text
Anzuchtzelt
ESP32-Programme fuer das Anzuchtzelt.
Hauptprogramm_V16
Hauptsteuerung fuer das grosse Zelt.
Watchdog_ESP32
Separater Watchdog-Controller zur Ueberwachung.
```

Bewertung:

- `Hauptprogramm_V16` ist zu pruefen, da das aktuelle Projektverzeichnis `Hauptprogramm_V17` heisst.
- `Watchdog_ESP32` ist zu pruefen, da diese Doku-Struktur nur das aktuelle Projekt beschreibt.

Alter Pfadhinweis aus der bisherigen `README.md`:

```text
C:\Users\hraki\OneDrive\Anzucht\Grosses Zelt\ESP32_Programme\Hauptprogramme
```

Bewertung:

- Zu pruefen, da der aktuelle Arbeitsordner `C:\Users\hraki\OneDrive\Anzucht\Hauptprogramme\Hauptprogramm_V17` ist.

## Alte Workflow-Hinweise

Aus der archivierten Datei `docs/Normaler_Arbeitsablauf.md` uebernommen:

```text
Dateien aendern
git status
git diff
git add .
git commit -m "Beschreibung der Aenderung"
git push
```

Bewertung:

- Durch `docs/05_Entwicklung.md` ersetzt.
- `pio run` fehlte und ist nach Codeaenderungen zu pruefen beziehungsweise auszufuehren.
- `git add .` ist zu pruefen; bevorzugt werden konkrete Dateien.

Aus der archivierten Datei `docs/Nach_jeder_Aenderung_in_CMD.md` uebernommen:

```text
cd C:\Users\hraki\OneDrive\Anzucht\Hauptprogramme
git status
git add .
git commit -m "Beschreibung der Aenderung"
git push
git commit -m "Update Hauptprogramm V16"
```

Bewertung:

- `Update Hauptprogramm V16` ist zu pruefen, da das aktuelle Projekt `Hauptprogramm_V17` ist.
- Der Pfad ist zu pruefen, da die aktuelle Arbeit im Unterordner `Hauptprogramm_V17` stattfindet.
- `pio run` fehlte und ist nach Codeaenderungen zu pruefen beziehungsweise auszufuehren.

## Alte Werkzeuguebersicht

Aus der archivierten Datei `docs/Grundaufbau_Entwicklungsumgebung.txt` uebernommen:

```text
VS Code
PlatformIO Extension
Git
GitHub-Konto
Codex
Arduino IDE
USB-Treiber
```

Bewertung:

- In `docs/05_Entwicklung.md` uebernommen.
- Arduino IDE als Reserve ist zu pruefen, bevor daraus ein verbindlicher Ablauf abgeleitet wird.

## Datei-/Namenshinweis

Beobachtung aus der aktuellen Dateiliste:

- Das Dateisystem zeigt `CODING_RULES.md`.
- `git ls-files` meldet aktuell ebenfalls `CODING_RULES.md`.

Bewertung:

- Kein aktueller Case-Mismatch in diesem Repo belegt.

## Unklare Dateien nach docs-Organisation

Diese Dateien wurden nicht geloescht und nicht inhaltlich veraendert, bleiben aber fachlich zu pruefen:

- `docs/hardware/Parameter.xlsx`: nicht im aktuell gelesenen Repo vorhanden; zu pruefen, falls diese Datei extern existiert.
- `docs/hardware/OPV_Trocknungszelt.jpeg`: nicht im aktuell gelesenen Repo vorhanden; zu pruefen, falls diese Datei extern existiert.
- `docs/datasheets/Suswe-750_manual.pdf`: nicht im aktuell gelesenen Repo vorhanden; zu pruefen, falls diese Datei extern existiert.
- `docs/archiv/ESP32_Programme/Z_Alt/Hauptprogramm_V16/wifi_secrets.h`: nicht im aktuell gelesenen Repo vorhanden; Secret-Archivhinweis bleibt zu pruefen, falls dieser externe Archivstand existiert.
- `docs/archiv/ESP32_Programme/`: nicht im aktuell gelesenen Repo vorhanden; zu pruefen, falls dieser externe Archivstand existiert.
