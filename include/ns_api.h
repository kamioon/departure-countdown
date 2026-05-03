#ifndef NS_API_H
#define NS_API_H

#include <Arduino.h>
#include <ArduinoJson.h>


// Forward declaration
class TimeManager;

/**
 * @brief Train departure information
 */
struct Departure {
    String direction;           // Destination (e.g., "Den Haag Centraal")
    String trainName;           // Train identifier (e.g., "NS 6084")
    String plannedDateTime;     // ISO 8601 planned time
    String actualDateTime;      // ISO 8601 actual time
    unsigned long plannedTime;  // Parsed Unix timestamp
    unsigned long actualTime;   // Parsed Unix timestamp
    String plannedTrack;        // Planned track number
    String actualTrack;         // Actual track number
    String trainType;           // Train type (e.g., "Sprinter", "Intercity")
    bool cancelled;             // Is train cancelled?
    String departureStatus;     // Status (e.g., "INCOMING", "ON_PLATFORM")
    int delayMinutes;           // Delay in minutes

    Departure() {
        plannedTime = 0;
        actualTime = 0;
        cancelled = false;
        delayMinutes = 0;
    }
};

/**
 * @brief NS API Client for fetching train departures
 *
 * Handles:
 * - HTTP requests to NS API
 * - Authentication with subscription key
 * - JSON response parsing
 * - Departure data extraction
 * - Error handling and retries
 */
class NSApiClient {
public:
    static const int MAX_DEPARTURES = 10;
    NSApiClient();
    ~NSApiClient();

    /**
     * @brief Initialize API client
     * @param apiKey NS API subscription key
     * @return true if initialization successful
     */
    bool begin(const String& apiKey);

    /**
     * @brief Set TimeManager for parsing ISO8601 timestamps
     * @param tm Pointer to TimeManager instance
     */
    void setTimeManager(TimeManager* tm);

    /**
     * @brief Fetch departures for a station
     * @param stationCode Station code (e.g., "AMST")
     * @param maxDepartures Maximum number of departures to fetch
     * @return Number of departures fetched, -1 on error
     */
    int fetchDepartures(const String& stationCode, int maxDepartures = MAX_DEPARTURES);

    /**
     * @brief Get departure by index
     * @param index Departure index (0-based)
     * @return Reference to departure, or empty departure if invalid
     */
    const Departure& getDeparture(int index) const;

    /**
     * @brief Get number of fetched departures
     * @return Number of departures
     */
    int getDepartureCount() const;

    /**
     * @brief Get next valid departure (not cancelled, not departed, and still catchable)
     * @param currentTime Current Unix timestamp
     * @param travelTimeMinutes Travel time in minutes (0 to skip leave time check)
     * @param bufferTimeMinutes Buffer time in minutes (0 to skip leave time check)
     * @return Pointer to next departure, or nullptr if none found
     */
    const Departure* getNextDeparture(unsigned long currentTime, int travelTimeMinutes = 0, int bufferTimeMinutes = 0) const;

    /**
     * @brief Clear all cached departures
     */
    void clearDepartures();

    /**
     * @brief Check if last request was successful
     * @return true if last fetch succeeded
     */
    bool isLastRequestSuccessful() const;

    /**
     * @brief Get last error message
     * @return Error message string
     */
    String getLastError() const;

    /**
     * @brief Set API key
     * @param apiKey NS API subscription key
     */
    void setApiKey(const String& apiKey);

    /**
     * @brief Get API URL for testing
     */
    static String getApiUrl(const String& stationCode, int maxJourneys);

private:
    String apiKey;
    Departure departures[MAX_DEPARTURES];  // Cache up to MAX_DEPARTURES departures
    int departureCount;
    bool lastRequestSuccessful;
    String lastError;
    TimeManager* timeManager;  // For parsing ISO8601 timestamps
    static const char* API_BASE_URL;
    static const int REQUEST_TIMEOUT = 10000;  // 10 seconds

    /**
     * @brief Parse JSON response
     * @param json JSON string
     * @return true if parsing successful
     */
    bool parseResponse(const String& json);

    /**
     * @brief Parse single departure from JSON
     * @param depJson JSON object for departure
     * @param departure Output departure structure
     * @return true if parsing successful
     */
    bool parseDeparture(JsonObject depJson, Departure& departure);

    /**
     * @brief Calculate delay in minutes
     * @param planned Planned timestamp
     * @param actual Actual timestamp
     * @return Delay in minutes
     */
    int calculateDelay(unsigned long planned, unsigned long actual);
};

#endif // NS_API_H
