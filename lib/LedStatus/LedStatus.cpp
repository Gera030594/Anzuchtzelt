#include "LedStatus.h"

#include <Arduino.h>
#include <math.h>
#include <NeoPixelBus.h>

#include "BmeSensor.h"
#include "Config.h"
#include "HeartbeatWatchdog.h"
#include "LampControl.h"
#include "Pins.h"
#include "WifiTime.h"

/************ LEDs / Farben (NeoPixelBus/RMT) ************/
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod> leds(NUM_LEDS, WS2812_PIN);  // WS2812 = 800 kbit/s, Farbfolge GRB
bool ledsDirty = false;
static RgbColor C(uint8_t r, uint8_t g, uint8_t b) {
  return RgbColor(r, g, b);
}

static RgbColor withBri(const RgbColor& c) {
  return c.Dim(LED_BRIGHTNESS);  // skaliert RGB linear
}

static void ledSet(uint8_t i, const RgbColor& color) {
  if (i < NUM_LEDS) {
    RgbColor scaled = withBri(color);      // Helligkeit anwenden
    RgbColor cur = leds.GetPixelColor(i);  // aktuell gesetzte (bereits skalierte) Farbe
    if (cur.R != scaled.R || cur.G != scaled.G || cur.B != scaled.B) {
      leds.SetPixelColor(i, scaled);
      ledsDirty = true;
    }
  }
}

enum class HumidityLedState : uint8_t {
  Unknown,
  Off,
  HighRed,
  HighYellow,
  HighGreen,
  LowGreen,
  LowYellow,
  LowRed
};

enum class TemperatureLedState : uint8_t {
  Unknown,
  Off,
  BmeError,
  ColdRed,
  ColdYellow,
  ColdGreen,
  WarmGreen,
  WarmYellow,
  WarmRed
};

enum class WifiNtpLedState : uint8_t {
  Unknown,
  OfflineUnsynced,
  OfflineSynced,
  OnlineUnsynced,
  OnlineSynced
};

static HumidityLedState getHumidityLedState(float rh, GrowPhase phase) {
  if (isnan(rh) || rh < 0.0 || rh > 100.0) return HumidityLedState::Off;

  if (phase == GrowPhase::Flowering) {
    if (rh >= 60.0) return HumidityLedState::HighRed;
    if (rh >= 55.0) return HumidityLedState::HighYellow;
    if (rh >= 50.0) return HumidityLedState::HighGreen;
    if (rh >= 45.0) return HumidityLedState::LowGreen;
    if (rh >= 40.0) return HumidityLedState::LowYellow;
    return HumidityLedState::LowRed;
  }

  if (rh > 70.0) return HumidityLedState::HighRed;
  if (rh >= 65.0) return HumidityLedState::HighYellow;
  if (rh >= 60.0) return HumidityLedState::HighGreen;
  if (rh >= 55.0) return HumidityLedState::LowGreen;
  if (rh >= 50.0) return HumidityLedState::LowYellow;
  return HumidityLedState::LowRed;
}

static void applyHumidityLedState(HumidityLedState state) {
  RgbColor led0 = C(0, 0, 0);
  RgbColor led1 = C(0, 0, 0);

  switch (state) {
    case HumidityLedState::HighRed:
      led0 = C(255, 0, 0);
      break;
    case HumidityLedState::HighYellow:
      led0 = C(255, 255, 0);
      break;
    case HumidityLedState::HighGreen:
      led0 = C(0, 255, 0);
      break;
    case HumidityLedState::LowGreen:
      led1 = C(0, 255, 0);
      break;
    case HumidityLedState::LowYellow:
      led1 = C(255, 255, 0);
      break;
    case HumidityLedState::LowRed:
      led1 = C(255, 0, 0);
      break;
    case HumidityLedState::Unknown:
    case HumidityLedState::Off:
      break;
  }

  ledSet(0, led0);
  ledSet(1, led1);
}

static void updateHumidityLeds(float rh, GrowPhase phase) {
  static HumidityLedState lastState = HumidityLedState::Unknown;
  static GrowPhase lastPhase = GrowPhase::Vegetation;
  static bool lastPhaseKnown = false;
  HumidityLedState state = getHumidityLedState(rh, phase);

  if (lastPhaseKnown && state == lastState && phase == lastPhase) return;
  lastState = state;
  lastPhase = phase;
  lastPhaseKnown = true;

  applyHumidityLedState(state);
}

static TemperatureLedState getTemperatureLedState(float temp, bool bmeDisplayError) {
  if (bmeDisplayError) return TemperatureLedState::BmeError;
  if (isnan(temp)) return TemperatureLedState::Off;

  if (temp < 18.0f) return TemperatureLedState::ColdRed;
  if (temp < 20.0f) return TemperatureLedState::ColdYellow;
  if (temp < 22.0f) return TemperatureLedState::ColdGreen;
  if (temp < 24.0f) return TemperatureLedState::WarmGreen;
  if (temp < 28.0f) return TemperatureLedState::WarmYellow;
  return TemperatureLedState::WarmRed;
}

static void applyTemperatureLedState(TemperatureLedState state) {
  RgbColor led4 = C(0, 0, 0);
  RgbColor led5 = C(0, 0, 0);

  switch (state) {
    case TemperatureLedState::BmeError:
      led4 = C(255, 0, 0);
      led5 = C(255, 0, 0);
      break;
    case TemperatureLedState::ColdRed:
      led5 = C(255, 0, 0);
      break;
    case TemperatureLedState::ColdYellow:
      led5 = C(255, 255, 0);
      break;
    case TemperatureLedState::ColdGreen:
      led5 = C(0, 255, 0);
      break;
    case TemperatureLedState::WarmGreen:
      led4 = C(0, 255, 0);
      break;
    case TemperatureLedState::WarmYellow:
      led4 = C(255, 255, 0);
      break;
    case TemperatureLedState::WarmRed:
      led4 = C(255, 0, 0);
      break;
    case TemperatureLedState::Unknown:
    case TemperatureLedState::Off:
      break;
  }

  ledSet(4, led4);
  ledSet(5, led5);
}

static void updateTemperatureLeds(float temp, bool bmeDisplayError) {
  static TemperatureLedState lastState = TemperatureLedState::Unknown;
  TemperatureLedState state = getTemperatureLedState(temp, bmeDisplayError);

  if (state == lastState) return;
  lastState = state;

  applyTemperatureLedState(state);
}

static void updateBmeLeds() {
  GrowPhase phase = getGrowPhase();

  updateTemperatureLeds(T, hasBmeDisplayError());
  updateHumidityLeds(hasValidHumidityForDisplay() ? RH : NAN, phase);
}

void updateModeLed() {
  if (!LED_STATUS_ENABLED) {
    return;
  }

  static GrowPhase lastPhase = GrowPhase::Vegetation;
  static bool lastPhaseKnown = false;
  GrowPhase phase = getGrowPhase();

  if (lastPhaseKnown && phase == lastPhase) return;
  lastPhase = phase;
  lastPhaseKnown = true;

  if (phase == GrowPhase::Flowering) {
    // 12h = Grün
    ledSet(LED_MODE, C(0, 255, 0));
  } else {
    // 18h = Orange
    ledSet(LED_MODE, C(255, 80, 0));
  }
}

static WifiNtpLedState getWifiNtpLedState(bool wifiOk, bool ntpOk) {
  if (!wifiOk && !ntpOk) return WifiNtpLedState::OfflineUnsynced;
  if (!wifiOk && ntpOk) return WifiNtpLedState::OfflineSynced;
  if (wifiOk && !ntpOk) return WifiNtpLedState::OnlineUnsynced;
  return WifiNtpLedState::OnlineSynced;
}

void updateStatusLed() {
  if (!LED_STATUS_ENABLED) {
    return;
  }

  bool wifiOk = isWifiConnected();
  bool ntpOk = isTimeSynced();

  static WifiNtpLedState lastState = WifiNtpLedState::Unknown;
  WifiNtpLedState state = getWifiNtpLedState(wifiOk, ntpOk);

  if (state == lastState) return;
  lastState = state;

  switch (state) {
    case WifiNtpLedState::OfflineUnsynced:
      ledSet(LED_STATUS, C(255, 0, 0));  //  Rot
      break;
    case WifiNtpLedState::OfflineSynced:
      ledSet(LED_STATUS, C(0, 0, 255));  //  Blau
      break;
    case WifiNtpLedState::OnlineUnsynced:
      ledSet(LED_STATUS, C(255, 150, 0));  //  Gelb
      break;
    case WifiNtpLedState::OnlineSynced:
      ledSet(LED_STATUS, C(0, 255, 0));  //  Grün
      break;
    case WifiNtpLedState::Unknown:
      break;
  }
}

void initLEDs() {
  leds.Begin();
  leds.ClearTo(RgbColor(0, 0, 0));  // Alle Pixel sicher auf Schwarz setzen und einmal senden
  ledsDirty = true;
  if (leds.CanShow()) {
    leds.Show();  // alles aus
    ledsDirty = false;
  }
  Serial.println(F("[LED] NeoPixelBus (RMT) initialisiert."));
}

static void updateHeartbeatLed(unsigned long now) {
  static HeartbeatStatus lastStatus = HeartbeatStatus::Grace;
  static bool lastStatusKnown = false;
  HeartbeatStatus status = getHeartbeatStatus(now);

  if (lastStatusKnown && status == lastStatus) return;
  lastStatus = status;
  lastStatusKnown = true;

  switch (status) {
    case HeartbeatStatus::Ok:
      ledSet(6, C(0, 255, 0));  // grün nach echtem RX
      break;
    case HeartbeatStatus::Grace:
      ledSet(6, C(0, 0, 0));  // aus während der Schonfrist
      break;
    case HeartbeatStatus::Timeout:
      ledSet(6, C(255, 0, 0));  // rot bei/ nach Timeout
      break;
  }
}

void ledUpdateTask() {
  if (!LED_STATUS_ENABLED) {
    return;
  }

  static unsigned long nextShowMs = 0;
  unsigned long now = millis();

  updateBmeLeds();
  updateHeartbeatLed(now);

  if (ledsDirty && leds.CanShow() && now >= nextShowMs) {
    leds.Show();
    ledsDirty = false;
    nextShowMs = now + 10;  // min. 10 ms zwischen Sendungen
  }
}
