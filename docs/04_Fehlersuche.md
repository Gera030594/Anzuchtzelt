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
- Secrets duerfen nicht im Code gespeichert werden.

Aus aktuellem Code belegt:

- Die Home-Assistant-Entitaet `ESP32-Verbindung` verwendet `anzuchtzelt/status` mit `online` oder `offline`.
- Die Home-Assistant-Entitaet `Zeitsynchronisation` verwendet `anzuchtzelt/status/time_synced` mit `true` oder `false`.
- `online` wird nach erfolgreicher MQTT-Verbindung retained veroeffentlicht; `offline` ist als retained Last Will konfiguriert.

Zu pruefen:

- Vorhandensein der lokalen Secret-Dateien, ohne Zugangsdaten auszugeben oder zu protokollieren.
- WLAN-Erreichbarkeit.
- Erreichbarkeit des konfigurierten MQTT-Brokers.
- Serial-Diagnose.
- Rohstatus unter `anzuchtzelt/status` und `anzuchtzelt/status/time_synced`.
- Ob bei einer wiederhergestellten Verbindung ein retained `online` das vorherige `offline` ersetzt.
- Ob die Zeit bereits synchronisiert wurde. `false` kann waehrend des Starts zunaechst erwartbar sein.

## MQTT- oder Home-Assistant-Status fehlt

Aus aktuellem Code belegt:

- Nach erfolgreicher MQTT-Verbindung werden die Home-Assistant-Discovery-Konfigurationen mit QoS 1 und retained veroeffentlicht.
- Alle Statusentitaeten verwenden die Verfuegbarkeit `anzuchtzelt/status`, soweit sie nicht selbst diese Verbindung abbilden.
- Die bereitgestellten Entitaeten sind Statusanzeigen. Es sind keine MQTT-Steuerbefehle oder Command-Topics implementiert.

Zu pruefen:

- Zuerst WLAN- und Brokerverbindung anhand der Serial-Diagnose pruefen.
- `anzuchtzelt/status` pruefen. Ein retained `offline` kann vom Last Will eines abgebrochenen Verbindungszustands stammen; nach erfolgreicher Neuverbindung muss retained `online` veroeffentlicht werden.
- Pruefen, ob die Discovery-Konfigurationen unter `homeassistant/+/+/config` retained vorhanden sind und Home-Assistant-MQTT-Discovery aktiviert ist.
- Bei vorhandener Discovery, aber fehlenden Werten die jeweiligen `state_topic`-Topics und die Availability pruefen.
- Keine Zugangsdaten in Diagnoseausgaben, Screenshots oder Dokumentation uebernehmen.

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
- `anzuchtzelt/status/bme_ok` meldet retained `true` oder `false`.
- Die Home-Assistant-Entitaet `BME680 Fehler` wertet `false` als Problemzustand.
- Temperatur und Luftfeuchte werden nur bei einem aktuell verwendbaren BME-Zustand unter `anzuchtzelt/sensor/temperature` und `anzuchtzelt/sensor/humidity` veroeffentlicht.

Zu pruefen:

- Sensorverkabelung.
- Sensoradresse.
- Serial-Diagnose, `BME680 Fehler` und `anzuchtzelt/status/bme_ok`.
- Temperatur- und Luftfeuchte-Topics zusammen mit `bme_ok` bewerten. Bei `false` werden keine neuen Messwerte veroeffentlicht; zuvor retained gespeicherte Messwerte koennen noch sichtbar sein.

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
- Die Home-Assistant-Entitaet `Motorfehler` verwendet `anzuchtzelt/status/motor_fault` mit `none`, `poti_invalid`, `timeout` oder als Absicherung `unknown`.
- Die Poti-Gueltigkeit steht unter `anzuchtzelt/status/poti_feedback_valid` mit `true` oder `false` bereit.
- Die Stellposition steht bei gueltiger Rueckmeldung unter `anzuchtzelt/sensor/motor_position_pct` als ganzzahliger Wert von 0 bis 100 bereit.

Serial-Ausgabe bei Move-Timeout:

```text
[MOTOR] ERROR: MOVE_TIMEOUT cur=... target=... dir=... elapsed=...
```

Zu pruefen:

- Poti-Rohwerte.
- Kalibrierung.
- L298N-Versorgung und Verdrahtung.
- Mechanische Blockade oder Endlage.
- Serial-Diagnose und Home-Assistant-Entitaet `Motorfehler`.
- `poti_feedback_valid` zuerst pruefen. Bei `false` wird keine neue Stellposition veroeffentlicht; ein zuvor retained gespeicherter Positionswert kann noch sichtbar sein.
- `motor_fault`, `poti_feedback_valid`, `motor_position_pct` und den zusammengefassten `system_state` gemeinsam auswerten.

## Heartbeat-Timeout

Aus `CODING_RULES.md` belegt:

- Heartbeat RX erfolgt per Interrupt.
- Timeout fuehrt zu einem Relaispuls am Watchdog-Reset-Relais.
- OK-Zustand darf nicht kuenstlich ohne echten RX erzeugt werden.

Aus aktuellem Code belegt:

- Heartbeat-TX erfolgt nur bei gesundem BME ueber `bmeHealthy(now)`.
- Heartbeat-Timeout: 20 s ohne RX.
- Relaispulsdauer: 1 s.
- Nach einem Relaispuls wird der naechste Reset erst nach Ablauf der naechsten Timeout-Frist erlaubt.
- Die Home-Assistant-Entitaet `Heartbeat` verwendet `anzuchtzelt/status/heartbeat` mit `grace`, `ok`, `timeout` oder als Absicherung `unknown`.
- Ein Heartbeat-Timeout setzt `anzuchtzelt/status/system_state` auf `fault`.

Zu pruefen:

- Heartbeat-Verbindung.
- Watchdog-Relais.
- Serial-Diagnose.
- Home-Assistant-Entitaet `Heartbeat` sowie die Rohstatus `heartbeat` und `system_state`.

## Systemstatus ist nicht `ok`

Aus aktuellem Code belegt:

- Die Home-Assistant-Entitaet `Systemstatus` verwendet `anzuchtzelt/status/system_state`.
- `fault` bedeutet Heartbeat-Timeout, Motorfehler, ungueltige Poti-Rueckmeldung oder BME-Fehler.
- `calibration` bedeutet aktiven Kalibriermodus, sofern kein vorrangiger Fehler besteht.
- `starting` bedeutet Heartbeat-Grace oder noch nicht synchronisierte Systemzeit.
- `unknown` ist der sichere Rueckfall fuer einen nicht einordenbaren Zustand.
- Der Systemstatus ist eine zusammengefasste Diagnose und keine Steuerfunktion.

Zu pruefen:

- Bei `fault`: `bme_ok`, `heartbeat`, `motor_fault` und `poti_feedback_valid` einzeln pruefen.
- Bei `starting`: `heartbeat` und `time_synced` pruefen.
- Bei `calibration`: `anzuchtzelt/status/calibration_mode_active` pruefen.
- Bei `unknown`: Serial-Diagnose und alle Detailstatus pruefen.
- Bei Availability `offline` zuerst WLAN und MQTT-Verbindung pruefen; `offline` ist nicht selbst einer der Systemstatuswerte.

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
