#include "Calibration.h"

#include <nvs_flash.h>

#include "Config.h"
#include "PotiFeedback.h"

Preferences prefs;

// Slots entsprechen festen Prozentzielen:
const int CAL_SLOT_PCT[CAL_MAX_PTS] = { 0, 25, 50, 75, 100 };

int calN = 0;                           // Anzahl derzeit aktiver Punkte (>=2)
int calRaw[CAL_MAX_PTS] = { 0 };        // Roh-ADC je Punkt (sortiert nach Raw)
int calPct[CAL_MAX_PTS] = { 0 };        // Ziel-% je Punkt (mit calRaw korrespond.)
bool calSlotUsed[CAL_MAX_PTS] = { 0 };  // Slot-belegt-Flags
bool calMode = false;                   // im Kalibrierassistent?

int getCalibrationPointCount() {
  return calN;
}

bool isCalibrationModeActive() {
  return calMode;
}

int rawToPercent(int raw) {
  // Falls keine valide LUT: Fallback auf 2-Punkt-Linear aus deinen Plausibilitätsgrenzen
  if (calN < 2) {
    if (raw < POTI_MIN_RAW) raw = POTI_MIN_RAW;
    if (raw > POTI_MAX_RAW) raw = POTI_MAX_RAW;
    long span = (long)POTI_MAX_RAW - POTI_MIN_RAW;
    long rel = (long)raw - POTI_MIN_RAW;
    int pct = (int)((rel * 100L) / span);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
  }

  // Clamping auf LUT-Randpunkte
  if (raw <= calRaw[0]) return calPct[0];
  if (raw >= calRaw[calN - 1]) return calPct[calN - 1];

  // Segment finden und linear interpolieren
  for (int i = 0; i < calN - 1; ++i) {
    int x0 = calRaw[i], x1 = calRaw[i + 1];
    if (raw >= x0 && raw <= x1) {
      int y0 = calPct[i], y1 = calPct[i + 1];
      float t = (float)(raw - x0) / (float)(x1 - x0);
      int pct = (int)roundf(y0 + t * (y1 - y0));
      if (pct < 0) pct = 0;
      if (pct > 100) pct = 100;
      return pct;
    }
  }
  return 0;  // sollte nie passieren
}

void calInitLoad() {
  uint8_t n = prefs.getUChar("n", 0xFF);
  size_t lenRaw = prefs.getBytesLength("raw");
  size_t lenPct = prefs.getBytesLength("pct");

  bool ok = (n >= 2 && n <= CAL_MAX_PTS && lenRaw >= n * sizeof(calRaw[0]) && lenPct >= n * sizeof(calPct[0]));

  if (ok) {
    prefs.getBytes("raw", calRaw, n * sizeof(calRaw[0]));
    prefs.getBytes("pct", calPct, n * sizeof(calPct[0]));
    calN = n;
    Serial.print(F("[CAL] LUT aus NVS geladen. N="));
    Serial.println(calN);
    calList();
    return;
  }

  calN = 2;
  calRaw[0] = POTI_MIN_RAW;
  calPct[0] = 0;
  calRaw[1] = POTI_MAX_RAW;
  calPct[1] = 100;
  Serial.println(F("[CAL] Keine valide LUT im NVS – 2-Punkt-Default."));
}

void calSave() {
  if (calN < 2) {
    Serial.println(F("[CAL] Speichern abgebrochen: calN < 2"));
    return;
  }

  prefs.putUChar("n", (uint8_t)calN);
  prefs.putBytes("raw", calRaw, calN * sizeof(calRaw[0]));
  prefs.putBytes("pct", calPct, calN * sizeof(calPct[0]));


  Serial.print(F("[CAL] Gespeichert. N="));
  Serial.println(calN);

  // direkt Reload zum Verifizieren:
}

void calList() {
  Serial.print(F("[CAL] Punkte (N="));
  Serial.print(calN);
  Serial.println(F("):"));
  for (int i = 0; i < calN; ++i) {
    Serial.print(F("  #"));
    Serial.print(i);
    Serial.print(F(": raw="));
    Serial.print(calRaw[i]);
    Serial.print(F(" -> "));
    Serial.print(calPct[i]);
    Serial.println(F(" %"));
  }
  // Slots (für 5-Punkt-Guided):
  Serial.println(F("[CAL] Slots (Ziel-% je Taste 1..5):"));
  for (int s = 0; s < CAL_MAX_PTS; ++s) {
    Serial.print(F("  Taste "));
    Serial.print(s + 1);
    Serial.print(F(" -> "));
    Serial.print(CAL_SLOT_PCT[s]);
    Serial.print(F("% : "));
    Serial.println(calSlotUsed[s] ? F("gesetzt") : F("leer"));
  }
}

void calEnter() {
  calMode = true;
  Serial.println(F("\n=== KALIBRIER-ASSISTENT ==="));
  Serial.println(F("Drehe auf FU ≈ 0% (0 Hz)  -> Taste '1'"));
  Serial.println(F("Drehe auf FU ≈25% (z.B. 12.5 Hz) -> '2'"));
  Serial.println(F("Drehe auf FU ≈50% (25 Hz) -> '3'"));
  Serial.println(F("Drehe auf FU ≈75% (37.5 Hz) -> '4'"));
  Serial.println(F("Drehe auf FU ≈100% (50 Hz) -> '5'"));
  Serial.println(F("Mit 'L' anzeigen, 'S' speichern, 'Q' beenden.\n"));
}

void calExit() {
  calMode = false;
  // Slots -> sortierte LUT neu aufbauen (falls nicht gespeichert, bleibt nur im RAM)
  calRebuildSortedFromSlots();
  Serial.println(F("[CAL] Assistent beendet."));
  calList();
}

void calCaptureSlot(uint8_t slotIdx) {
  if (slotIdx >= CAL_MAX_PTS) return;
  // Sofortiger Rohwert (Median ohne EMA, damit die Verzögerung die Messung nicht verfälscht)
  int raw = readPotiRawInstant();
  calSlotUsed[slotIdx] = true;

  // In temporäre Sammlung schreiben
  // (Wir bauen später eine sortierte LUT daraus)
  // Wir nutzen calRaw/calPct hier als „Arbeitsbereich“: Slot an Ziel-% binden
  // Beim Rebuild sortieren wir dann nach RAW.
  // Für den Moment tragen wir ihn direkt ein:
  // Achtung: In der finalen LUT darf die Reihenfolge beliebig sein – wird sortiert.
  // Wir speichern einfach:
  //   calPct == Ziel-% aus CAL_SLOT_PCT
  //   calRaw == gemessener Rohwert
  // Später calRebuildSortedFromSlots() konsolidiert und sortiert.
  // (Mehrfaches Setzen desselben Slots überschreibt den Wert.)
  // Um es einfach zu halten, packen wir Slotwerte temporär in die oberen Indizes:
  // → wir nutzen hier dasselbe Array, Rebuild macht's sauber.

  // Finde/ersetze Eintrag mit gleicher Ziel-% (falls vorhanden), sonst hänge an.
  bool replaced = false;
  for (int i = 0; i < calN; i++) {
    if (calPct[i] == CAL_SLOT_PCT[slotIdx]) {
      calRaw[i] = raw;
      replaced = true;
      break;
    }
  }
  if (!replaced) {
    if (calN < CAL_MAX_PTS) {
      calRaw[calN] = raw;
      calPct[calN] = CAL_SLOT_PCT[slotIdx];
      calN++;
    } else {
      // Wenn schon 5 drin sind und wir ersetzen wollten,
      // wird oben bereits ersetzt – hier also nichts tun.
    }
  }

  calRebuildSortedFromSlots();
  Serial.print(F("[CAL] Slot "));
  Serial.print(slotIdx + 1);
  Serial.print(F(" ("));
  Serial.print(CAL_SLOT_PCT[slotIdx]);
  Serial.print(F("%) = raw "));
  Serial.println(raw);
}

void calRebuildSortedFromSlots() {
  // 1) Nur Einträge behalten, deren Ziel-% einem belegten Slot entsprechen (Robustheit)
  int tmpN = 0;
  int tmpRaw[CAL_MAX_PTS], tmpPct[CAL_MAX_PTS];

  for (int s = 0; s < CAL_MAX_PTS; ++s) {
    // Ist dieser Ziel-% in calPct vorhanden UND Slot belegt?
    bool have = false;
    if (calSlotUsed[s]) {
      for (int i = 0; i < calN; i++) {
        if (calPct[i] == CAL_SLOT_PCT[s]) {
          tmpRaw[tmpN] = calRaw[i];
          tmpPct[tmpN] = calPct[i];
          tmpN++;
          have = true;
          break;
        }
      }
      // Falls Slot belegt aber kein passender Eintrag (sollte nicht passieren) – ignorieren
    }
  }

  // Mind. 2 Punkte brauchen wir; sonst fallback auf Plausibilitätslinie
  if (tmpN < 2) {
    Serial.println(F("[CAL] <2 neue Punkte – bestehende LUT bleibt unverändert."));
    return;  // WICHTIG: Nichts überschreiben!
  }          //Damit verhinderst du, dass du dir eine bereits gelernte LUT versehentlich wieder durch den 2-Punkt-Fallback kaputt machst, wenn du den Assistenten ohne genügend neue Punkte verlässt.


  // 2) Nach RAW sortieren (Insertion Sort bei kleiner N ok)
  for (int i = 1; i < tmpN; i++) {
    int r = tmpRaw[i], p = tmpPct[i], j = i - 1;
    while (j >= 0 && tmpRaw[j] > r) {
      tmpRaw[j + 1] = tmpRaw[j];
      tmpPct[j + 1] = tmpPct[j];
      j--;
    }
    tmpRaw[j + 1] = r;
    tmpPct[j + 1] = p;
  }

  // 3) In finale LUT kopieren
  calN = tmpN;
  for (int i = 0; i < calN; i++) {
    calRaw[i] = tmpRaw[i];
    calPct[i] = tmpPct[i];
  }
}

void openNVS() {
  // Versuch, Namespace "cal" zu öffnen
  if (!prefs.begin("cal", false)) {
    Serial.println(F("[CAL] NVS open FAILED, versuche Reparatur..."));

    // NVS initialisieren
    esp_err_t err = nvs_flash_init();

    // Wenn keine freien Seiten oder neue Version → komplett löschen
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      Serial.println(F("[CAL] NVS beschädigt – Partition wird gelöscht und neu initialisiert."));
      nvs_flash_erase();  // Alles löschen
      nvs_flash_init();   // Neu initialisieren
    }

    // Zweiter Versuch, Namespace erneut zu öffnen
    if (!prefs.begin("cal", false)) {
      Serial.println(F("[CAL] NVS open FAILED (nach Reparaturversuch)."));
    } else {
      Serial.println(F("[CAL] NVS open OK (after repair)"));
    }
  } else {
    Serial.println(F("[CAL] NVS open OK"));
  }
}
