#pragma once

// ---------------- Pinbelegung ----------------
const int relayPin = 19;              // GPIO-Pin für das Lampen-Relais definieren
const bool RELAY_ACTIVE_LOW = false;  // Relais ist aktiv bei HIGH-Pegel
const int modePin = 18;               // Pin, an dem der Schalter hängt (Beispiel: GPIO18)
const int I2C_SDA_PIN = 21;           // I2C SDA für BME680
const int I2C_SCL_PIN = 22;           // I2C SCL für BME680

#define LED_STATUS 7                            // z. B. LED7 = WLAN/NTP-Statusanzeige
#define LED_MODE 8                              // LED8 zeigt 12h/18h Modus

/************ Pinbelegung (frei gewählt) ************/
#define WS2812_PIN 16        // Datenpin WS2812
#define NUM_LEDS 10          // Anzahl LED0..LED9
#define L298N_IN1 26         // Richtung A
#define L298N_IN2 27         // Richtung B
#define L298N_ENA 25         // PWM-Enable (LEDC)
#define POTI_ADC_PIN 34      // Rückmeldung Poti (ADC1_CH6)
#define HB_IN_PIN 32         // Heartbeat Eingang (Interrupt)
#define HB_OUT_PIN 33        // Heartbeat Ausgang (Puls)
#define C3_WD_RELAIS_PIN 23  // Relais-Ausgang, HIGH = aktiviert
