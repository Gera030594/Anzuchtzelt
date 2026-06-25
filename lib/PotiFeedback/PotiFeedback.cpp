#include "PotiFeedback.h"

#include "Config.h"
#include "Pins.h"

int potiRawNow = 0;                          // aktueller ADC
bool potiValid = false;                      // gültige Rückmeldung?

/************ Poti lesen (roh) ************/
#define EMA_SHIFT 4  // 1/16 -> stärker glätten als 1/8
int readPotiRaw() {
  // 5 schnelle Messungen, median statt mean (robust gg. Spikes)
  int v[5];
  v[0] = analogRead(POTI_ADC_PIN);
  v[1] = analogRead(POTI_ADC_PIN);
  v[2] = analogRead(POTI_ADC_PIN);
  v[3] = analogRead(POTI_ADC_PIN);
  v[4] = analogRead(POTI_ADC_PIN);
  // tiny sort for 5 elements (insertion sort)
  for (int i = 1; i < 5; i++) {
    int k = v[i], j = i - 1;
    while (j >= 0 && v[j] > k) {
      v[j + 1] = v[j];
      j--;
    }
    v[j + 1] = k;
  }
  int med = v[2];  // Median

  static bool inited = false;
  static int ema = 0;
  if (!inited) {
    ema = med;
    inited = true;
  } else {
    ema += (med - ema) >> EMA_SHIFT;
  }            // α = 1/16
  return ema;  // 0..4095, geglättet
}

int readPotiRawInstant() {
  // mehrfach lesen + Median, KEINE EMA (direkte Momentaufnahme)
  const int N = 7;
  int v[N];
  for (int i = 0; i < N; i++) v[i] = analogRead(POTI_ADC_PIN);
  // sort
  for (int i = 1; i < N; i++) {
    int k = v[i], j = i - 1;
    while (j >= 0 && v[j] > k) {
      v[j + 1] = v[j];
      j--;
    }
    v[j + 1] = k;
  }
  return v[N / 2];
}

void initPoti() {
  analogReadResolution(12);
  /*
analogReadResolution(12):
Stellt die Ausgabeauflösung von analogRead() auf 12 Bit ein → Rückgabebereich 0..4095.
ESP32 kann 9–12 Bit ausgeben; 12 Bit nutzt die volle HW-Breite.
Einfluss auf Skalierung, nicht auf die echte physikalische Genauigkeit oder Geschwindigkeit.
*/
  (void)analogRead(POTI_ADC_PIN);                   // ADC-Kanal vor Pin-Attenuation initialisieren
  analogSetPinAttenuation(POTI_ADC_PIN, ADC_11db);  // Vollbereich ~3.3 V
                                                    /*analogSetPinAttenuation(POTI_ADC_PIN, ADC_11db):
Setzt die Eingangs-Dämpfung des ADC für diesen Pin.
ADC_11db erweitert den messbaren Spannungsbereich auf ca. 0..3,3 V.
Perfekt, wenn dein Poti mit 3,3 V gespeist wird: du nutzt den vollen Messbereich.*/
  Serial.println(F("[POTI] ADC konfiguriert (12 Bit, 0–3,3 V)."));
}

void readPotiTask(unsigned long now) {
  potiRawNow = readPotiRaw();
  potiValid = (potiRawNow > POTI_MIN_RAW && potiRawNow < POTI_MAX_RAW);
}
