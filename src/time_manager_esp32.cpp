#include "time_manager.h"
#include <RTClib.h>
#include <esp_sntp.h>

// POSIX TZ string for Europe/Amsterdam — handles CET/CEST transitions automatically.
// CET-1: UTC+1 in winter; CEST: UTC+2 in summer.
// DST begins last Sunday of March at 02:00, ends last Sunday of October at 03:00.
static const char* TZ_AMSTERDAM = "CET-1CEST,M3.5.0,M10.5.0/3";

TimeManager::TimeManager() : lastSyncTime(0), timeSynced(false) {
    ntpUDP = nullptr;
    timeClient = nullptr;
    rtc = nullptr;
}

TimeManager::~TimeManager() {
    if (rtc) delete rtc;
}

bool TimeManager::begin() {
    // Configure ESP32's built-in SNTP with Amsterdam timezone.
    // time(nullptr) will return UTC epoch; localtime() will return Amsterdam local time.
    configTzTime(TZ_AMSTERDAM, "pool.ntp.org", "time.google.com", "time.cloudflare.com");

    // Initialize RTC (optional — may not be connected)
    rtc = new RTC_DS3231();
    if (!rtc->begin()) {
        Serial.println("Warning: RTC not found, continuing without RTC");
        delete rtc;
        rtc = nullptr;
    } else {
        Serial.println("RTC detected and initialized");
    }

    if (!forceSync()) {
        Serial.println("Initial NTP sync failed, will retry later");
    }

    return true;
}

bool TimeManager::update() {
    unsigned long now = millis();
    if (now - lastSyncTime >= SYNC_INTERVAL) {
        return forceSync();
    }
    return true;
}

bool TimeManager::forceSync() {
    Serial.println("Waiting for SNTP sync...");

    // Wait up to 10 s for the ESP32 SNTP stack to set the clock
    for (int i = 0; i < 20; i++) {
        if (time(nullptr) > 1000000000UL) {
            timeSynced = true;
            lastSyncTime = millis();
            syncRTCFromNTP();
            Serial.println("SNTP sync successful");
            return true;
        }
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nSNTP sync timed out");
    return false;
}

unsigned long TimeManager::getCurrentTime() {
    if (timeSynced) {
        return (unsigned long)time(nullptr);  // UTC epoch
    } else if (rtc) {
        DateTime now = rtc->now();
        return now.unixtime();
    }
    return 0;
}

void TimeManager::syncRTCFromNTP() {
    if (!rtc) return;

    unsigned long epochUtc = (unsigned long)time(nullptr);
    rtc->adjust(DateTime(epochUtc));
    Serial.println("RTC synced from NTP");
}
