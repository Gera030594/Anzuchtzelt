#include "WifiTime.h"

#include <WiFi.h>

#include "Config.h"
#include "LampControl.h"
#include "LedStatus.h"
#include "Pins.h"
#include "../../include/wifi_secrets.h"

// ---------------- WLAN-Daten ----------------
const char* ssid = WIFI_SSID;         // WLAN-Name definieren
const char* password = WIFI_PASSWORD;  // WLAN-Passwort definieren

// ---------------- NTP-Server und Zeitzone ----------------
const char* ntpServer = "pool.ntp.org";  // Adresse des NTP-Zeitservers definieren
const long gmtOffset_sec = 3600;         // GMT-Offset (CET = +1 Stunde)

// ---------------- Zeitsteuerung ----------------
unsigned long lastNTPSync = 0;                   // Zeitpunkt der letzten NTP-Synchronisation speichern
bool firstLampCheckDone = false;                 // Flag, ob erste Lampenprüfung erfolgt ist
bool timeSynced = false;                         // Flag: wurde Zeit bereits erfolgreich synchronisiert?

// ---------------- WLAN-Überwachung ----------------
unsigned long lastWiFiCheck = 0;                // Zeitpunkt der letzten WLAN-Prüfung speichern
bool wifiConnecting = false;                    // Statusflag, ob aktuell versucht wird, WLAN zu verbinden
unsigned long wifiAttemptStart = 0;             // Zeitpunkt des WLAN-Verbindungsversuchs
void handleWiFi(unsigned long now) {
  if (now - lastWiFiCheck >= wifiCheckInterval) {  // Alle 10s prüfen
    lastWiFiCheck = now;
    checkWiFiReconnect();  // WLAN prüfen und ggf. neu verbinden
  }
}

void handleNTP(unsigned long now) {
  // Wenn WLAN getrennt ist → kein Zeitabgleich möglich
  if (WiFi.status() != WL_CONNECTED) return;

  // Wenn Zeit noch nie oder aktuell ungültig → häufiger versuchen
  if (!timeSynced && now - lastNTPSync >= retrySyncInterval) {
    synchronizeTime();
    return;
  }

  // Wenn Zeit synchronisiert → alle 24h neuen Versuch
  if (timeSynced && now - lastNTPSync >= ntpSyncInterval) {
    synchronizeTime();  // Bei Fehlschlag wird timeSynced = false → fällt zurück in Retrymodus
  }
}

void connectWiFi() {
  if (wifiConnecting) return;   // Wenn bereits verbunden → abbrechen
  wifiConnecting = true;        // Flag setzen
  wifiAttemptStart = millis();  // Zeitpunkt speichern
  Serial.println("🔌 Verbinde mit WLAN...");
  WiFi.begin(ssid, password);  // WLAN starten
}

void checkWiFiReconnect() {
  // Wenn WLAN verbunden ist und gerade eine Verbindung hergestellt wurde
  if (WiFi.status() == WL_CONNECTED) {
    if (wifiConnecting) {  // Nur ausführen, wenn wir vorher im Verbindungsversuch waren
      wifiConnecting = false;
      Serial.println("✅ WLAN verbunden!");
      Serial.print("📡 IP-Adresse: ");
      Serial.println(WiFi.localIP());
    }
    return;  // Verbindung steht → nichts weiter tun
  }

  // Wenn keine Verbindung besteht und kein Versuch läuft → neuen Versuch starten
  if (!wifiConnecting) {
    connectWiFi();
  }

  // Wenn ein Versuch läuft, aber nach 15 Sekunden kein Erfolg → neu versuchen
  if (wifiConnecting && millis() - wifiAttemptStart > 15000) {
    Serial.println("⚠️ WLAN-Verbindung fehlgeschlagen, neuer Versuch...");
    WiFi.disconnect(true);
    wifiConnecting = false;
  }
}

void synchronizeTime() {
  Serial.println("Synchronisiere Zeit mit NTP...");

  // Immer zunächst mit GMT (ohne Sommerzeit) synchronisieren
  configTime(gmtOffset_sec, 0, ntpServer);

  struct tm timeinfo;

  // Versuche einmalig, Zeit abzurufen
  if (!getLocalTime(&timeinfo)) {
    Serial.println("❌ NTP-Zeit konnte nicht synchronisiert werden.");
    timeSynced = false;
    return;
  }


  // Jetzt prüfen, ob Sommerzeit aktiv ist
  bool sommer = isSommerzeit(&timeinfo);
  int offset = sommer ? 3600 : 0;

  // Systemzeit neu konfigurieren mit richtigem Offset
  configTime(gmtOffset_sec, offset, ntpServer);
  getLocalTime(&timeinfo);  // Zeit erneut aktualisieren

  Serial.printf("✅ Zeit synchronisiert. Aktuell: %szeit (Offset: %d Sekunden)\n",
                sommer ? "Sommer" : "Winter", offset);

  timeSynced = true;
  lastNTPSync = millis();

  if (!firstLampCheckDone) {
    checkLampState();
    firstLampCheckDone = true;
  }
}

bool isSommerzeit(struct tm* timeinfo) {
  int year = timeinfo->tm_year + 1900;  // Jahr korrigieren (tm_year ist Jahre seit 1900)
  int month = timeinfo->tm_mon + 1;     // Monat korrigieren (tm_mon ist 0–11)
  int day = timeinfo->tm_mday;          // Tag im Monat

  // --- Letzten Sonntag im März berechnen ---
  // Erster April minus Wochentag → letzter Sonntag im März
  int letzterSonntagMaerz = 31 - ((5 + year * 5 / 4) % 7);

  // --- Letzten Sonntag im Oktober berechnen ---
  int letzterSonntagOktober = 31 - ((2 + year * 5 / 4) % 7);

  // --- Regel prüfen ---
  // Sommerzeit: zwischen letztem Sonntag im März, 2:00 Uhr, und letztem Sonntag im Oktober, 3:00 Uhr
  if ((month > 3 && month < 10) || (month == 3 && (day > letzterSonntagMaerz || (day == letzterSonntagMaerz && timeinfo->tm_hour >= 2))) || (month == 10 && (day < letzterSonntagOktober || (day == letzterSonntagOktober && timeinfo->tm_hour < 3)))) {
    return true;  // Sommerzeit aktiv
  } else {
    return false;  // Winterzeit aktiv
  }
}

void updateStatusLed() {
  bool wifiOk = (WiFi.status() == WL_CONNECTED);
  bool ntpOk = timeSynced;

  static bool lastWifiOk = false;
  static bool lastNtpOk = false;

  if (wifiOk == lastWifiOk && ntpOk == lastNtpOk) return;

  lastWifiOk = wifiOk;
  lastNtpOk = ntpOk;

  if (!wifiOk && !ntpOk) {
    ledSet(LED_STATUS, C(255, 0, 0));  //  Rot
  } else if (!wifiOk && ntpOk) {
    ledSet(LED_STATUS, C(0, 0, 255));  //  Blau
  } else if (wifiOk && !ntpOk) {
    ledSet(LED_STATUS, C(255, 150, 0));  //  Gelb
  } else {
    ledSet(LED_STATUS, C(0, 255, 0));  //  Grün
  }
}
