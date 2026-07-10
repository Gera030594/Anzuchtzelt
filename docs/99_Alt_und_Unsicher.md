# Alte und unsichere Informationen

Status: historischer Stand vor Hauptprogramm_V18. Inhalte in dieser Datei sind keine aktuelle Anleitung und duerfen nur nach erneuter Pruefung verwendet werden.

Das fruehere WS2812-/LedStatus-System ist im aktuellen Hauptprogramm_V18 nicht mehr vorhanden. Veraltete NeoPixelBus- und SPI-Shim-Anweisungen werden nicht als historische Arbeitsanleitung aufbewahrt.

## Historische README-Hinweise

Aus einer frueheren `README.md` uebernommen:

```text
Anzuchtzelt
ESP32-Programme fuer das Anzuchtzelt.
Hauptprogramm_V16
Hauptsteuerung fuer das grosse Zelt.
Watchdog_ESP32
Separater Watchdog-Controller zur Ueberwachung.
```

Historische Bewertung:

- `Hauptprogramm_V16` bezeichnet einen alten Programmstand. Das aktuelle Projekt ist Hauptprogramm_V18.
- `Watchdog_ESP32` war ein separater Projektverweis und beschreibt nicht die aktuelle Struktur dieses Repositories.

Alter Pfadhinweis aus der frueheren `README.md`:

```text
C:\Users\hraki\OneDrive\Anzucht\Grosses Zelt\ESP32_Programme\Hauptprogramme
```

Historische Bewertung:

- Der Pfad stammt aus einer frueheren Ablage und ist keine aktuelle Arbeitsanweisung fuer Hauptprogramm_V18.

## Historische Workflow-Hinweise

Aus der frueheren Datei `docs/Normaler_Arbeitsablauf.md` uebernommen:

```text
Dateien aendern
git status
git diff
git add .
git commit -m "Beschreibung der Aenderung"
git push
```

Historische Bewertung:

- Dieser Ablauf wurde durch `docs/05_Entwicklung.md` ersetzt.
- `pio run` fehlte; nach Codeaenderungen ist der PlatformIO-Build zu pruefen.
- Statt `git add .` sollen konkrete Dateien hinzugefuegt werden.

Aus der frueheren Datei `docs/Nach_jeder_Aenderung_in_CMD.md` uebernommen:

```text
cd C:\Users\hraki\OneDrive\Anzucht\Hauptprogramme
git status
git add .
git commit -m "Beschreibung der Aenderung"
git push
git commit -m "Update Hauptprogramm V16"
```

Historische Bewertung:

- `Update Hauptprogramm V16` bezeichnet einen alten Programmstand; aktuell ist Hauptprogramm_V18.
- Der genannte Pfad und `git add .` sind keine aktuelle Arbeitsanweisung.
- `pio run` fehlte; der aktuelle Ablauf steht in `docs/05_Entwicklung.md`.

## Historische Werkzeuguebersicht

Aus der frueheren Datei `docs/Grundaufbau_Entwicklungsumgebung.txt` uebernommen:

```text
VS Code
PlatformIO Extension
Git
GitHub-Konto
Codex
Arduino IDE
USB-Treiber
```

Historische Bewertung:

- Die aktuelle Werkzeuguebersicht steht in `docs/05_Entwicklung.md`.
- Aus diesem historischen Block allein darf kein verbindlicher Ablauf abgeleitet werden.

## Historischer Datei-/Namenshinweis

Bei einer frueheren Pruefung wurde sowohl im Dateisystem als auch in `git ls-files` der Name `CODING_RULES.md` festgestellt. Der Hinweis belegt nur den damaligen Stand und ist keine Aussage ueber spaetere Arbeitsstaende.

## Historisch unklare externe Dateien

Bei einer frueheren Dokumentationsorganisation wurden folgende externe oder nicht vorhandene Dateien erwaehnt:

- `docs/hardware/Parameter.xlsx`
- `docs/hardware/OPV_Trocknungszelt.jpeg`
- `docs/datasheets/Suswe-750_manual.pdf`
- `docs/archiv/ESP32_Programme/Z_Alt/Hauptprogramm_V16/wifi_secrets.h`
- `docs/archiv/ESP32_Programme/`

Diese Liste beschreibt keinen aktuellen Repository-Inhalt. Falls solche Dateien ausserhalb dieses Projekts existieren, muessen Zweck, Aktualitaet und insbesondere moegliche Secret-Inhalte separat geprueft werden.
