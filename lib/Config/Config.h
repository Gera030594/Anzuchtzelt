#pragma once

#include <Arduino.h>

// ---------------- Zeitsteuerung ----------------
const unsigned long lampCheckInterval = 300000;  // Lampenprüfung alle 5 Minuten (300.000 ms)
const unsigned long ntpSyncInterval = 86400000;  // NTP-Sync einmal pro Tag (24h)
const unsigned long retrySyncInterval = 30000;   // Intervall für wiederholte NTP-Versuche (30 Sekunden)

// ---------------- WLAN-Überwachung ----------------
const unsigned long wifiCheckInterval = 10000;  // WLAN alle 10 Sekunden überprüfen

// ---- Kalibrierung: 3..5 Punkte (Slots sind an % gebunden) ----
#define CAL_MAX_PTS 5

/************ PWM (LEDC) ESP32 ************/
#define PWM_FREQ 10000   // 10 kHz PWM für leisen Motorbetrieb
#define PWM_RES_BITS 10  // 10 Bit Auflösung (0..1023)
#define PWM_CHANNEL 0    // LEDC-Kanal 0

/************ Lampenmodus / GrowPhase ************/
const unsigned long MODE_DEBOUNCE_MS = 200UL;  // Modus-Schalter non-blocking entprellen

/************ BME680 ************/
const unsigned long BME_INTERVAL = 10000UL;  // 10 s
const unsigned long BME_RETRY_MS = 3000;     // alle 3 s Reinit versuchen, wenn defekt
const int BME_BAD_LIMIT = 3;                 // ab 3x Fehler -> „defekt“-Zustand
const unsigned long BME_REINIT_WAIT = 15;    // 15 ms Wartezeit statt delay()

/************ Heartbeat ************/
const unsigned long HB_SEND_INTERVAL = 1000UL;  // alle 1 s senden
const unsigned long HB_TIMEOUT = 20000UL;       // 20 s ohne RX -> Timeout
const unsigned long HB_PULSE_LEN = 500UL;       // Pulsbreite 500 ms

/************ Motor / Regelung ************/
const int POTI_MIN_RAW = 0;                 // Plausibel-Grenzen Poti-Rohwert (ADC)
const int POTI_MAX_RAW = 5000;               // (4095 ist 12 Bit-Max; etwas Luft)
const unsigned long POTI_RECOVERY_STABLE_MS = 2000UL;  // Poti muss nach Fehler 2 s ununterbrochen gültig sein
const int DEAD_BAND_PCT = 2;                 // Totband ±2 % verhindert Zittern
const int MIN_DUTY = 140;                    // Mindest-PWM zum Überwinden der Haftreibung (0..1023)
const int MAX_DUTY = 900;                    // Max-PWM, Reserven lassen
const unsigned long MOVE_TIMEOUT = 10000UL;  // 10 s für Bewegung

/************ Relais Spannungsversorgung des Watchdog C3-ESP32******************/
const unsigned long RELAY_PULSE_LEN = 1000UL;  // 1 Sekunde aktiv
