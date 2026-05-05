#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <time.h>

class RTC_DS3231;

class TimeManager {
public:
    TimeManager();
    ~TimeManager();

    bool begin();
    bool update();
    unsigned long getCurrentTime();
    unsigned long parseISO8601(const String& isoString);
    String formatTime(unsigned long timestamp);
    String formatTimeShort(unsigned long timestamp);
    long getSecondsDifference(unsigned long future, unsigned long past = 0);
    bool isTimeSynced();
    bool forceSync();
    unsigned long getLastSyncTime();

private:
    void* ntpUDP;     // unused, kept for ABI compat with native stub
    void* timeClient; // unused, kept for ABI compat with native stub
    RTC_DS3231* rtc;

    unsigned long lastSyncTime;
    bool timeSynced;
    const unsigned long SYNC_INTERVAL = 3600000;  // re-sync every hour

    void syncRTCFromNTP();
    bool parseDateComponent(const String& dateStr, int& year, int& month, int& day);
    bool parseTimeComponent(const String& timeStr, int& hour, int& minute, int& second);
};

#endif // TIME_MANAGER_H
