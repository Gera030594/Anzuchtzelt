#include <Arduino.h>
#include <Wire.h>

#include "BmeSensor.h"
#include "Calibration.h"
#include "HeartbeatWatchdog.h"
#include "LampControl.h"
#include "LedStatus.h"
#include "MotorControl.h"
#include "Pins.h"
#include "PotiFeedback.h"
#include "SerialCommands.h"
#include "WifiTime.h"

/************ Allgemeine Initialisierung ************/
void initGeneral() {
  Serial.begin(115200);  // Serielle Schnittstelle starten
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);  // I²C Start (SDA=21, SCL=22)
  Wire.setClock(100000);                 // 100 kHz I2C-Standardtakt
}

void setup() {
  initGeneral();
  initRelay();
  initPoti();
  initLEDs();
  initMotorControl();
  initHeartbeat();
  bmeInit();
  bmeInitialCheck();
  openNVS();
  calInitLoad();                  // liest n/raw/pct aus NVS (calInitLoad() darf KEIN prefs.begin() mehr aufrufen!)
  initHardwareLampensteuerung();  // Initialisiere Pins, serielle Schnittstelle, Status
  updateModeLed();
  connectWiFi();      // WLAN-Verbindung aufbauen
  checkLampState();   // Initialer Zustand (z. B. Relais aus)
  updateStatusLed();
  Serial.println(F("[SYS] Setup abgeschlossen."));
}

void loop() {
  const unsigned long now = millis();  // aktuelle Zeit zwischenspeichern

  readPotiTask(now);      // Poti einlesen und prüfen// --- Einmaliges Poti-Read & Gültigkeit für diese Loop-Runde ---
  bme680Task(now);        // BME680 periodisch abfragen
  heartbeatTask(now);     // Heartbeat Senden + Überwachung
  motorControlTask(now);  // Motor ansteuern (non-blocking)
  handleWiFi(now);        // WLAN prüfen und ggf. neu verbinden
  handleNTP(now);         // NTP-Zeit regelmäßig synchronisieren
  handleLamp(now);        // Lampenstatus prüfen und schalten
  updateStatusLed();
  updateModeLed();
  ledUpdateTask();         // WS2812 aktualisieren
  serialCommandTask(now);  // Serielle Eingaben & Debug
}
