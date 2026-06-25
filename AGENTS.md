# Agenten-Anweisungen

Für alle Codeänderungen in diesem Repository gilt:

1. `CODING_RULES.md` lesen und befolgen.
2. Bestehende Architektur und Task-Struktur erhalten.
3. Code muss non-blocking bleiben.
4. Pins nicht ändern, außer es wird ausdrücklich verlangt.
5. Secrets und Secret-Beispieldateien nicht ändern, außer es wird ausdrücklich verlangt.
6. BME680-Recovery-Logik nicht entfernen.
7. Heartbeat-Watchdog-Logik nicht entfernen.
8. Motor-Failsafe-Logik nicht entfernen.
9. Kalibrierungs-/NVS-Logik nicht entfernen.
10. PlatformIO-Build-Kompatibilität erhalten.
11. Kleine, prüfbare Änderungen bevorzugen.
12. Nach Codeänderungen `pio run` ausführen oder anfordern.