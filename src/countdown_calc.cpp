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

    if (!departure || !configManager || !timeManager) {
        info.state = COUNTDOWN_ERROR;
        info.message = "Invalid state";
        return info;
    }

    unsigned long depTime = getDepartureTimestamp(departure);

    if (depTime == 0) {
        info.state = COUNTDOWN_ERROR;
        info.message = "Invalid departure time";
        return info;
    }

    info = calculate(depTime);
    info.departure = departure;  // set AFTER to avoid racing with the background fetch task
    return info;
}

CountdownInfo CountdownCalculator::calculate(unsigned long departureTime) {
    CountdownInfo info;

    if (!configManager || !timeManager) {
        info.state = COUNTDOWN_ERROR;
        info.message = "Not initialized";
        return info;
    }

    unsigned long currentTime = timeManager->getCurrentTime();

    if (currentTime == 0) {
        info.state = COUNTDOWN_ERROR;
        info.message = "Time not synced";
        return info;
    }

    int travelTime = configManager->getActiveTravelTime();
    int bufferTime = configManager->getConfig().bufferTime;

    info.departureTime = departureTime;
    info.leaveTime = calculateLeaveTime(departureTime, travelTime, bufferTime);

    info.secondsUntilDeparture = (long)(departureTime - currentTime);
    info.secondsUntilLeave     = (long)(info.leaveTime - currentTime);

    if (info.secondsUntilDeparture < 0) {
        info.state   = COUNTDOWN_DEPARTED;
        info.message = "Train departed";
    } else {
        info.state   = getCountdownState(info.secondsUntilLeave);
        info.message = getStateMessage(info.state);
    }

    return info;
}

unsigned long CountdownCalculator::calculateLeaveTime(unsigned long departureTime,
                                                     int travelTime,
                                                     int bufferTime) {
    unsigned long totalSeconds = (travelTime + bufferTime) * 60;

    if (departureTime < totalSeconds) {
        return 0;
    }

    return departureTime - totalSeconds;
}

unsigned long CountdownCalculator::getDepartureTimestamp(const Departure* departure) {
    if (!departure || !timeManager) {
        return 0;
    }

    // Always use planned time — users leave based on the schedule, not real-time delays.
    if (departure->plannedTime > 0) {
        return departure->plannedTime;
    } else if (departure->plannedDateTime.length() > 0) {
        return timeManager->parseISO8601(departure->plannedDateTime);
    } else if (departure->actualTime > 0) {
        return departure->actualTime;
    } else if (departure->actualDateTime.length() > 0) {
        return timeManager->parseISO8601(departure->actualDateTime);
    }

    return 0;
}

String CountdownCalculator::formatCountdown(long seconds) {
    if (seconds < 0) seconds = 0;
    int mins = (int)(seconds / 60);
    int secs = (int)(seconds % 60);

    char buffer[6];
    sprintf(buffer, "%02d:%02d", mins, secs);
    return String(buffer);
}

String CountdownCalculator::formatCountdownLong(long seconds) {
    bool negative = seconds < 0;
    long absSeconds = labs(seconds);

    int hours = absSeconds / 3600;
    int mins  = (absSeconds % 3600) / 60;
    int secs  = absSeconds % 60;

    char buffer[10];
    if (negative) {
        sprintf(buffer, "-%02d:%02d:%02d", hours, mins, secs);
    } else {
        sprintf(buffer, "%02d:%02d:%02d", hours, mins, secs);
    }

    return String(buffer);
}

CountdownState CountdownCalculator::getCountdownState(long secondsUntilLeave) {
    if (secondsUntilLeave < 0)   return COUNTDOWN_DEPARTED;
    if (secondsUntilLeave < 120) return COUNTDOWN_URGENT;
    if (secondsUntilLeave < 300) return COUNTDOWN_TIME_TO_GO;
    if (secondsUntilLeave < 600) return COUNTDOWN_READY;
    return COUNTDOWN_SAFE;
}

String CountdownCalculator::getStateName(CountdownState state) {
    switch (state) {
        case COUNTDOWN_IDLE:        return "Idle";
        case COUNTDOWN_SAFE:        return "Safe";
        case COUNTDOWN_READY:       return "Get Ready";
        case COUNTDOWN_TIME_TO_GO:  return "Time to Go";
        case COUNTDOWN_URGENT:      return "Leave Now!";
        case COUNTDOWN_DEPARTED:    return "Departed";
        case COUNTDOWN_ERROR:       return "Error";
        default:                    return "Unknown";
    }
}

String CountdownCalculator::getStateMessage(CountdownState state) {
    switch (state) {
        case COUNTDOWN_IDLE:        return "No departure";
        case COUNTDOWN_SAFE:        return "Plenty of time";
        case COUNTDOWN_READY:       return "Get ready to leave";
        case COUNTDOWN_TIME_TO_GO:  return "Time to leave soon";
        case COUNTDOWN_URGENT:      return "LEAVE NOW!";
        case COUNTDOWN_DEPARTED:    return "Train departed";
        case COUNTDOWN_ERROR:       return "System error";
        default:                    return "";
    }
}
