#include "time_manager.h"

unsigned long TimeManager::parseISO8601(const String& isoString) {
    // Expected format: "2025-12-24T23:28:00+0100"
    if (isoString.length() < 19) {
        Serial.println("ISO8601 string too short: " + isoString);
        return 0;
    }

    // Split date and time
    int tIndex = isoString.indexOf('T');
    if (tIndex == -1) {
        Serial.println("Invalid ISO8601 format (no T separator): " + isoString);
        return 0;
    }

    String dateStr = isoString.substring(0, tIndex);
    String timeStr = isoString.substring(tIndex + 1);

    // Remove timezone offset
    int plusIndex = timeStr.indexOf('+');
    int minusIndex = timeStr.indexOf('-');
    if (plusIndex > 0) {
        timeStr = timeStr.substring(0, plusIndex);
    } else if (minusIndex > 0) {
        timeStr = timeStr.substring(0, minusIndex);
    }

    // Parse date components
    int year, month, day;
    if (!parseDateComponent(dateStr, year, month, day)) {
        return 0;
    }

    // Parse time components
    int hour, minute, second;
    if (!parseTimeComponent(timeStr, hour, minute, second)) {
        return 0;
    }

    // Create tm struct
    struct tm timeinfo = {0};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    timeinfo.tm_isdst = -1;

    // Convert to Unix timestamp
    time_t timestamp = mktime(&timeinfo);

    return (unsigned long)timestamp;
}

bool TimeManager::parseDateComponent(const String& dateStr, int& year, int& month, int& day) {
    // Expected format: "2025-12-24"
    int firstDash = dateStr.indexOf('-');
    int secondDash = dateStr.indexOf('-', firstDash + 1);

    if (firstDash == -1 || secondDash == -1) {
        Serial.println("Invalid date format: " + dateStr);
        return false;
    }

    year = dateStr.substring(0, firstDash).toInt();
    month = dateStr.substring(firstDash + 1, secondDash).toInt();
    day = dateStr.substring(secondDash + 1).toInt();

    return true;
}

bool TimeManager::parseTimeComponent(const String& timeStr, int& hour, int& minute, int& second) {
    // Expected format: "23:28:00"
    int firstColon = timeStr.indexOf(':');
    int secondColon = timeStr.indexOf(':', firstColon + 1);

    if (firstColon == -1 || secondColon == -1) {
        Serial.println("Invalid time format: " + timeStr);
        return false;
    }

    hour = timeStr.substring(0, firstColon).toInt();
    minute = timeStr.substring(firstColon + 1, secondColon).toInt();
    second = timeStr.substring(secondColon + 1).toInt();

    return true;
}

String TimeManager::formatTime(unsigned long timestamp) {
    time_t rawtime = (time_t)timestamp;
    struct tm* timeinfo = localtime(&rawtime);

    char buffer[9];
    sprintf(buffer, "%02d:%02d:%02d",
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec);

    return String(buffer);
}

String TimeManager::formatTimeShort(unsigned long timestamp) {
    time_t rawtime = (time_t)timestamp;
    struct tm* timeinfo = localtime(&rawtime);

    char buffer[6];
    sprintf(buffer, "%02d:%02d",
            timeinfo->tm_hour,
            timeinfo->tm_min);

    return String(buffer);
}

long TimeManager::getSecondsDifference(unsigned long future, unsigned long past) {
    if (past == 0) {
        past = getCurrentTime();
    }

    if (future < past) {
        return 0;
    }

    return (long)(future - past);
}

bool TimeManager::isTimeSynced() {
    return timeSynced;
}

unsigned long TimeManager::getLastSyncTime() {
    return lastSyncTime;
}
