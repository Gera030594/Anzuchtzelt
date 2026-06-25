# Coding Rules – Anzuchtzelt ESP32

## Ziel

Dieses Projekt steuert ein Anzuchtzelt mit ESP32.

Der Code muss die bestehende Arbeitsweise erhalten:

* non-blocking Ablauf
* klare Task-Struktur
* stabile Hardware-Ansteuerung
* nachvollziehbare Diagnose über Serial und LEDs
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
* Status-LED-Logik entfernen
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
│  ├─ secrets.h
│  └─ secrets_example.h
├─ lib/
├─ test/
├─ CODING_RULES.md
├─ README.md
└─ .gitignore
```

Langfristig darf Code aus `main.cpp` in Module ausgelagert werden, aber nur schrittweise.

Zielstruktur für spätere Auslagerung:

```text
lib/
├─ Config/
│  ├─ Pins.h
│  └─ Config.h
├─ LedStatus/
│  ├─ LedStatus.h
│  └─ LedStatus.cpp
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
├─ WifiTime/
│  ├─ WifiTime.h
│  └─ WifiTime.cpp
└─ LampControl/
   ├─ LampControl.h
   └─ LampControl.cpp
```

Bis zur Auslagerung bleibt `main.cpp` erlaubt, solange die innere Abschnittsstruktur erhalten bleibt.

---

## Hauptarchitektur

Der Code arbeitet nach folgendem Muster:

```text
setup()
├─ Initialisierung
├─ Hardware vorbereiten
├─ NVS öffnen
├─ Sensoren prüfen
├─ WLAN/NTP/Lampensteuerung starten
└─ Startzustand herstellen

loop()
├─ now = millis()
├─ Poti-Task
├─ BME680-Task
├─ Heartbeat-Task
├─ Motor-Task
├─ LED-Update-Task
├─ Serial-Command-Task
├─ WLAN-Handler
├─ NTP-Handler
└─ Lampen-Handler
```

`loop()` bleibt schlank.

Erlaubt in `loop()`:

```cpp
const unsigned long now = millis();

readPotiTask(now);
bme680Task(now);
heartbeatTask(now);
motorControlTask(now);
ledUpdateTask();
serialCommandTask(now);
handleWiFi(now);
handleNTP(now);
handleLamp(now);
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
Serial.begin(115200);
initGeneral();
openNVS();
calInitLoad();
initPoti();
initLEDs();
initMotorControl();
initHeartbeat();
bmeInit();
bmeInitialCheck();
initHardwareLampensteuerung();
connectWiFi();
synchronizeTime();
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
ledUpdateTask();
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
initLEDs();
initMotorControl();
initHeartbeat();
initHardwareLampensteuerung();
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
LED_BRIGHTNESS
```

### Pins

Pin-Konstanten verwenden `UPPER_SNAKE_CASE` und enden auf `_PIN`, außer bestehende Namen bleiben aus Kompatibilitätsgründen erhalten.

```cpp
WS2812_PIN
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

Derzeit im Hauptcode:

```cpp
WS2812_PIN
NUM_LEDS
L298N_IN1
L298N_IN2
L298N_ENA
POTI_ADC_PIN
HB_IN_PIN
HB_OUT_PIN
C3_WD_RELAIS_PIN
relayPin
modePin
LED_STATUS
LED_MODE
```

Neue Pinzahlen dürfen nicht verstreut im Code stehen.

Nicht erlaubt:

```cpp
pinMode(23, OUTPUT);
digitalWrite(16, HIGH);
```

Besser:

```cpp
pinMode(C3_WD_RELAIS_PIN, OUTPUT);
digitalWrite(C3_WD_RELAIS_PIN, HIGH);
```

Spätere Zielstruktur:

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
LED_BRIGHTNESS
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

WLAN-Daten liegen nur in einer nicht getrackten Datei.

Erlaubt:

```cpp
#include "secrets.h"
```

oder bei bestehendem Projektstand:

```cpp
#include "wifi_secrets.h"
```

Nicht erlaubt:

```cpp
const char* ssid = "MeinWLAN";
const char* password = "MeinPasswort";
```

Empfohlen:

```cpp
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
```

Die Secret-Datei darf nicht committed werden.

`.gitignore` muss mindestens enthalten:

```gitignore
.pio/
.vscode/
include/secrets.h
include/wifi_secrets.h
*.bin
*.elf
```

Eine Beispiel-Datei darf committed werden:

```text
include/secrets_example.h
```

oder:

```text
include/wifi_secrets_example.h
```

---

## LED-Regeln

WS2812-LEDs werden über `NeoPixelBus` gesteuert.

Bestehende Grundregeln bleiben:

```text
LED-Farbe nur setzen, wenn sie sich ändert
Show() nur ausführen, wenn ledsDirty == true
Show() nur, wenn leds.CanShow()
maximale Update-Rate beachten
globale Helligkeit über LED_BRIGHTNESS
```

Bestehende Hilfsfunktionen bleiben erhalten:

```cpp
RgbColor C(uint8_t r, uint8_t g, uint8_t b);
RgbColor withBri(const RgbColor& c);
void ledSet(uint8_t i, const RgbColor& color);
void ledUpdateTask();
```

Direkte LED-Zugriffe außerhalb der LED-Hilfsfunktionen sind zu vermeiden.

Nicht bevorzugt:

```cpp
leds.SetPixelColor(3, RgbColor(...));
leds.Show();
```

Besser:

```cpp
ledSet(3, C(255, 0, 0));
```

`leds.Show()` bleibt auf `ledUpdateTask()` begrenzt.

---

## Status-LED-Belegung

Die Status-LEDs sind Teil der Diagnose und dürfen nicht ohne Ersatzlogik entfernt werden.

Bestehende Bedeutung:

```text
LED0/LED1/LED2: BME680 Plausibilitätsstatus
LED3: Motor/Poti/Failsafe
LED4: obere Temperaturzone
LED5: untere Temperaturzone
LED6: Heartbeat-Watchdog
LED7: WLAN/NTP-Status
LED8: Lampenmodus 12h/18h
```

Farblogik muss eindeutig bleiben.

Beispiele:

```text
Rot: Fehler / ungültig / Timeout
Gelb/Orange: Warnung / Zwischenbereich
Grün: OK / gültig / aktiv
Aus: nicht betroffen / Schonfrist / neutral
```

Neue LED-Bedeutungen werden in der Kopf-Dokumentation und in `README.md` dokumentiert.

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
LED-Status setzen
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
motorDriveToward(int curPct, int tgtPct);
motorStop();
motorControlTask(unsigned long now);
tempToSetpoint(float temp);
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
motorDriveToward(curPct, targetPct);
motorStop();
```

Failsafe-Regeln bleiben:

```text
Poti ungültig → Motor aus
Move-Timeout → Motor aus
BME-Fehler → definierter sicherer Zielwert
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
3 bis 5 Kalibrierpunkte
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
LED6 zeigt Zustand
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
Status über LED anzeigen
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
updateModeLed();
updateStatusLed();
```

Regeln:

```text
Lampenstatus zyklisch prüfen
kein permanentes Abfragen ohne Intervall
12h/18h-Modus über Schalter
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
LED-Belegung
```

---

## Fehlerbehandlung

Hardwarefehler werden sichtbar gemacht.

Pflicht bei Sensor-/Hardwarefehlern:

```text
Statusvariable setzen
LED-Zustand setzen
Serial-Diagnose ausgeben
sicheren Zustand herstellen
Reinitialisierung oder sicheren Fallback versuchen, falls passend
```

Keine stillen Fehler.

Beispiele:

```cpp
bmeOK = false;
motorStop();
ledSet(3, C(255, 0, 0));
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
Poti ungültig → Motor aus
Move-Timeout → Motor aus
BME-Fehler → sicherer Zielwert oder Motorstopp nach Logik
```

Heartbeat-Failsafe:

```text
Timeout → LED6 rot
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
LED-Index: uint8_t
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
LEDs
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

Bestehende Libraries:

```cpp
Adafruit_BME680
Wire
NeoPixelBus
Preferences
nvs_flash
WiFi
time
```

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
LED-Diagnose entfernen
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
3. LED-Hilfsfunktionen auslagern
4. BME-Funktionen auslagern
5. Motorfunktionen auslagern
6. Kalibrierung auslagern
7. Heartbeat auslagern
8. WLAN/NTP auslagern
9. Lampensteuerung auslagern
```

Nach jedem Schritt:

```text
Build ausführen
Upload testen, falls Hardware betroffen
Serial-Ausgabe prüfen
LED-Status prüfen
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
LED-Diagnose bleibt erhalten
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

Der vorhandene Code verwendet diese Abschnittsstruktur:

```text
Kopf-Dokumentation
Includes
WLAN-Daten
NTP/Zeit
Pinbelegung
Zeitsteuerung
WLAN-Überwachung
Kalibrierung
PWM
LEDs/Farben
BME680
Heartbeat
Motor/Regelung
Serial-Änderungsausgabe
Relais
Vorwärtsdeklarationen
Setup
Loop
ISR
Poti
Rohwert-Prozent-Umrechnung
Temperatur-Sollwert
Motor
LED-Zonen
Statusausgabe
Kalibrierung/NVS
BME/I2C
Initialisierung
Tasks
WLAN/NTP/Lampe
Sommerzeit
Status-LEDs
```

Diese Struktur bleibt gültig, bis Module ausgelagert werden.

---

## Harte Regel

Eine Änderung ist nur akzeptabel, wenn danach gilt:

```text
Der ESP32 bleibt non-blocking.
Der Motor bleibt failsafe.
Der BME680 bleibt fehlerbehandelt und reinitialisierbar.
Der Heartbeat bleibt wirksam.
Die LEDs bleiben diagnostisch nutzbar.
Die Kalibrierung bleibt erhalten.
Die Secrets bleiben privat.
Das Projekt baut mit PlatformIO.
```
