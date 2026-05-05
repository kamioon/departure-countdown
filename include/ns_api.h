#ifndef NS_API_H
#define NS_API_H

#include <Arduino.h>
#include <ArduinoJson.h>

class TimeManager;

struct Departure {
    String direction;
    String trainName;
    String plannedDateTime;
    String actualDateTime;
    unsigned long plannedTime;
    unsigned long actualTime;
    String plannedTrack;
    String actualTrack;
    String trainType;
    bool cancelled;
    String departureStatus;
    int delayMinutes;

    Departure() {
        plannedTime = 0;
        actualTime = 0;
        cancelled = false;
        delayMinutes = 0;
    }
};

class NSApiClient {
public:
    static const int MAX_DEPARTURES = 10;
    NSApiClient();
    ~NSApiClient();

    bool begin(const String& apiKey);
    void setTimeManager(TimeManager* tm);
    int fetchDepartures(const String& stationCode, int maxDepartures = MAX_DEPARTURES);
    const Departure& getDeparture(int index) const;
    int getDepartureCount() const;
    const Departure* getNextDeparture(unsigned long currentTime, int travelTimeMinutes = 0, int bufferTimeMinutes = 0) const;
    void clearDepartures();
    bool isLastRequestSuccessful() const;
    String getLastError() const;
    void setApiKey(const String& apiKey);
    static String getApiUrl(const String& stationCode, int maxJourneys);

private:
    String apiKey;
    Departure departures[MAX_DEPARTURES];
    int departureCount;
    bool lastRequestSuccessful;
    String lastError;
    TimeManager* timeManager;
    static const char* API_BASE_URL;
    static const int REQUEST_TIMEOUT = 10000;

    bool parseResponse(const String& json);
    bool parseDeparture(JsonObject depJson, Departure& departure);
};

#endif // NS_API_H
