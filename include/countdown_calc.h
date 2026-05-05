#ifndef COUNTDOWN_CALC_H
#define COUNTDOWN_CALC_H

#include <Arduino.h>
#include "ns_api.h"
#include "config.h"
#include "time_manager.h"

enum CountdownState {
    COUNTDOWN_IDLE = 0,
    COUNTDOWN_SAFE,          // >10 min to leave
    COUNTDOWN_READY,         // 5–10 min
    COUNTDOWN_TIME_TO_GO,    // 2–5 min
    COUNTDOWN_URGENT,        // <2 min
    COUNTDOWN_DEPARTED,
    COUNTDOWN_ERROR
};

struct CountdownInfo {
    CountdownState state;
    long secondsUntilLeave;
    long secondsUntilDeparture;
    unsigned long leaveTime;
    unsigned long departureTime;
    const Departure* departure;
    String direction;  // safe copy — readable without holding the API mutex
    String message;

    CountdownInfo() {
        state = COUNTDOWN_IDLE;
        secondsUntilLeave = 0;
        secondsUntilDeparture = 0;
        leaveTime = 0;
        departureTime = 0;
        departure = nullptr;
    }
};

class CountdownCalculator {
public:
    CountdownCalculator();
    ~CountdownCalculator();

    bool begin(ConfigManager* config, TimeManager* timeManager);
    CountdownInfo calculate(const Departure* departure);
    CountdownInfo calculate(unsigned long departureTime);

    static unsigned long calculateLeaveTime(unsigned long departureTime, int travelTime, int bufferTime);
    static String formatCountdown(long seconds);
    static String formatCountdownLong(long seconds);
    static CountdownState getCountdownState(long secondsUntilLeave);
    static String getStateName(CountdownState state);
    static String getStateMessage(CountdownState state);

private:
    ConfigManager* configManager;
    TimeManager* timeManager;

    unsigned long getDepartureTimestamp(const Departure* departure);
};

#endif // COUNTDOWN_CALC_H
