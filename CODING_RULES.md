# Coding Rules – Anzuchtzelt ESP32

## Ziel

Dieses Projekt steuert ein Anzuchtzelt mit ESP32.

Der Code muss die bestehende Arbeitsweise erhalten:

* non-blocking Ablauf
* klare Task-Struktur
* stabile Hardware-Ansteuerung
* nachvollziehbare Diagnose über Serial, MQTT und Home Assistant
* sichere Behandlung von Sensorfehlern
* keine sensiblen Daten im Repository
* PlatformIO-kompatibler Aufbau

Diese Regeln gelten für alle Änderungen am Projekt.

---

## Verbindliches Grundprinzip

Bestehende Funktionslogik wird erhalten.

Keine Änderung darf ohne ausdrückliche Freigabe:

* den Programmablauf grundsätzlich neu schreiben
* die Task-Struktur entfernen
* blockierende Wartezeiten einbauen
* Pins ändern
* Secrets anfassen
* Libraries austauschen
* MQTT-Statusausgabe oder Home-Assistant-Discovery entfernen
* BME680-Fehlerbehandlung oder BME680-Reinit entfernen
* Heartbeat-Watchdog entfernen
* Poti-Kalibrierung entfernen
* NVS-Kalibrierwerte überschreiben
* Motor-Failsafe verschlechtern

Bei Unsicherheit gilt:

```text
kleinste sinnvolle Änderung
bestehende Struktur erhalten
Buildfähigkeit erhalten
Hardware-Sicherheit erhalten
```

---

## Projektstruktur

Die PlatformIO-Grundstruktur bleibt:

```text
Hauptprogramme/
├─ platformio.ini
├─ src/
│  └─ main.cpp
├─ include/
│  ├─ wifi_secrets.h
│  └─ wifi_secrets.example.h
├─ lib/
├─ test/
├─ CODING_RULES.md
├─ README.md
└─ .gitignore
```

Die Fachlogik ist in Module ausgelagert; `main.cpp` orchestriert Setup und Loop.
Weitere Auslagerungen oder Strukturänderungen erfolgen nur schrittweise.

Aktuelle Modulstruktur:

```text
lib/
├─ Config/
│  ├─ Pins.h
│  └─ Config.h
├─ BmeSensor/
│  ├─ BmeSensor.h
│  └─ BmeSensor.cpp
├─ MotorControl/
│  ├─ MotorControl.h
│  └─ MotorControl.cpp
├─ HeartbeatWatchdog/
│  ├─ HeartbeatWatchdog.h
│  └─ HeartbeatWatchdog.cpp
├─ Calibration/
│  ├─ Calibration.h
│  └─ Calibration.cpp
├─ LampControl/
│  ├─ LampControl.h
│  └─ LampControl.cpp
├─ MqttStatus/
│  ├─ MqttStatus.h
│  └─ MqttStatus.cpp
├─ PotiFeedback/
│  ├─ PotiFeedback.h
│  └─ PotiFeedback.cpp
├─ SerialCommands/
│  ├─ SerialCommands.h
│  └─ SerialCommands.cpp
└─ WifiTime/
   ├─ WifiTime.h
   └─ WifiTime.cpp
```

`main.cpp` bleibt die schlanke Orchestrierung, solange die Aufrufreihenfolge und
Task-Struktur erhalten bleiben.

---

## Hauptarchitektur

Der Code arbeitet nach folgendem Muster:

```text
setup()
├─ Initialisierung
├─ Hardware vorbereiten
├─ Poti, Motor und Heartbeat initialisieren
├─ BME680 initialisieren und initial prüfen
├─ NVS öffnen und Kalibrierung laden
├─ Lampensteuerung starten
├─ WLAN starten
├─ MQTT-Statusmodul starten
└─ Startzustand herstellen

loop()
├─ now = millis()
├─ Poti-Task
├─ BME680-Task
├─ Heartbeat-Task
├─ Motor-Task
├─ WLAN-Handler
├─ NTP-Handler
├─ Lampen-Handler
├─ MQTT-Status veröffentlichen
└─ Serial-Command-Task
```

`loop()` bleibt schlank.

Erlaubt in `loop()`:

```cpp
const unsigned long now = millis();

readPotiTask(now);
bme680Task(now);
heartbeatTask(now);
motorControlTask(now);
handleWiFi(now);
handleNTP(now);
handleLamp(now);
mqttStatusTask(now);
serialCommandTask(now);
```

Nicht erlaubt in `loop()`:

```text
lange if-Ketten
direkte Sensorlogik
direkte Motorlogik über viele Zeilen
direkte WLAN-Verbindungslogik
direkte NTP-Logik
blockierende Wartezeiten
Serial-Menüs als große Inline-Blöcke
```

---

## Non-blocking-Regel

Der gesamte Code bleibt non-blocking.

Nicht erlaubt:

```cpp
delay(5000);
while (...) {
}
```

Erlaubt nur in sehr kleinen Hardware-Sonderfällen:

```cpp
delayMicroseconds(...);
```

oder sehr kurze Wartezeiten, wenn die Hardware das zwingend braucht und es kommentiert ist.

Standard ist:

```cpp
unsigned long now = millis();

if (now - lastRun >= INTERVAL_MS) {
    lastRun = now;
    runTask();
}
```

Jede zyklische Funktion bekommt bevorzugt `now` als Parameter:

```cpp
void bme680Task(unsigned long now);
void heartbeatTask(unsigned long now);
void motorControlTask(unsigned long now);
void handleWiFi(unsigned long now);
void handleNTP(unsigned long now);
void handleLamp(unsigned long now);
```

---

## Zeitvariablen

Zeitvariablen werden eindeutig benannt.

Erlaubte Muster:

```cpp
lastBMEGood_ms
lastBMEAttempt_ms
bmeReinitStart_ms
lastHB_TX_ms
lastHB_RX_ms
moveStart_ms
relayStart_ms
boot_ms
```

Regel:

```text
Zeitpunkte enden bevorzugt auf _ms
Intervalle enden bevorzugt auf _MS oder _INTERVAL
Timeouts enden bevorzugt auf _TIMEOUT
Pulsdauern enden bevorzugt auf _PULSE_LEN
```

Beispiele:

```cpp
const unsigned long BME_INTERVAL = 10000UL;
const unsigned long HB_TIMEOUT = 20000UL;
const unsigned long RELAY_PULSE_LEN = 1000UL;
```

Neue Zeitwerte dürfen nicht als nackte Zahlen im Code stehen.

Nicht erlaubt:

```cpp
if (now - lastCheck > 30000) {
}
```

Besser:

```cpp
const unsigned long WIFI_RETRY_INTERVAL_MS = 30000UL;

if (now - lastCheck > WIFI_RETRY_INTERVAL_MS) {
}
```

---

## Setup-Regel

`setup()` initialisiert nur.

Erlaubt:

```cpp
initGeneral();
initRelay();
initPoti();
initMotorControl();
initHeartbeat();
bmeInit();
bmeInitialCheck();
openNVS();
calInitLoad();
initHardwareLampensteuerung();
connectWiFi();
initMqttStatus();
checkLampState();
```

Nicht erlaubt:

```text
Dauerbetrieb
lange Schleifen
blockierende WLAN-Warteketten
endlose Sensorprüfungen
dauerhafte Kalibrierlogik
```

---

## Task-Regel

Jede größere Funktionseinheit läuft als eigene Task- oder Handler-Funktion.

Bestehende Task-Funktionen:

```cpp
readPotiTask(unsigned long now);
bme680Task(unsigned long now);
heartbeatTask(unsigned long now);
motorControlTask(unsigned long now);
mqttStatusTask(unsigned long now);
serialCommandTask(unsigned long now);
handleWiFi(unsigned long now);
handleNTP(unsigned long now);
handleLamp(unsigned long now);
```

Neue zyklische Aufgaben folgen demselben Muster:

```cpp
void updateNameTask(unsigned long now);
```

oder:

```cpp
void handleName(unsigned long now);
```

---

## Namensregeln

### Funktionen

Funktionen verwenden `camelCase`.

Beispiele:

```cpp
readPotiRaw();
rawToPercent();
tempToSetpoint();
motorStop();
motorDriveToward();
changeLogMaybe();
printStatus();
calSave();
calList();
bmeService();
checkLampState();
synchronizeTime();
isSommerzeit();
```

### Init-Funktionen

Initialisierungen beginnen mit `init`.

```cpp
initGeneral();
initRelay();
initPoti();
initMotorControl();
initHeartbeat();
initHardwareLampensteuerung();
initMqttStatus();
```

### Handler/Tasks

Zyklische Funktionen heißen:

```cpp
...Task(now)
```

oder:

```cpp
handle...(now)
```

### Konstanten

Konstanten verwenden bevorzugt `UPPER_SNAKE_CASE`.

```cpp
BME_INTERVAL
BME_RETRY_MS
BME_BAD_LIMIT
HB_SEND_INTERVAL
HB_TIMEOUT
POTI_MIN_RAW
POTI_MAX_RAW
DEAD_BAND_PCT
MIN_DUTY
MAX_DUTY
MOVE_TIMEOUT
RELAY_PULSE_LEN
```

### Pins

Pin-Konstanten verwenden `UPPER_SNAKE_CASE` und enden auf `_PIN`, außer bestehende Namen bleiben aus Kompatibilitätsgründen erhalten.

```cpp
POTI_ADC_PIN
HB_IN_PIN
HB_OUT_PIN
C3_WD_RELAIS_PIN
L298N_IN1
L298N_IN2
L298N_ENA
```

Bestehende Namen wie `relayPin` und `modePin` dürfen bleiben, sollen bei späterer Auslagerung aber zentralisiert werden.

---

## Pinbelegung

Pinbelegung wird zentral gehalten.

Derzeit in `lib/Config/Pins.h`:

```cpp
L298N_IN1
L298N_IN2
L298N_ENA
POTI_ADC_PIN
HB_IN_PIN
HB_OUT_PIN
C3_WD_RELAIS_PIN
relayPin
modePin
```

GPIO16 ist aktuell frei und nicht für eine Projektfunktion reserviert. Es
existiert keine WS2812-Datenleitung und keine externe Statusanzeige am ESP32.

Neue Pinzahlen dürfen nicht verstreut im Code stehen.

Nicht erlaubt:

```cpp
pinMode(23, OUTPUT);
```

Besser:

```cpp
pinMode(C3_WD_RELAIS_PIN, OUTPUT);
digitalWrite(C3_WD_RELAIS_PIN, HIGH);
```

Zentrale Ablage:

```text
lib/Config/Pins.h
```

---

## Konfiguration

Alle festen technischen Werte werden zentral als Konstanten geführt.

Beispiele aus dem Bestand:

```cpp
PWM_FREQ
PWM_RES_BITS
PWM_CHANNEL
BME_INTERVAL
BME_RETRY_MS
BME_BAD_LIMIT
HB_SEND_INTERVAL
HB_TIMEOUT
HB_PULSE_LEN
POTI_MIN_RAW
POTI_MAX_RAW
DEAD_BAND_PCT
MIN_DUTY
MAX_DUTY
MOVE_TIMEOUT
RELAY_PULSE_LEN
lampCheckInterval
ntpSyncInterval
retrySyncInterval
wifiCheckInterval
```

Neue Grenzwerte, Intervalle, Zielwerte oder Hardwareparameter werden nicht direkt in Logik geschrieben.

Nicht erlaubt:

```cpp
if (temperature > 28.0) {
}
```

Besser:

```cpp
constexpr float TEMP_UPPER_WARN_C = 28.0f;

if (temperature > TEMP_UPPER_WARN_C) {
}
```

---

## Secrets

WLAN- und MQTT-Zugangsdaten liegen nur in nicht getrackten lokalen Dateien.

Aktueller Projektstand:

```cpp
#include "../../include/wifi_secrets.h"
#include "../../include/mqtt_secrets.h"
```

Die lokalen Secret-Dateien sind:

```text
include/wifi_secrets.h
include/mqtt_secrets.h
```

Zugangsdaten dürfen nicht als Stringliterale in Quellcode oder Dokumentation
stehen. Der Programmcode verwendet ausschließlich die Makros aus den lokalen
Secret-Headern.

Die Secret-Dateien dürfen nicht committed werden.

`.gitignore` muss mindestens enthalten:

```gitignore
.pio/
.vscode/
include/wifi_secrets.h
include/mqtt_secrets.h
*.bin
*.elf
```

Die zugehörigen Beispieldateien dürfen committed werden:

```text
include/wifi_secrets.example.h
include/mqtt_secrets.example.h
```

---

## Status über MQTT und Home Assistant

Das aktuelle Projekt verwendet keine WS2812-Status-LEDs. Das frühere Modul
`lib/LedStatus`, NeoPixelBus und der lokale SPI-Shim wurden vollständig
entfernt. GPIO16 ist frei und nicht reserviert.

`MqttStatus` ist das zentrale, nur lesende Ausgabemodul. Fachmodule stellen
ihre Zustände über öffentliche Getter bereit; `MqttStatus` liest diese Werte
und veröffentlicht sie, ohne die Fachlogik oder interne Variablen zu ändern.

Verbindliche Regeln:

```text
retained Availability über anzuchtzelt/status
online-Publish bei Verbindung und offline als Last Will
Home-Assistant-MQTT-Discovery für genau 16 Statusentitäten
Zustände nach MQTT-Reconnect erneut veröffentlichen
Statuswerte im Normalbetrieb nur bei fachlicher Änderung veröffentlichen
Systemstatus nur als Zusammenfassung verwenden
Detailentitäten für die genaue Ursache erhalten
keine command_topics
keine MQTT-Switches oder MQTT-Number-Entitäten
keine Subscriptions für Steuerbefehle
keine internen Variablen der Fachmodule direkt verändern
```

Der Systemstatus auf `anzuchtzelt/status/system_state` verwendet die Payloads
`ok`, `starting`, `calibration`, `fault` und `unknown`.

Home Assistant ist die zentrale Statusanzeige. Die lokale Motor-, Lampen-,
Sensor-, Kalibrierungs- und Watchdog-Logik arbeitet unabhängig davon weiter,
ob Home Assistant geöffnet oder erreichbar ist. Ein Ausfall von Home
Assistant oder des Brokers darf diese lokale Fachlogik nicht stoppen.

Änderungen an Topics, Discovery-Entitäten oder deren fachlicher Bedeutung
werden gemeinsam mit der Übersicht in `README.md` aktualisiert.

---

## BME680-Regeln

Der BME680 bleibt robust angebunden.

Bestehende Regeln:

```text
Auto-Detect 0x76/0x77
Plausibilitätsprüfung Temperatur
Plausibilitätsprüfung Feuchte
Fehlmessungen zählen
bei mehreren Fehlern bmeOK=false
Reinit mit Fallback
normale I2C-Reinitialisierung über Wire.begin(...)
keine manuelle SDA/SCL-Bus-Recovery per Freitakten
keine blockierende Endlossuche
```

Bestehende Funktionen bleiben erhalten:

```cpp
bmeInit();
bmeInitialCheck();
bmeConfigure();
bmeTryInitNow();
bmeService(unsigned long now);
bme680Task(unsigned long now);
bmeHealthy(unsigned long now);
```

Plausibilitätsgrenzen:

```text
Temperatur ungültig: NaN, < 0 °C, > 100 °C
Feuchte ungültig: NaN, < 0 %, > 100 %
```

Sensorfehler dürfen nicht still ignoriert werden.

Bei ungültigen Werten muss mindestens passieren:

```text
bmeOK entsprechend setzen
Fehlerzähler aktualisieren
öffentlichen BME-Status für MQTT und Diagnose aktualisieren
Failsafe-Logik berücksichtigen
Serial-Diagnose ausgeben, falls relevant
```

---

## I2C-Reinitialisierung

Manuelle I2C-Bus-Recovery per SDA/SCL-Freitakten wird nicht mehr geschützt und soll nicht neu eingeführt werden.

Regel:

```text
BME680-Probleme werden über Fehlerzähler, Statusflags und Serial-Diagnose sichtbar gemacht
bei BME680-Problemen bleibt normale Reinitialisierung über Wire.begin(...) und bme.begin(...)
Auto-Detect 0x76/0x77 bleibt erhalten
keine manuelle SCL-Taktung
keine manuell erzeugte STOP-Bedingung über SDA/SCL
keine harte Dauerblockade
```

Der separate C3-Reset über `C3_WD_RELAIS_PIN` gehört zum Heartbeat-Watchdog und bleibt von der BME680-Reinitialisierung getrennt erhalten.

---

## Motorregelung

Der Motor wird über L298N gesteuert.

Bestehende technische Regeln:

```text
PWM über LEDC
PWM-Frequenz 10 kHz
PWM-Auflösung 10 Bit
ENA über PWM
IN1/IN2 für Richtung
Totband gegen Zittern
Mindest-PWM gegen Haftreibung
Max-PWM mit Reserve
Move-Timeout
Motorstopp bei Fehler
```

Bestehende Funktionen:

```cpp
int getMotorTargetPct();
void setMotorTargetPct(int pct);
MotorFault getMotorFault();
void clearMotorFault();
motorDriveToward(int curPct, int tgtPct);
motorStop();
motorControlTask(unsigned long now);
tempToSetpoint(float temp);
```

`targetPct` ist gekapselt und wird über `setMotorTargetPct()` auf 0..100 % begrenzt.

Aktueller Motorfehlerstatus:

```cpp
enum MotorFault {
  MOTOR_FAULT_NONE,
  MOTOR_FAULT_POTI_INVALID,
  MOTOR_FAULT_TIMEOUT
};
```

`getMotorFault()` liefert den Status. `clearMotorFault()` löscht ihn bewusst.

Aktuelle Bewegungslogik:

```text
Deadband entscheidet nur, ob eine neue Bewegung gestartet wird.
Beim Start werden activeTargetPct und moveDir gespeichert.
Während moveActive true ist, stoppt das Deadband die Bewegung nicht.
Aufwärtsfahrt stoppt bei curPct >= activeTargetPct.
Abwärtsfahrt stoppt bei curPct <= activeTargetPct.
moveDir == 0 stoppt den Motor.
```

Aktuelle Sperren:

```text
potiValid == false -> motorStop(), Bewegung abbrechen, MOTOR_FAULT_POTI_INVALID
MOTOR_FAULT_TIMEOUT -> motorStop(), Bewegung abbrechen, keine neue Bewegung bis clearMotorFault() oder Neustart
```

Temperatur-zu-Zielwert:

```text
<22 °C -> 10 %
22..32 °C -> linear 10..100 %
>=32 °C -> 100 %
```

Direkte Motorsteuerung außerhalb dieser Funktionen ist zu vermeiden.

Nicht bevorzugt:

```cpp
digitalWrite(L298N_IN1, HIGH);
digitalWrite(L298N_IN2, LOW);
ledcWrite(PWM_CHANNEL, duty);
```

Besser:

```cpp
motorDriveToward(curPct, activeTargetPct);
motorStop();
```

Failsafe-Regeln bleiben:

```text
Poti ungültig → Motor aus, MOTOR_FAULT_POTI_INVALID
Move-Timeout → Motor aus, MOTOR_FAULT_TIMEOUT, Timeout-Sperre bis clearMotorFault() oder Neustart
BME-Temperaturfehler oder kompletter BME-Lesefehler → definierter sicherer Zielwert
reine Feuchte-Unplausibilitaet bei gueltiger Temperatur → kein Motor-Failsafe
```

---

## Poti-Regeln

Das Poti ist die Positionsrückmeldung.

Bestehende Regeln:

```text
ADC 12 Bit
ADC_11db
Medianfilter
EMA-Glättung
Plausibilitätsgrenzen
Rohwert-zu-Prozent über LUT
Fallback bei ungültiger LUT
```

Bestehende Funktionen:

```cpp
readPotiRaw();
readPotiRawInstant();
rawToPercent(int raw);
readPotiTask(unsigned long now);
```

Regel:

```text
readPotiRaw() für normalen Betrieb
readPotiRawInstant() für Kalibrierung/Teach-In
```

Poti-Werte müssen immer auf Plausibilität geprüft werden.

Ungültiges Poti darf nicht zu Motorbewegung führen.

---

## Kalibrierung

Die LUT-Kalibrierung bleibt erhalten.

Bestehende Regeln:

```text
technisch mindestens 2 gültige Kalibrierpunkte
Slots: 0 %, 25 %, 50 %, 75 %, 100 %
Serial-Bedienung
NVS-Speicherung
Fallback bei weniger als 2 validen Punkten
bestehende LUT nicht ungewollt überschreiben
```

Bestehende Funktionen:

```cpp
calInitLoad();
calSave();
calList();
calEnter();
calExit();
calCaptureSlot(uint8_t slotIdx);
calRebuildSortedFromSlots();
```

Serielle Befehle:

```text
r → Statusdump
K → Kalibriermodus starten
1..5 → Punkt erfassen
L → Punkte anzeigen
S → Speichern in NVS
V → Reload aus NVS
Q → Verlassen
D → NVS-Diagnose
```

NVS-Regeln:

```text
Namespace: cal
gespeichert werden n, raw[], pct[]
NVS-Reparatur bei beschädigten Pages bleibt erhalten
Erase Flash nicht unbedacht aktivieren
```

Keine Änderung darf vorhandene Kalibrierwerte ungefragt löschen.

---

## Heartbeat-Watchdog

Der Heartbeat-Watchdog bleibt als Sicherheitsfunktion erhalten.

Bestehende Regeln:

```text
Heartbeat TX nur bei gesundem BME
Sendepuls zyklisch
Pulsbreite begrenzt
RX per Interrupt
Timeout-Erkennung
Boot-Schonfrist
Relais-Reset bei Timeout
Reset-Cooldown
Heartbeat-Status ist über Getter und MQTT auswertbar
```

Bestehende Funktionen:

```cpp
hbISR();
initHeartbeat();
heartbeatTask(unsigned long now);
```

ISR-Regeln:

```text
ISR so kurz wie möglich
keine Serial-Ausgabe in ISR
keine komplexe Logik in ISR
nur volatile/zeitkritische Statuswerte setzen
```

Timeout darf nicht durch künstliches Zurücksetzen verschleiert werden.

Echter RX bleibt die Grundlage für OK-Zustand.

---

## Relais-Regeln

Relaislogik wird gekapselt.

Bestehende Relais:

```text
C3_WD_RELAIS_PIN: Watchdog-Reset-Relais
relayPin: Lampen-Relais
```

Regel:

```text
aktive Logik muss dokumentiert sein
Relaiszustand muss als bool nachvollziehbar sein
Pulsrelais wird zeitgesteuert ausgeschaltet
kein delay() für Relaispulse
```

Beispiel:

```cpp
const bool RELAY_ACTIVE_LOW = false;
bool relayState = false;
bool relayActive = false;
unsigned long relayStart_ms = 0;
```

Direkte Relaislogik darf nicht mehrfach verstreut werden.

---

## WLAN-Regeln

WLAN bleibt non-blocking.

Bestehende Funktionen:

```cpp
connectWiFi();
checkWiFiReconnect();
handleWiFi(unsigned long now);
```

Regeln:

```text
keine blockierende Warteschleife auf WLAN
Reconnect zyklisch prüfen
Verbindungsversuch zeitlich begrenzen
Verbindungsstatus über MQTT-Availability und Serial diagnostizierbar machen
Secrets nicht im Code speichern
```

Nicht erlaubt:

```cpp
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
}
```

Besser:

```cpp
handleWiFi(now);
```

---

## NTP- und Zeitregeln

NTP wird zyklisch und fehlertolerant behandelt.

Bestehende Funktionen:

```cpp
synchronizeTime();
handleNTP(unsigned long now);
isSommerzeit(struct tm* timeinfo);
```

Regeln:

```text
NTP nur sinnvoll versuchen, wenn WLAN vorhanden ist
bei fehlender Zeit häufiger Retry
bei synchronisierter Zeit längeres Intervall
Sommer-/Winterzeit beachten
Zeitstatus in timeSynced speichern
```

Zeitabhängige Verbraucher dürfen fehlende Zeit nicht ignorieren.

---

## Lampensteuerung

Die Lampensteuerung bleibt als eigener Bereich erhalten.

Bestehende Funktionen:

```cpp
initHardwareLampensteuerung();
handleLamp(unsigned long now);
checkLampState();
```

Grow-Phase, Lampenmodus und logischer Relaiszustand werden über öffentliche
Getter gelesen und von `MqttStatus` nur lesend veröffentlicht.

Regeln:

```text
Lampenstatus zyklisch prüfen
kein permanentes Abfragen ohne Intervall
12h/18h-Modus über Schalter
12h/18h-Modus ist auch Grow-Phase: 18h/6h = Vegetation, 12h/12h = Bluete
Relaiszustand nur ändern, wenn neuer Zustand abweicht
WLAN/NTP-Status getrennt von Lampenlogik behandeln
```

---

## Serial-Regeln

Serial dient Diagnose und Bedienung.

Baudrate:

```text
115200
```

Bestehende Funktionen:

```cpp
serialCommandTask(unsigned long now);
printStatus();
changeLogMaybe(unsigned long now);
```

Regeln:

```text
Serial-Ausgabe darf loop nicht blockieren
Statusausgabe bei Änderung bevorzugen
voller Statusdump nur auf Befehl
Kalibrierbefehle klar getrennt behandeln
```

Logging-Präfixe sind bevorzugt:

```text
[INFO]
[WARN]
[ERROR]
[DEBUG]
```

Bestehende Ausgaben dürfen schrittweise auf dieses Format gebracht werden.

Nicht bevorzugt:

```cpp
Serial.println("geht nicht");
Serial.println("test");
```

Besser:

```cpp
Serial.println("[ERROR] BME680 read failed");
Serial.println("[INFO] Calibration saved");
```

---

## Kommentare

Kommentare erklären Zweck und Verhalten, nicht jede einzelne Codezeile.

Gut:

```cpp
// Relais nach Pulsdauer wieder loslassen, ohne den Timeout künstlich zurückzusetzen.
```

Schlecht:

```cpp
// Setzt Variable auf true
relayActive = true;
```

Kopfdokumentation bleibt erlaubt, muss aber aktuell gehalten werden.

Wenn Verhalten geändert wird, müssen diese Stellen aktualisiert werden:

```text
Kopfkommentar
README.md
CODING_RULES.md
Serial-Cheatsheet
Home-Assistant-Entitäten und MQTT-Topics
```

---

## Fehlerbehandlung

Hardwarefehler werden sichtbar gemacht.

Pflicht bei Sensor-/Hardwarefehlern:

```text
Statusvariable setzen
öffentlichen Status für MQTT und Diagnose aktualisieren
Serial-Diagnose ausgeben
sicheren Zustand herstellen
Reinitialisierung oder sicheren Fallback versuchen, falls passend
```

Keine stillen Fehler.

Beispiele:

```cpp
bmeOK = false;
motorStop();
// Der zugehörige Fachstatus wird über den öffentlichen Getter sichtbar.
Serial.println("[ERROR] Poti invalid, motor stopped");
```

---

## Failsafe-Regeln

Failsafe hat Vorrang vor Komfortfunktionen.

Bestehende Failsafe-Fälle:

```text
BME ungültig
BME nicht erreichbar
Poti ungültig
Motor-Move-Timeout
Heartbeat-Timeout
WLAN/NTP nicht verfügbar
```

Reaktion muss immer definiert sein.

Motor-Failsafe:

```text
Poti ungültig → Motor aus, MOTOR_FAULT_POTI_INVALID
Move-Timeout → Motor aus, MOTOR_FAULT_TIMEOUT, Timeout-Sperre bis clearMotorFault() oder Neustart
BME-Temperaturfehler oder kompletter BME-Lesefehler → sicherer Zielwert oder Motorstopp nach Logik
reine Feuchte-Unplausibilitaet bei gueltiger Temperatur → kein Motor-Failsafe
```

Heartbeat-Failsafe:

```text
Timeout → Heartbeat- und Systemstatus melden den Fehler über MQTT
Timeout → C3-Reset-Relaispuls
kein künstlicher OK-Zustand ohne echten RX
```

---

## Datentypen

Bestehende Arduino-kompatible Typen bleiben erlaubt.

Bevorzugt:

```cpp
uint8_t
uint16_t
uint32_t
unsigned long
int
float
bool
```

Regeln:

```text
Zeitwerte: unsigned long
ADC-Rohwerte: int
Prozentwerte: int
Temperatur/Feuchte: float
Status: bool oder enum class
```

Bei neuen Zuständen ist `enum class` bevorzugt.

Beispiel:

```cpp
enum class SensorState {
    Ok,
    InvalidValue,
    NotReachable,
    Recovering
};
```

Bestehende bool-Flags dürfen bleiben.

---

## Globale Variablen

Globale Variablen sind in Arduino-Projekten erlaubt, aber sie müssen einem klaren Bereich zugeordnet sein.

Bestehende Bereiche:

```text
WLAN/NTP
Lampensteuerung
Kalibrierung
PWM/Motor
MQTT-Status
BME680
Heartbeat
Poti
Relais
Serial-Änderungslog
```

Neue globale Variablen werden beim passenden Abschnitt eingefügt und kommentiert.

Nicht erlaubt:

```cpp
int x;
bool flag;
unsigned long t;
```

Besser:

```cpp
unsigned long lastFanUpdate_ms = 0;
bool fanRelayActive = false;
int targetHumidityPercent = 65;
```

---

## Präprozessor-Regel

Neue Konstanten sollen bevorzugt `constexpr` oder `const` verwenden.

Bestehende `#define` dürfen bleiben.

Nicht bevorzugt bei neuen Werten:

```cpp
#define NEW_INTERVAL 5000
```

Besser:

```cpp
constexpr unsigned long NEW_INTERVAL_MS = 5000UL;
```

`#define` bleibt erlaubt für:

```text
Library-Kompatibilität
Präprozessor-Schalter
bestehende Pin-/PWM-Werte bis zur späteren Bereinigung
```

---

## Externe Libraries

Libraries werden über `platformio.ini` verwaltet.

Aktuelle `lib_deps`:

```text
adafruit/Adafruit BME680 Library
adafruit/Adafruit Unified Sensor
adafruit/Adafruit BusIO
bertmelis/espMqttClient @ 1.7.3
```

NeoPixelBus und der lokale SPI-Shim gehören nicht mehr zum Projekt. Die vom
ESP32-Arduino-Framework bereitgestellte SPI-Unterstützung bleibt verfügbar,
unter anderem für Adafruit BusIO.

Regeln:

```text
keine Library ohne Grund austauschen
keine Fremdbibliothek manuell in src kopieren
Versionsänderungen dokumentieren
nach Library-Änderung Build prüfen
```

---

## PlatformIO-Regeln

Das Projekt muss mit PlatformIO baubar bleiben.

Vor jedem Commit:

```powershell
pio run
```

Bei Upload-Test:

```powershell
pio run -t upload
```

Serial Monitor:

```powershell
pio device monitor
```

Board-, Framework- und Library-Konfiguration liegt in:

```text
platformio.ini
```

---

## Git-Regeln

Vor jedem Commit:

```powershell
git status
pio run
```

Dann:

```powershell
git add .
git commit -m "type: short description"
```

Commit-Typen:

```text
feat: neue Funktion
fix: Fehlerbehebung
refactor: interne Umstrukturierung
docs: Dokumentation
chore: Projektpflege
test: Tests
```

Beispiele:

```text
docs: add coding rules
fix: prevent blocking wifi reconnect
refactor: split bme sensor logic
feat: add lamp mode status led
chore: update platformio dependencies
```

Keine Misch-Commits.

Ein Commit soll nur eine sachliche Änderung enthalten.

---

## Codex-Regeln

Codex darf:

```text
Buildfehler analysieren
kleine Fehler beheben
Funktionen auslagern
Kommentare aktualisieren
README aktualisieren
CODING_RULES aktualisieren
Serial-Ausgaben vereinheitlichen
Konstanten zentralisieren
```

Codex darf nicht ohne ausdrückliche Freigabe:

```text
Projekt komplett neu schreiben
Task-Struktur entfernen
loop() vergrößern
delay() einbauen
Pins ändern
Secrets ändern
Libraries austauschen
BME680-Fehlerbehandlung oder BME680-Reinit entfernen
Heartbeat-Watchdog entfernen
Kalibrierlogik entfernen
NVS-Daten löschen
MQTT-Statusausgabe oder Home-Assistant-Discovery entfernen
Motor-Failsafe abschwächen
```

Jede Codex-Aufgabe muss enthalten:

```text
Bestehende Struktur erhalten.
Non-blocking bleiben.
Keine Secrets anfassen.
Keine Pins ändern.
Buildfähigkeit erhalten.
```

---

## Refactoring-Regeln

Refactoring erfolgt schrittweise.

Erlaubte Reihenfolge:

```text
1. Konstanten sammeln
2. Pins auslagern
3. öffentliche Lesezugriffe der Fachmodule ergänzen
4. BME-Funktionen auslagern
5. Motorfunktionen auslagern
6. Kalibrierung auslagern
7. Heartbeat auslagern
8. WLAN/NTP auslagern
9. Lampensteuerung auslagern
10. Statuswerte nur lesend in MqttStatus integrieren
```

Nach jedem Schritt:

```text
Build ausführen
Upload testen, falls Hardware betroffen
Serial-Ausgabe prüfen
MQTT-Status und Discovery prüfen
Commit erstellen
```

Nicht erlaubt:

```text
mehrere Hardwarebereiche gleichzeitig umbauen
große Umbenennungen ohne Funktionstest
alte Logik löschen, bevor neue getestet ist
```

---

## Qualitätsprüfung vor Push

Vor jedem Push prüfen:

```text
Build erfolgreich
keine Secrets im Commit
keine .pio-Dateien im Commit
keine unnötigen Binärdateien
keine blockierenden delays
loop() bleibt schlank
Task-Struktur bleibt erhalten
BME680-Autodetect, Fehlerbehandlung und Reinit bleiben erhalten
Heartbeat bleibt erhalten
Motor-Failsafe bleibt erhalten
Kalibrierung bleibt erhalten
MQTT-Status bleibt nur lesend und vollständig
alle 16 Home-Assistant-Discovery-Entitäten bleiben erhalten
Serial-Befehle funktionieren
Pins unverändert
```

Technische Prüfung:

```powershell
git status
pio run
git diff --staged
git push
```

---

## Aktueller Code-Stil

Der vorhandene Code ist modular aufgebaut:

```text
src/main.cpp: Setup- und Loop-Orchestrierung
lib/Config: Konstanten und Pins
lib/BmeSensor: BME680 und Messwert-Cache
lib/Calibration: LUT und NVS
lib/HeartbeatWatchdog: Heartbeat und Reset-Failsafe
lib/LampControl: Grow-Phase und Lampenrelais
lib/MotorControl: Motorregelung, PWM und MotorFault
lib/MqttStatus: nur lesende MQTT-/Home-Assistant-Statusausgabe
lib/PotiFeedback: ADC- und Positionsrückmeldung
lib/SerialCommands: serielle Diagnose und Bedienung
lib/WifiTime: WLAN und Zeitsynchronisation
```

`main.cpp` bleibt schlank. Fachlogik gehört in das zuständige Modul;
`MqttStatus` konsumiert ausschließlich öffentliche Statusschnittstellen.

---

## Harte Regel

Eine Änderung ist nur akzeptabel, wenn danach gilt:

```text
Der ESP32 bleibt non-blocking.
Der Motor bleibt failsafe.
Der BME680 bleibt fehlerbehandelt und reinitialisierbar.
Der Heartbeat bleibt wirksam.
MQTT und Home Assistant bleiben eine nur lesende Statusausgabe.
Ein Ausfall von Home Assistant stoppt keine lokale Fachlogik.
Die Kalibrierung bleibt erhalten.
Die Secrets bleiben privat.
Das Projekt baut mit PlatformIO.
```
