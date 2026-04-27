#ifndef COUNTDOWN_CALC_H
#define COUNTDOWN_CALC_H

#include <Arduino.h>
#include "ns_api.h"
#include "config.h"
#include "time_manager.h"

/**
 * @brief Countdown state enumeration
 */
enum CountdownState {
    COUNTDOWN_IDLE = 0,         // No active countdown
    COUNTDOWN_SAFE,             // Plenty of time (>10 min)
    COUNTDOWN_READY,            // Get ready (5-10 min)
    COUNTDOWN_TIME_TO_GO,       // Time to leave (2-5 min)
    COUNTDOWN_URGENT,           // Leave now! (<2 min)
    COUNTDOWN_DEPARTED,         // Train already departed
    COUNTDOWN_ERROR             // Error state
};

/**
 * @brief Countdown information
 */
struct CountdownInfo {
    CountdownState state;
    long secondsUntilLeave;      // Seconds until leave time
    long secondsUntilDeparture;  // Seconds until train departure
    unsigned long leaveTime;     // Unix timestamp when to leave
    unsigned long departureTime; // Unix timestamp of train departure
    const Departure* departure;  // Pointer to next departure (may be nullptr during background fetch)
    String direction;            // Copy of departure direction — safe to read without holding the API mutex
    String message;              // Status message

    CountdownInfo() {
        state = COUNTDOWN_IDLE;
        secondsUntilLeave = 0;
        secondsUntilDeparture = 0;
        leaveTime = 0;
        departureTime = 0;
        departure = nullptr;
    }
};

/**
 * @brief Smart Countdown Calculator
 *
 * Handles:
 * - Calculate "leave home" time based on departure and travel time
 * - Real-time countdown updates
 * - Next departure selection logic
 * - Handle edge cases (train in <5 min, already departed)
 * - State management for alerts
 */
class CountdownCalculator {
public:
    CountdownCalculator();
    ~CountdownCalculator();

    /**
     * @brief Initialize calculator
     * @param config Configuration manager reference
     * @param timeManager Time manager reference
     * @return true if initialization successful
     */
    bool begin(ConfigManager* config, TimeManager* timeManager);

    /**
     * @brief Calculate countdown from departure
     * @param departure Departure to calculate from
     * @return Countdown information
     */
    CountdownInfo calculate(const Departure* departure);

    /**
     * @brief Calculate countdown from departure time
     * @param departureTime Unix timestamp of departure
     * @return Countdown information
     */
    CountdownInfo calculate(unsigned long departureTime);

    /**
     * @brief Calculate leave time
     * @param departureTime Unix timestamp of departure
     * @param travelTime Travel time in minutes
     * @param bufferTime Buffer time in minutes
     * @return Unix timestamp when to leave
     */
    static unsigned long calculateLeaveTime(unsigned long departureTime,
                                           int travelTime,
                                           int bufferTime);

    /**
     * @brief Format seconds as MM:SS
     * @param seconds Seconds to format
     * @return Formatted string
     */
    static String formatCountdown(long seconds);

    /**
     * @brief Format seconds as HH:MM:SS
     * @param seconds Seconds to format
     * @return Formatted string
     */
    static String formatCountdownLong(long seconds);

    /**
     * @brief Get countdown state based on remaining time
     * @param secondsUntilLeave Seconds until leave time
     * @return Countdown state
     */
    static CountdownState getCountdownState(long secondsUntilLeave);

    /**
     * @brief Get state name
     * @param state Countdown state
     * @return State name as string
     */
    static String getStateName(CountdownState state);

    /**
     * @brief Get status message for state
     * @param state Countdown state
     * @return Status message
     */
    static String getStateMessage(CountdownState state);

private:
    ConfigManager* configManager;
    TimeManager* timeManager;

    /**
     * @brief Parse departure time from Departure struct
     * @param departure Departure information
     * @return Unix timestamp
     */
    unsigned long getDepartureTimestamp(const Departure* departure);
};

#endif // COUNTDOWN_CALC_H
