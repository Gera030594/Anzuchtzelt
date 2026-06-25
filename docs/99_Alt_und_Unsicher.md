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

Aus `docs/archiv/Normaler_Arbeitsablauf.md` uebernommen:

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

Aus `docs/archiv/Nach_jeder_Aenderung_in_CMD.md` uebernommen:

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

Aus `docs/archiv/Grundaufbau_Entwicklungsumgebung.txt` uebernommen:

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

Beobachtung aus der Dateiliste:

- Das Dateisystem zeigt `CODING_RULES.md`.
- `git ls-files` meldete `CODING_Rules.md`.

Bewertung:

- Zu pruefen, falls das Repository auf einem case-sensitiven Dateisystem verwendet wird.

## Unklare Dateien nach docs-Organisation

Diese Dateien wurden nicht geloescht und nicht inhaltlich veraendert, bleiben aber fachlich zu pruefen:

- `docs/hardware/Parameter.xlsx`: zu pruefen, da die Sheets `Parameter` und `Fault Code` nicht eindeutig einer aktuellen Funktion zugeordnet wurden.
- `docs/hardware/OPV_Trocknungszelt.jpeg`: zu pruefen, da der konkrete Bezug zum aktuellen Aufbau nicht aus dem Dateinamen allein belegbar ist.
- `docs/datasheets/Suswe-750_manual.pdf`: zu pruefen, da die konkrete verbaute Komponente nicht aus dem Dateinamen allein belegbar ist.
- `docs/archiv/ESP32_Programme/Z_Alt/Hauptprogramm_V16/wifi_secrets.h`: zu pruefen, da eine Datei mit Secret-Namen im Archiv liegt. Inhalt wurde nicht geprueft oder veraendert.
- `docs/archiv/ESP32_Programme/`: zu pruefen, da es sich um alte Programmstaende und Nebenprogramme handelt, nicht um die aktuelle Hauptdokumentation.
