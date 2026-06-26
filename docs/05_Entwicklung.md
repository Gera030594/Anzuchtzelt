# Entwicklung

Status: ersetzt die alten Workflow-Notizen. Die archivierten alten Hinweise liegen aktuell direkt unter `docs/` und sind in `docs/99_Alt_und_Unsicher.md` bewertet.

## Werkzeuge

Aus vorhandener Dokumentation belegt:

- VS Code
- PlatformIO Extension
- Git
- GitHub-Konto
- Codex
- Arduino IDE als Reserve
- USB-Treiber fuer ESP32

Zu pruefen:

- `.vscode/extensions.json` ist vorhanden und enthaelt zwei PlatformIO-nahe Empfehlungen.
- Zu pruefen bleibt nur, welche dieser Empfehlungen lokal bevorzugt wird.

## Normaler Ablauf

```powershell
git status
git diff
pio run
git add <konkrete Dateien>
git commit -m "type: kurze beschreibung"
git push
```

## Nach Codeaenderungen

Aus `AGENTS.md` und `CODING_RULES.md` belegt:

- `pio run` ausfuehren oder melden, falls nicht moeglich.
- Bestehende Architektur und Task-Struktur erhalten.
- Non-blocking bleiben.
- Keine Pins aendern, ausser ausdruecklich verlangt.
- Keine Secrets oder Secret-Beispieldateien aendern, ausser ausdruecklich verlangt.
- BME680-Recovery, Heartbeat-Watchdog, Motor-Failsafe und Kalibrierung/NVS nicht entfernen.

## Commit-Hinweise

Aus `CODING_RULES.md` belegt:

- `feat`: neue Funktion
- `fix`: Fehlerbehebung
- `refactor`: interne Umstrukturierung
- `docs`: Dokumentation
- `chore`: Projektpflege
- `test`: Tests

Zu pruefen:

- Vor `git push` sicherstellen, dass keine Secrets, `.pio`-Dateien oder unnoetigen generierten Dateien enthalten sind.

## Dokumentationsaenderungen

- README kurz halten.
- Doppelte Inhalte vermeiden.
- Alte oder unsichere Hinweise in `docs/99_Alt_und_Unsicher.md` bewerten.
- Archivierte alte Notizen liegen aktuell direkt unter `docs/`, nicht in `docs/archiv/`.
- Unsichere Aussagen mit `zu pruefen` markieren.
