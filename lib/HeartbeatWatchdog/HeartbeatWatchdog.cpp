#include "HeartbeatWatchdog.h"

#include "BmeSensor.h"
#include "Config.h"
#include "LedStatus.h"
#include "Pins.h"

/************ Heartbeat ************/
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
void IRAM_ATTR hbISR() {
  hbSeen = true;            // Jetzt wissen wir: echter RX ist da
  lastHB_RX_ms = millis();  // Zeitpunkt der letzten RX-Flanke merken
  hbTimeoutLatched = false;
  hbNextResetEligible_ms = lastHB_RX_ms + HB_TIMEOUT;
}

void initRelay() {
  pinMode(C3_WD_RELAIS_PIN, OUTPUT);
  digitalWrite(C3_WD_RELAIS_PIN, LOW);  // beim Start immer AUS (LOW)
  Serial.println(F("[RELAY] Watchdog-Reset-Relais initialisiert."));
}

void initHeartbeat() {
  boot_ms = millis();                             // <— Bootzeit merken
  hbNextResetEligible_ms = boot_ms + HB_TIMEOUT;  // erste Schonfrist ab Boot
  pinMode(HB_OUT_PIN, OUTPUT);
  digitalWrite(HB_OUT_PIN, LOW);                                     // Heartbeat-Sender-Pin
  pinMode(HB_IN_PIN, INPUT_PULLUP);                                  // Heartbeat-Empfang (mit Pullup)
  attachInterrupt(digitalPinToInterrupt(HB_IN_PIN), hbISR, RISING);  // RX-Flanke mit ISR
  hbSeen = false;                                                    // Noch kein Heartbeat gesehen
  lastHB_RX_ms = 0;                                                  // 0 = unbekannt/nie empfangen
  Serial.println(F("[HB] Heartbeat RX/TX initialisiert."));
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
  unsigned long sinceRef = hbSeen ? lastHB_RX_ms : boot_ms;
  bool timedOut = (now - sinceRef > HB_TIMEOUT);

  // LED-Logik mit Schonfrist
  bool inGrace = (!hbSeen && (now - boot_ms <= HB_TIMEOUT));
  bool hbOk = (hbSeen && !timedOut);

  if (hbOk) {
    ledSet(6, C(0, 255, 0));  // grün nach echtem RX
  } else if (inGrace) {
    ledSet(6, C(0, 0, 0));  // aus während der Schonfrist
  } else {
    ledSet(6, C(255, 0, 0));  // rot bei/ nach Timeout
  }
  // ===== Edge-getriggerter FAILSAFE nur beim Eintreten des Timeouts =====
  if (timedOut && !hbTimeoutLatched) {
    hbTimeoutLatched = true;  // nur EINMAL pro Timeout
    /*if (potiValid) {
      targetPct = 25;  // Failsafe wegen HB
      Serial.println(F("[HB] TIMEOUT → FAILSAFE target=25% (edge)"));
    }*/
  }
  // Relais: erst nach abgelaufener Schonfrist/Timeout auslösen
  // Nur auslösen, wenn Timeout UND Cooldown abgelaufen
  if (timedOut && !relayActive && now >= hbNextResetEligible_ms) {
    relayActive = true;
    relayStart_ms = now;
    hbSeen = false;  // bleibt rot bis echte RX-ISR kommt
    digitalWrite(C3_WD_RELAIS_PIN, HIGH);
    Serial.println(F("[HB] Timeout → Relais aktiviert (Reset des C3)!"));
    hbNextResetEligible_ms = now + HB_TIMEOUT;  // nächste Chance erst wieder in 20 s
  }


  // Relais nach Pulsdauer wieder loslassen (ohne künstliches Zurücksetzen des Timers)
  if (relayActive && (now - relayStart_ms >= RELAY_PULSE_LEN)) {
    digitalWrite(C3_WD_RELAIS_PIN, LOW);
    relayActive = false;
    // KEIN lastHB_RX_ms = now;  // damit LED6 rot bleibt, bis echter RX kommt
    Serial.println(F("[HB] Relais deaktiviert, Überwachung neu gestartet."));
  }
}
