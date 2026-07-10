#include "LampControl.h"

#include <time.h>

#include "Config.h"
#include "Pins.h"

bool relayState = false;              // Aktueller Status des Relais speichern
unsigned long lastLampCheck = 0;      // Zeitpunkt der letzten Lampenprüfung speichern
bool lampMode12h = false;             // Lampenmodus (true = 12h, false = 18h)
static bool modePinConfigured = false;
static bool modeRawHigh = false;
static bool modeStableHigh = false;
static bool modeDebounceInitialized = false;
static unsigned long modeRawChanged_ms = 0;

static void ensureModePinConfigured() {
  if (!modePinConfigured) {
    pinMode(modePin, INPUT_PULLDOWN);
    modePinConfigured = true;
  }
}

static bool readModePinRawHigh() {
  ensureModePinConfigured();
  return digitalRead(modePin) == HIGH;
}

static void initGrowPhaseDebounce(unsigned long now) {
  modeRawHigh = readModePinRawHigh();
  modeStableHigh = modeRawHigh;
  modeRawChanged_ms = now;
  modeDebounceInitialized = true;
}

void updateGrowPhase(unsigned long now) {
  if (!modeDebounceInitialized) {
    initGrowPhaseDebounce(now);
    return;
  }

  bool rawHigh = readModePinRawHigh();

  if (rawHigh != modeRawHigh) {
    modeRawHigh = rawHigh;
    modeRawChanged_ms = now;
    return;
  }

  if (modeStableHigh != modeRawHigh && (now - modeRawChanged_ms >= MODE_DEBOUNCE_MS)) {
    modeStableHigh = modeRawHigh;
  }
}

GrowPhase getGrowPhase() {
  ensureModePinConfigured();
  return modeStableHigh ? GrowPhase::Flowering : GrowPhase::Vegetation;
}

bool isLampRelayOn() {
  return relayState;
}

void handleLamp(unsigned long now) {
  updateGrowPhase(now);

  if (now - lastLampCheck >= lampCheckInterval) {
    lastLampCheck = now;
    checkLampState();  // Lampenstatus prüfen
  }
}

void initHardwareLampensteuerung() {
  pinMode(relayPin, OUTPUT);                              // Relais-Pin als Ausgang definieren
  digitalWrite(relayPin, RELAY_ACTIVE_LOW ? HIGH : LOW);  // AUS-Zustand
  ensureModePinConfigured();                              // Pin für den Modus-Schalter mit internem Pulldown aktivieren
  initGrowPhaseDebounce(millis());
  Serial.println("Hardware initialisiert.");
}

void checkLampState() {
  updateGrowPhase(millis());

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Keine gültige Zeit verfügbar.");
    return;
  }

  // Schalterzustand auslesen
  lampMode12h = (getGrowPhase() == GrowPhase::Flowering);  // HIGH = 12h Modus, LOW = 18h Modus

  int startHour, endHour;

  if (lampMode12h) {
    startHour = 8;  // z.B. 8:00 Uhr an
    endHour = 20;   // 20:00 Uhr aus → 12 Stunden
  } else {
    startHour = 4;  // 4:00 Uhr an
    endHour = 22;   // 22:00 Uhr aus → 18 Stunden
  }

  int currentHour = timeinfo.tm_hour;
  bool newRelayState = (currentHour >= startHour && currentHour < endHour);

  if (newRelayState != relayState) {
    relayState = newRelayState;
    digitalWrite(relayPin, RELAY_ACTIVE_LOW ? !relayState : relayState);
    Serial.printf("💡 Lampe %s (%dh-Modus, Pin-Status: %s) um %02d:%02d Uhr\n",
                  relayState ? "EIN" : "AUS",
                  lampMode12h ? 12 : 18,
                  (relayState ? (RELAY_ACTIVE_LOW ? "LOW" : "HIGH") : (RELAY_ACTIVE_LOW ? "HIGH" : "LOW")),
                  timeinfo.tm_hour, timeinfo.tm_min);
  }
}
