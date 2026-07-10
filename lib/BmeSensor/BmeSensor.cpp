#include "BmeSensor.h"

#include <Adafruit_BME680.h>
#include <Wire.h>
#include <math.h>

#include "Config.h"
#include "MotorControl.h"
#include "Pins.h"
#include "PotiFeedback.h"

/************ BME680 ************/
Adafruit_BME680 bme;                         // Sensorinstanz
float T = NAN, RH = NAN;                     // Messwerte
bool bmeOK = false;                          // letzter Sensorstatus ok?
unsigned long tBME = 0;                      // Timer BME

// ---- BME Watchdog / Auto-Recover ----
uint8_t bmeAddr = 0;                      // zuletzt valide I2C-Adresse (0x76/0x77)
unsigned long lastBMEGood_ms = 0;         // Zeitstempel letzte GUT- Messung
unsigned long lastBMEAttempt_ms = 0;      // letzter Reinit-Versuch
int bmeBadStreak = 0;                     // aufeinanderfolgende Fehlmessungen
bool bmeReinitPending = false;
unsigned long bmeReinitStart_ms = 0;
static bool bmeDisplayError = true;
static bool bmeHumidityDisplayValid = false;

static void setBmeDisplayError(bool error) {
  bmeDisplayError = error;
}

static bool isBmeDisplayValueValid(float value) {
  return !(isnan(value) || value < 0.0 || value > 100.0);
}

static void setHumidityDisplayValue(float rh) {
  bmeHumidityDisplayValid = isBmeDisplayValueValid(rh);
}

bool hasBmeDisplayError() {
  return bmeDisplayError;
}

bool hasValidHumidityForDisplay() {
  return bmeHumidityDisplayValid;
}

bool tryGetBmeTemperatureC(float& value) {
  if (!isBmeDisplayValueValid(T)) return false;
  value = T;
  return true;
}

bool tryGetBmeHumidityPct(float& value) {
  if (!hasValidHumidityForDisplay()) return false;
  value = RH;
  return true;
}

void bmeConfigure() {
  bme.setTemperatureOversampling(BME680_OS_1X);
  bme.setHumidityOversampling(BME680_OS_1X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_0);
  bme.setGasHeater(0, 0);
}

bool i2cDevicePresent(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);  // 0 == ACK
}

void bmeTryInitNow() {
  lastBMEAttempt_ms = millis();
  bmeReinitStart_ms = millis();
  bmeReinitPending = true;

  // I2C wieder aktivieren
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);  // 100 kHz I2C-Standardtakt

  Serial.println(F("[BME] Reinit vorbereitet, warte auf Stabilisierung..."));
}

void bmeService(unsigned long now) {
  // Wenn defekt, periodisch Reinit starten
  if (!bmeOK && (now - lastBMEAttempt_ms >= BME_RETRY_MS) && !bmeReinitPending) {
    bmeTryInitNow();
  }

  // Falls Reinit läuft → nach 15 ms bme.begin() ausführen
  if (bmeReinitPending && (now - bmeReinitStart_ms >= BME_REINIT_WAIT)) {
    bmeReinitPending = false;  // abgeschlossen

    uint8_t foundAddr = 0;
    if (i2cDevicePresent(0x76)) foundAddr = 0x76;
    else if (i2cDevicePresent(0x77)) foundAddr = 0x77;

    if (foundAddr == 0) {
      bmeOK = false;
      Serial.println(F("[BME] not present (no ACK on 0x76/0x77)"));
      return;
    }

    if (bme.begin(foundAddr, true)) {
      bmeAddr = foundAddr;
      bmeConfigure();
      bmeOK = true;
      bmeBadStreak = 0;
      lastBMEGood_ms = now;
      Serial.print(F("[BME] reinit OK @0x"));
      Serial.println(bmeAddr, HEX);
    } else {
      bmeOK = false;
      Serial.print(F("[BME] begin() failed @0x"));
      Serial.println(foundAddr, HEX);
    }
  }
}

void bmeInit() {
  if (bme.begin(0x76)) {  // viele Breakouts nutzen 0x76 (SDO=GND)
    bmeAddr = 0x76;
    bmeOK = true;
  } else if (bme.begin(0x77)) {  // Alternative (SDO=VDDIO)
    bmeAddr = 0x77;
    bmeOK = true;
  } else {
    bmeOK = false;  // kein BME bei 0x76/0x77 gefunden
  }

  if (!bmeOK) {
    setBmeDisplayError(true);  // Sensorfehler signalisieren
    setHumidityDisplayValue(NAN);
    Serial.println(F("BME680 nicht gefunden (0x76/0x77)."));
  } else {
    Serial.print(F("BME680 gefunden @ 0x"));
    Serial.println(bmeAddr, HEX);

    // Jetzt konfigurieren:
    bme.setTemperatureOversampling(BME680_OS_1X);
    bme.setHumidityOversampling(BME680_OS_1X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_0);
    bme.setGasHeater(0, 0);
  }
}

void bmeInitialCheck() {
  // 1) BME prüfen
  if (!bmeOK || !bme.performReading()) {                                   // Wenn BME nicht ok / keine Lesung
    bmeOK = false;                                                         // Sensorfehler markieren
    setBmeDisplayError(true);                                              // BME-Fehler anzeigen
    setHumidityDisplayValue(NAN);
    potiRawNow = readPotiRaw();                                            // Poti lesen
    potiValid = (potiRawNow > POTI_MIN_RAW && potiRawNow < POTI_MAX_RAW);  // Plausibel?
    if (potiValid) {                                                       // Poti ok -> auf 25% fahren (Failsafe)
      setMotorTargetPct(25);                                               // Ziel 25%
      moveActive = true;
      moveStart_ms = millis();  // Bewegung starten
    }
  } else {  // BME ok -> Messung übernehmen
    T = bme.temperature;
    RH = bme.humidity;  // Temperatur / Feuchte speichern

    bool tBad = (isnan(T) || T < 0.0 || T > 100.0);
    bool hBad = (isnan(RH) || RH < 0.0 || RH > 100.0);
    setBmeDisplayError(!bmeOK || tBad || hBad);
    setHumidityDisplayValue(hBad ? NAN : RH);

    // Temperatur prüfen
    if (tBad) {                    // Temperatur ungültig?
      potiRawNow = readPotiRaw();  // Poti prüfen
      potiValid = (potiRawNow > POTI_MIN_RAW && potiRawNow < POTI_MAX_RAW);
      if (potiValid) {
        setMotorTargetPct(25);
        moveActive = true;
        moveStart_ms = millis();
      }                         // Failsafe
    }
    if (!tBad && !hBad) {
      bmeBadStreak = 0;
      lastBMEGood_ms = millis();
    }
  }
}

void bme680Task(unsigned long now) {
  if (now - tBME < BME_INTERVAL) return;  // Noch nicht Zeit
  tBME = now;

  if (bme.performReading()) {
    T = bme.temperature;
    RH = bme.humidity;

    bool tBad = (isnan(T) || T < 0.0 || T > 100.0);
    bool hBad = (isnan(RH) || RH < 0.0 || RH > 100.0);
    setHumidityDisplayValue(hBad ? NAN : RH);

    if (!tBad && !hBad) {
      bmeOK = true;
      bmeBadStreak = 0;
      lastBMEGood_ms = now;
      setBmeDisplayError(false);
      int newTarget = tempToSetpoint(T);
      if (newTarget != getMotorTargetPct()) setMotorTargetPct(newTarget);
    } else {
      // Fehler/Failsafe
      bmeBadStreak++;
      if (bmeBadStreak >= BME_BAD_LIMIT) bmeOK = false;
      setBmeDisplayError(true);
      if (tBad) {
        if (potiValid) {
          setMotorTargetPct(25);
          moveActive = true;
          moveStart_ms = now;
        }
      } else {
        int newTarget = tempToSetpoint(T);
        if (newTarget != getMotorTargetPct()) setMotorTargetPct(newTarget);
      }
    }
  } else {
    // Lesefehler (BME antwortet nicht)
    bmeBadStreak++;
    if (bmeBadStreak >= BME_BAD_LIMIT) bmeOK = false;

    setBmeDisplayError(true);  // Haupt-Sensorfehler
    setHumidityDisplayValue(NAN);

    // Failsafe-Logik bleibt gleich:
    if (potiValid) {
      setMotorTargetPct(25);
      moveActive = true;
      moveStart_ms = now;
    }
  }
  // 🔹 Konsistente Fehlerbehandlung (egal welcher Fehlerpfad)
  if (!bmeOK) {
    setBmeDisplayError(true);
    bmeService(now);
  }
}

bool bmeHealthy(unsigned long now) {
  const unsigned long FRESH_WINDOW = BME_INTERVAL + 2000UL;  // 60s + 2s Toleranz
  return bmeOK && (now - lastBMEGood_ms <= FRESH_WINDOW);
}
