#include "SerialCommands.h"

#include <math.h>
#include <stdlib.h>

#include "BmeSensor.h"
#include "Calibration.h"
#include "Config.h"
#include "HeartbeatWatchdog.h"
#include "MotorControl.h"
#include "PotiFeedback.h"

/************ Serielle Änderungs-Ausgabe ************/
int lastPct = -1000;                         // für Änderungslog
int lastTargetPct = -1000;                   // für Änderungslog
int lastPotiRaw = -10000;                    // für Änderungslog
int lastT10 = -32768;                        // Temperatur *10 letztmalig ausgegeben
int lastRH10 = -32768;                       // rF *10 letztmalig ausgegeben
void changeLogMaybe(unsigned long now) {
  static int lastPctOut = -1000;       // zuletzt ausgegebene %-Position
  static unsigned long lastLogMs = 0;  // Rate-Limit
  if (now - lastLogMs < 200) return;   // max. 5 Zeilen/s

  int T10 = isnan(T) ? 32767 : (int)roundf(T * 10.0f);
  int RH10 = isnan(RH) ? 32767 : (int)roundf(RH * 10.0f);
  int pctNow = rawToPercent(potiRawNow);

  bool pctChanged = (lastPctOut == -1000) || (abs(pctNow - lastPctOut) >= DEAD_BAND_PCT);
  bool tChanged = (T10 != lastT10);
  bool hChanged = (RH10 != lastRH10);
  int currentTargetPct = getMotorTargetPct();
  bool setChanged = (currentTargetPct != lastTargetPct);

  if (!(pctChanged || tChanged || hChanged || setChanged)) return;

  // Update "last"-Werte erst NACH der Prüfung
  lastPctOut = pctNow;
  lastT10 = T10;
  lastRH10 = RH10;
  lastTargetPct = currentTargetPct;
  lastPotiRaw = potiRawNow;
  lastLogMs = now;

  Serial.print("T=");
  if (T10 == 32767) Serial.print("NaN");
  else {
    Serial.print(T10 / 10.0f, 1);
    Serial.print("C");
  }
  Serial.print(" | RH=");
  if (RH10 == 32767) Serial.print("NaN%");
  else {
    Serial.print(RH10 / 10.0f, 1);
    Serial.print("%");
  }
  Serial.print(" | PotiRaw=");
  Serial.print(potiRawNow);
  Serial.print(" | Ist%=");
  Serial.print(pctNow);
  Serial.print(" | Soll%=");
  Serial.println(currentTargetPct);
}

void printStatus() {
  int pctNow = rawToPercent(potiRawNow);  // Ist-% bestimmen
  Serial.println(F("---- STATUS ----"));
  Serial.print(F("BME680: "));
  Serial.println(bmeOK ? "OK" : "FEHLER");
  Serial.print(F("Temperatur: "));
  if (isnan(T)) Serial.println("NaN");
  else {
    Serial.print(T, 1);
    Serial.println(" C");
  }
  Serial.print(F("Luftfeuchte: "));
  if (isnan(RH)) Serial.println("NaN");
  else {
    Serial.print(RH, 1);
    Serial.println(" %");
  }
  Serial.print(F("Poti RAW: "));
  Serial.println(potiRawNow);
  Serial.print(F("Ist%: "));
  Serial.println(pctNow);
  Serial.print(F("Soll%: "));
  Serial.println(getMotorTargetPct());
  Serial.print(F("Heartbeat RX vor (ms): "));
  Serial.println(millis() - lastHB_RX_ms);
  Serial.print(F("Motor aktiv: "));
  Serial.println(moveActive ? "JA" : "NEIN");
  Serial.println(F("----------------"));
}

void serialCommandTask(unsigned long now) {
  changeLogMaybe(now);

  if (!Serial.available()) return;

  char c = Serial.read();
  if (c == 'r' || c == 'R') { printStatus(); }

  // --- Kalibrierbefehle ---
  switch (c) {
    case 'K':
    case 'k': calEnter(); break;  // Assistent starten
    case 'Q':
    case 'q': calExit(); break;  // Assistent verlassen
    case 'L':
    case 'l': calList(); break;  // Punkte anzeigen
    case 'S':
    case 's': calSave(); break;  // dauerhaft speichern
    case 'V':
    case 'v':
      calInitLoad();
      break;
    case 'D':
    case 'd':
      {  // DIAGNOSE: Rohinfos aus NVS anzeigen
        uint8_t n = prefs.getUChar("n", 0xFF);
        size_t lenRaw = prefs.getBytesLength("raw");
        size_t lenPct = prefs.getBytesLength("pct");
        Serial.println(F("=== NVS-DIAG ==="));
        Serial.print(F("n="));
        Serial.println(n == 0xFF ? -1 : (int)n);
        Serial.print(F("raw.len="));
        Serial.println(lenRaw);
        Serial.print(F("pct.len="));
        Serial.println(lenPct);
        Serial.println(F("============="));
        break;
      }
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
      calCaptureSlot((uint8_t)(c - '1'));  // Slot 0..4 aufnehmen
      break;
  }
}
