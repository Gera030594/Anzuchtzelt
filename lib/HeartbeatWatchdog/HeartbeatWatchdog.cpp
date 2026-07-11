#include "HeartbeatWatchdog.h"

#include "BmeSensor.h"
#include "Config.h"
#include "Pins.h"

/************ Heartbeat ************/
static portMUX_TYPE heartbeatMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool hbSeen = false;  // wurde seit Boot/Reset ein Heartbeat empfangen?
unsigned long boot_ms = 0;
volatile unsigned long lastHB_RX_ms = 0;        // Zeit letzte empfangene Flanke
unsigned long lastHB_TX_ms = 0;                 // Zeit letzter gesendeter Puls
unsigned long hbPulseStart_ms = 0;              // Startzeit des Sende-Pulses
unsigned long hbNextResetEligible_ms = 0;       // frühester Zeitpunkt für den nächsten Reset
volatile bool hbTimeoutLatched = false;         // <— NEU: Timeout nur einmal „wirksam“

/************ Relais Spannungsversorgung des Watchdog C3-ESP32******************/
bool relayActive = false;
unsigned long relayStart_ms = 0;

struct HeartbeatSnapshot {
  bool seen;
  unsigned long lastRxMs;
  unsigned long bootMs;
  unsigned long nextResetEligibleMs;
};

static HeartbeatSnapshot getHeartbeatSnapshot() {
  HeartbeatSnapshot snapshot;
  portENTER_CRITICAL(&heartbeatMux);
  snapshot.seen = hbSeen;
  snapshot.lastRxMs = lastHB_RX_ms;
  snapshot.bootMs = boot_ms;
  snapshot.nextResetEligibleMs = hbNextResetEligible_ms;
  portEXIT_CRITICAL(&heartbeatMux);
  return snapshot;
}

static bool timeReached(unsigned long now, unsigned long deadline) {
  return static_cast<int32_t>(now - deadline) >= 0;
}

static void IRAM_ATTR hbISR() {
  portENTER_CRITICAL_ISR(&heartbeatMux);
  hbSeen = true;            // Jetzt wissen wir: echter RX ist da
  lastHB_RX_ms = millis();  // Zeitpunkt der letzten RX-Flanke merken
  hbTimeoutLatched = false;
  hbNextResetEligible_ms = lastHB_RX_ms + HB_TIMEOUT;
  portEXIT_CRITICAL_ISR(&heartbeatMux);
}

void initRelay() {
  pinMode(C3_WD_RELAIS_PIN, OUTPUT);
  digitalWrite(C3_WD_RELAIS_PIN, LOW);  // beim Start immer AUS (LOW)
  Serial.println(F("[RELAY] Watchdog-Reset-Relais initialisiert."));
}

void initHeartbeat() {
  const unsigned long heartbeatBootMs = millis();
  portENTER_CRITICAL(&heartbeatMux);
  boot_ms = heartbeatBootMs;                             // <— Bootzeit merken
  hbNextResetEligible_ms = boot_ms + HB_TIMEOUT;         // erste Schonfrist ab Boot
  hbSeen = false;                                        // Noch kein Heartbeat gesehen
  lastHB_RX_ms = 0;                                      // 0 = unbekannt/nie empfangen
  portEXIT_CRITICAL(&heartbeatMux);

  pinMode(HB_OUT_PIN, OUTPUT);
  digitalWrite(HB_OUT_PIN, LOW);                                     // Heartbeat-Sender-Pin
  pinMode(HB_IN_PIN, INPUT_PULLUP);                                  // Heartbeat-Empfang (mit Pullup)
  attachInterrupt(digitalPinToInterrupt(HB_IN_PIN), hbISR, RISING);  // RX-Flanke mit ISR
  Serial.println(F("[HB] Heartbeat RX/TX initialisiert."));
}

HeartbeatStatus getHeartbeatStatus(unsigned long now) {
  (void)now;
  const HeartbeatSnapshot snapshot = getHeartbeatSnapshot();
  const unsigned long heartbeatNow = millis();
  const unsigned long sinceRef = snapshot.seen ? snapshot.lastRxMs : snapshot.bootMs;
  const unsigned long elapsed = heartbeatNow - sinceRef;
  const bool timedOut = elapsed > HB_TIMEOUT;
  const bool inGrace = !snapshot.seen && elapsed <= HB_TIMEOUT;
  const bool hbOk = snapshot.seen && !timedOut;

  if (hbOk) {
    return HeartbeatStatus::Ok;
  }
  if (inGrace) {
    return HeartbeatStatus::Grace;
  }
  return HeartbeatStatus::Timeout;
}

void heartbeatTask(unsigned long now) {
  // 1) Heartbeat senden – nur, wenn Sensor ok
  if (bmeHealthy(now) && (now - lastHB_TX_ms >= HB_SEND_INTERVAL)) {
    lastHB_TX_ms = now;
    digitalWrite(HB_OUT_PIN, HIGH);
    hbPulseStart_ms = now;
    Serial.println(F("[HB] Sende-Puls (BME OK)"));
  }
  if (hbPulseStart_ms && (now - hbPulseStart_ms >= HB_PULSE_LEN)) {
    digitalWrite(HB_OUT_PIN, LOW);
    hbPulseStart_ms = 0;
  }

  // 2) Überwachen mit Schonfrist nach Boot
  // Wenn noch nie ein HB gesehen wurde, messen wir die "Inaktivität" ab Boot.
  const HeartbeatSnapshot snapshot = getHeartbeatSnapshot();
  const unsigned long heartbeatNow = millis();
  const unsigned long sinceRef = snapshot.seen ? snapshot.lastRxMs : snapshot.bootMs;
  const unsigned long elapsed = heartbeatNow - sinceRef;
  const bool timedOut = elapsed > HB_TIMEOUT;

  // ===== Edge-getriggerter FAILSAFE nur beim Eintreten des Timeouts =====
  if (timedOut) {
    portENTER_CRITICAL(&heartbeatMux);
    const bool heartbeatUnchanged =
        hbSeen == snapshot.seen && lastHB_RX_ms == snapshot.lastRxMs;
    if (heartbeatUnchanged && !hbTimeoutLatched) {
      hbTimeoutLatched = true;  // nur EINMAL pro Timeout
    }
    portEXIT_CRITICAL(&heartbeatMux);
  }

  // Relais: erst nach abgelaufener Schonfrist/Timeout auslösen
  // Nur auslösen, wenn Timeout UND Cooldown abgelaufen
  bool activateRelay = false;
  if (timedOut && !relayActive &&
      timeReached(heartbeatNow, snapshot.nextResetEligibleMs)) {
    portENTER_CRITICAL(&heartbeatMux);
    const bool heartbeatUnchanged =
        hbSeen == snapshot.seen && lastHB_RX_ms == snapshot.lastRxMs;
    const bool resetDeadlineUnchanged =
        hbNextResetEligible_ms == snapshot.nextResetEligibleMs;
    if (heartbeatUnchanged && resetDeadlineUnchanged) {
      hbSeen = false;  // bleibt rot bis echte RX-ISR kommt
      hbNextResetEligible_ms = heartbeatNow + HB_TIMEOUT;
      activateRelay = true;
    }
    portEXIT_CRITICAL(&heartbeatMux);
  }

  if (activateRelay) {
    relayActive = true;
    relayStart_ms = now;
    digitalWrite(C3_WD_RELAIS_PIN, HIGH);
    Serial.println(F("[HB] Timeout → Relais aktiviert (Reset des C3)!"));
  }

  // Relais nach Pulsdauer wieder loslassen (ohne künstliches Zurücksetzen des Timers)
  if (relayActive && (now - relayStart_ms >= RELAY_PULSE_LEN)) {
    digitalWrite(C3_WD_RELAIS_PIN, LOW);
    relayActive = false;
    // KEIN lastHB_RX_ms = now;  // damit LED6 rot bleibt, bis echter RX kommt
    Serial.println(F("[HB] Relais deaktiviert, Überwachung neu gestartet."));
  }
}
