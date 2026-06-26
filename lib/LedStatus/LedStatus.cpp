#include "LedStatus.h"

#include <math.h>

#include "Config.h"
#include "MotorControl.h"
#include "Pins.h"

/************ LEDs / Farben (NeoPixelBus/RMT) ************/
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod> leds(NUM_LEDS, WS2812_PIN);  // WS2812 = 800 kbit/s, Farbfolge GRB
bool ledsDirty = false;
RgbColor C(uint8_t r, uint8_t g, uint8_t b) {
  return RgbColor(r, g, b);
}

RgbColor withBri(const RgbColor& c) {
  return c.Dim(LED_BRIGHTNESS);  // skaliert RGB linear
}

void ledSet(uint8_t i, const RgbColor& color) {
  if (i < NUM_LEDS) {
    RgbColor scaled = withBri(color);      // Helligkeit anwenden
    RgbColor cur = leds.GetPixelColor(i);  // aktuell gesetzte (bereits skalierte) Farbe
    if (cur.R != scaled.R || cur.G != scaled.G || cur.B != scaled.B) {
      leds.SetPixelColor(i, scaled);
      ledsDirty = true;
    }
  }
}

void updateZoneLEDs(float temp) {
  if (isnan(temp)) {
    // Kein gültiger Sensorwert -> beide LEDs aus
    ledSet(4, C(0, 0, 0));
    ledSet(5, C(0, 0, 0));
    return;  // direkt raus – spart unnötige Vergleiche
  }

  // --- Kalt-Zone (LED5) ---
  if (temp < 18.0f) {
    ledSet(5, C(255, 0, 0));
  } else if (temp < 20.0f) {
    ledSet(5, C(255, 255, 0));
  } else if (temp < 22.0f) {
    ledSet(5, C(0, 255, 0));
  } else {
    ledSet(5, C(0, 0, 0));
  }

  // --- Warm-Zone (LED4) ---
  if (temp < 22.0f) {
    ledSet(4, C(0, 0, 0));  // Untere Zone aktiv -> obere aus
  } else if (temp < 24.0f) {
    ledSet(4, C(0, 255, 0));
  } else if (temp < 28.0f) {
    ledSet(4, C(255, 255, 0));
  } else {
    ledSet(4, C(255, 0, 0));
  }
}

void initLEDs() {
  leds.Begin();
  leds.ClearTo(RgbColor(0, 0, 0));  // Alle Pixel sicher auf Schwarz setzen und einmal senden
  leds.Show();                      // alles aus
  ledsDirty = false;
  Serial.println(F("[LED] NeoPixelBus (RMT) initialisiert."));
}

void ledUpdateTask() {
  static unsigned long nextShowMs = 0;
  unsigned long now = millis();

  if (getMotorFault() != MOTOR_FAULT_NONE) {
    ledSet(3, C(255, 0, 0));
  }

  if (ledsDirty && leds.CanShow() && now >= nextShowMs) {
    leds.Show();
    ledsDirty = false;
    nextShowMs = now + 10;  // min. 10 ms zwischen Sendungen
  }
}
