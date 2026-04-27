#include "countdown_calc.h"

CountdownCalculator::CountdownCalculator() : configManager(nullptr), timeManager(nullptr) {
}

CountdownCalculator::~CountdownCalculator() {
}

bool CountdownCalculator::begin(ConfigManager* config, TimeManager* timeMgr) {
    if (!config || !timeMgr) {
        return false;
    }

    configManager = config;
    timeManager = timeMgr;

    return true;
}

CountdownInfo CountdownCalculator::calculate(const Departure* departure) {
    CountdownInfo info;

    Serial.println("[DEBUG] CountdownCalculator::calculate(Departure*) called");

    if (!departure || !configManager || !timeManager) {
        Serial.println("[DEBUG] Invalid pointers!");
        info.state = COUNTDOWN_ERROR;
        info.message = "Invalid state";
        return info;
    }

    // Parse departure time
    unsigned long depTime = getDepartureTimestamp(departure);

    Serial.print("[DEBUG] getDepartureTimestamp returned: ");
    Serial.println(depTime);

    if (depTime == 0) {
        Serial.println("[DEBUG] Departure time is 0!");
        info.state = COUNTDOWN_ERROR;
        info.message = "Invalid departure time";
        return info;
    }

    Serial.println("[DEBUG] About to call calculate(unsigned long)");
    info = calculate(depTime);  // Get countdown info
    info.departure = departure;  // Set departure pointer AFTER
    return info;
}

CountdownInfo CountdownCalculator::calculate(unsigned long departureTime) {
    CountdownInfo info;

    if (!configManager || !timeManager) {
        info.state = COUNTDOWN_ERROR;
        info.message = "Not initialized";
        return info;
    }

    // Get current time
    unsigned long currentTime = timeManager->getCurrentTime();

    if (currentTime == 0) {
        info.state = COUNTDOWN_ERROR;
        info.message = "Time not synced";
        return info;
    }

    // Get travel time and buffer from configuration
    int travelTime = configManager->getActiveTravelTime();
    int bufferTime = configManager->getConfig().bufferTime;

    // Debug: Show configuration
    Serial.print("  Travel time: ");
    Serial.print(travelTime);
    Serial.print(" min, Buffer: ");
    Serial.print(bufferTime);
    Serial.println(" min");

    // Calculate leave time
    info.departureTime = departureTime;
    info.leaveTime = calculateLeaveTime(departureTime, travelTime, bufferTime);

    // Debug: Show leave time calculation
    Serial.print("  Departure: ");
    Serial.print(departureTime);
    Serial.print(", Leave time: ");
    Serial.print(info.leaveTime);
    Serial.print(", Current: ");
    Serial.println(currentTime);

    // Calculate time remaining
    info.secondsUntilDeparture = (long)(departureTime - currentTime);
    info.secondsUntilLeave = (long)(info.leaveTime - currentTime);

    // Debug: Show countdown values
    Serial.print("  Seconds until departure: ");
    Serial.print(info.secondsUntilDeparture);
    Serial.print(", Seconds until leave: ");
    Serial.println(info.secondsUntilLeave);

    // Determine state
    if (info.secondsUntilDeparture < 0) {
        info.state = COUNTDOWN_DEPARTED;
        info.message = "Train departed";
    } else {
        info.state = getCountdownState(info.secondsUntilLeave);
        info.message = getStateMessage(info.state);
    }

    return info;
}

unsigned long CountdownCalculator::calculateLeaveTime(unsigned long departureTime,
                                                     int travelTime,
                                                     int bufferTime) {
    // leaveTime = departureTime - travelTime - bufferTime
    int totalMinutes = travelTime + bufferTime;
    unsigned long totalSeconds = totalMinutes * 60;

    if (departureTime < totalSeconds) {
        return 0;  // Invalid: can't leave before epoch
    }

    return departureTime - totalSeconds;
}

unsigned long CountdownCalculator::getDepartureTimestamp(const Departure* departure) {
    if (!departure || !timeManager) {
        return 0;
    }

    // ALWAYS use planned time for countdown calculation
    // Users need to leave based on scheduled departure time, NOT delays!
    if (departure->plannedTime > 0) {
        return departure->plannedTime;
    } else if (departure->plannedDateTime.length() > 0) {
        // Parse planned datetime string
        return timeManager->parseISO8601(departure->plannedDateTime);
    } else if (departure->actualTime > 0) {
        // Fallback to actual time only if planned time unavailable
        return departure->actualTime;
    } else if (departure->actualDateTime.length() > 0) {
        // Parse actual datetime string
        return timeManager->parseISO8601(departure->actualDateTime);
    }

    return 0;
}

String CountdownCalculator::formatCountdown(long seconds) {
    bool negative = seconds < 0;
    long absSeconds = labs(seconds);

    int mins = absSeconds / 60;
    int secs = absSeconds % 60;

    char buffer[7];
    if (negative) {
        sprintf(buffer, "-%02d:%02d", mins, secs);
    } else {
        sprintf(buffer, "%02d:%02d", mins, secs);
    }

    return String(buffer);
}

String CountdownCalculator::formatCountdownLong(long seconds) {
    bool negative = seconds < 0;
    long absSeconds = labs(seconds);

    int hours = absSeconds / 3600;
    int mins = (absSeconds % 3600) / 60;
    int secs = absSeconds % 60;

    char buffer[10];
    if (negative) {
        sprintf(buffer, "-%02d:%02d:%02d", hours, mins, secs);
    } else {
        sprintf(buffer, "%02d:%02d:%02d", hours, mins, secs);
    }

    return String(buffer);
}

CountdownState CountdownCalculator::getCountdownState(long secondsUntilLeave) {
    if (secondsUntilLeave < 0) {
        return COUNTDOWN_DEPARTED;
    } else if (secondsUntilLeave < 120) {  // <2 min
        return COUNTDOWN_URGENT;
    } else if (secondsUntilLeave < 300) {  // 2-5 min
        return COUNTDOWN_TIME_TO_GO;
    } else if (secondsUntilLeave < 600) {  // 5-10 min
        return COUNTDOWN_READY;
    } else {
        return COUNTDOWN_SAFE;
    }
}

String CountdownCalculator::getStateName(CountdownState state) {
    switch (state) {
        case COUNTDOWN_IDLE:
            return "Idle";
        case COUNTDOWN_SAFE:
            return "Safe";
        case COUNTDOWN_READY:
            return "Get Ready";
        case COUNTDOWN_TIME_TO_GO:
            return "Time to Go";
        case COUNTDOWN_URGENT:
            return "Leave Now!";
        case COUNTDOWN_DEPARTED:
            return "Departed";
        case COUNTDOWN_ERROR:
            return "Error";
        default:
            return "Unknown";
    }
}

String CountdownCalculator::getStateMessage(CountdownState state) {
    switch (state) {
        case COUNTDOWN_IDLE:
            return "No departure";
        case COUNTDOWN_SAFE:
            return "Plenty of time";
        case COUNTDOWN_READY:
            return "Get ready to leave";
        case COUNTDOWN_TIME_TO_GO:
            return "Time to leave soon";
        case COUNTDOWN_URGENT:
            return "LEAVE NOW!";
        case COUNTDOWN_DEPARTED:
            return "Train departed";
        case COUNTDOWN_ERROR:
            return "System error";
        default:
            return "";
    }
}
