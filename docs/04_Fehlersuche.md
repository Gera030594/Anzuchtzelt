# Fehlersuche

Status: aus vorhandener Projektdokumentation abgeleitet; konkrete Messwerte und reale Verdrahtung zu pruefen.

## Build schlaegt fehl

- `pio run` ausfuehren.
- `platformio.ini` nicht ohne ausdruecklichen Grund aendern.
- Library-Aenderungen nur begruendet und dokumentiert vornehmen.
- Secrets nicht einchecken.

Zu pruefen:

- Vollstaendige Fehlermeldung aus PlatformIO.

## WLAN oder NTP funktioniert nicht

Aus `CODING_RULES.md` belegt:

- WLAN bleibt non-blocking.
- NTP wird nur sinnvoll versucht, wenn WLAN vorhanden ist.
- WLAN/NTP-Status wird ueber LED7 angezeigt.
- Secrets duerfen nicht im Code gespeichert werden.

Zu pruefen:

- Lokale Secret-Datei.
- WLAN-Erreichbarkeit.
- Serial-Diagnose.
- Ob Zeit bereits synchronisiert wurde.

## BME680-Fehler

Aus `CODING_RULES.md` belegt:

- Auto-Detect 0x76/0x77 bleibt erhalten.
- Temperatur und Feuchte werden auf Plausibilitaet geprueft.
- Fehler werden nicht still ignoriert.
- Reinitialisierung erfolgt ueber normale I2C-/BME-Initialisierung.
- Keine manuelle SDA/SCL-Bus-Recovery einfuehren.

Aus aktuellem Code belegt:

- Nach 3 Fehlversuchen wird `bmeOK=false` gesetzt.
- Retry bei defektem BME erfolgt alle 3 s.
- Die Reinit-Wartezeit ist nicht-blockierend und betraegt 15 ms.
- Beim Init und Reinit wird 0x76/0x77 per Autodetect geprueft.

Zu pruefen:

- Sensorverkabelung.
- Sensoradresse.
- Serial-Diagnose und LED0/LED1/LED2.

## Motor bewegt sich nicht oder stoppt

Aus `CODING_RULES.md` belegt:

- Ungueltiges Poti fuehrt zu Motor aus.
- Move-Timeout fuehrt zu Motor aus.
- BME-Fehler fuehrt zu definiertem sicherem Zielwert oder Motorstopp nach Logik.

Aktuelle MotorFault-Zustaende:

- `MOTOR_FAULT_POTI_INVALID`: Poti-Rueckmeldung ungueltig, Motor wird gestoppt.
- `MOTOR_FAULT_TIMEOUT`: Bewegung hat `MOVE_TIMEOUT` ueberschritten, Motor wird gestoppt.
- Nach `MOTOR_FAULT_TIMEOUT` ist die Motorsteuerung verriegelt und startet keine neue Bewegung.
- Die Verriegelung bleibt bis Neustart oder internem `clearMotorFault()` bestehen.
- LED3 bleibt frei und zeigt keine MotorFault-Zustaende an.

Serial-Ausgabe bei Move-Timeout:

```text
[MOTOR] ERROR: MOVE_TIMEOUT cur=... target=... dir=... elapsed=...
```

Zu pruefen:

- Poti-Rohwerte.
- Kalibrierung.
- L298N-Versorgung und Verdrahtung.
- Mechanische Blockade oder Endlage.
- Serial-Diagnose und MotorFault-Status.

## Heartbeat-Timeout

Aus `CODING_RULES.md` belegt:

- Heartbeat RX erfolgt per Interrupt.
- Timeout fuehrt zu LED6 rot.
- Timeout fuehrt zu einem Relaispuls am Watchdog-Reset-Relais.
- OK-Zustand darf nicht kuenstlich ohne echten RX erzeugt werden.

Aus aktuellem Code belegt:

- Heartbeat-TX erfolgt nur bei gesundem BME ueber `bmeHealthy(now)`.
- Heartbeat-Timeout: 20 s ohne RX.
- Relaispulsdauer: 1 s.
- Nach einem Relaispuls wird der naechste Reset erst nach Ablauf der naechsten Timeout-Frist erlaubt.

Zu pruefen:

- Heartbeat-Verbindung.
- Watchdog-Relais.
- Serial-Diagnose.

## Kalibrierung oder NVS auffaellig

Aus `CODING_RULES.md` belegt:

- Kalibrierwerte werden in NVS gespeichert.
- Bestehende Werte duerfen nicht ungefragt geloescht werden.
- NVS-Diagnose ist ueber Serial-Befehl `D` vorgesehen.

Aus aktuellem Code belegt:

- `openNVS()` versucht den Namespace `cal` zu oeffnen.
- Bei bestimmten NVS-Init-Fehlern kann `openNVS()` `nvs_flash_erase()` ausfuehren.
- Das ist sicherheitsrelevant, weil dadurch NVS-Inhalte geloescht werden koennen.
- Diese Recovery darf nicht unbedacht geaendert oder erweitert werden.

Zu pruefen:

- Ausgabe von `L` und `D`.
- Ob mindestens zwei gueltige Kalibrierpunkte vorhanden sind.
