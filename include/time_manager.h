#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <time.h>

class RTC_DS3231;

/**
 * @brief Manages time synchronization via NTP and RTC
 *
 * Handles:
 * - NTP time synchronization
 * - RTC integration for offline accuracy
 * - Timezone handling (Europe/Amsterdam)
 * - Time formatting utilities
 * - 1-second tick for display updates
 */
class TimeManager {
public:
    TimeManager();
    ~TimeManager();

    /**
     * @brief Initialize NTP and RTC
     * @return true if initialization successful
     */
    bool begin();

    /**
     * @brief Update time from NTP if needed
     * @return true if time was synced
     */
    bool update();

    /**
     * @brief Get current Unix timestamp
     * @return Current time in seconds since epoch
     */
    unsigned long getCurrentTime();

    /**
     * @brief Parse ISO 8601 datetime string to Unix timestamp
     * @param isoString ISO datetime (e.g., "2025-12-24T23:28:00+0100")
     * @return Unix timestamp or 0 if parse failed
     */
    unsigned long parseISO8601(const String& isoString);

    /**
     * @brief Format timestamp as HH:MM:SS
     * @param timestamp Unix timestamp
     * @return Formatted time string
     */
    String formatTime(unsigned long timestamp);

    /**
     * @brief Format timestamp as HH:MM
     * @param timestamp Unix timestamp
     * @return Formatted time string
     */
    String formatTimeShort(unsigned long timestamp);

    /**
     * @brief Calculate seconds between two timestamps
     * @param future Future timestamp
     * @param past Past timestamp (default: current time)
     * @return Seconds difference
     */
    long getSecondsDifference(unsigned long future, unsigned long past = 0);

    /**
     * @brief Check if time has been synced
     * @return true if time is valid
     */
    bool isTimeSynced();

    /**
     * @brief Force NTP sync
     * @return true if sync successful
     */
    bool forceSync();

    /**
     * @brief Get last sync time
     * @return Unix timestamp of last NTP sync
     */
    unsigned long getLastSyncTime();

private:
    void* ntpUDP;    // unused, kept for ABI compat with native stub
    void* timeClient; // unused, kept for ABI compat with native stub
    RTC_DS3231* rtc;

    unsigned long lastSyncTime;
    bool timeSynced;
    const unsigned long SYNC_INTERVAL = 3600000;  // Re-check sync every hour

    /**
     * @brief Sync RTC from NTP
     */
    void syncRTCFromNTP();

    /**
     * @brief Parse date component from ISO string
     */
    bool parseDateComponent(const String& dateStr, int& year, int& month, int& day);

    /**
     * @brief Parse time component from ISO string
     */
    bool parseTimeComponent(const String& timeStr, int& hour, int& minute, int& second);
};

#endif // TIME_MANAGER_H
