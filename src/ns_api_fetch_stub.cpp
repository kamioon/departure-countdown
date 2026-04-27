#include "ns_api.h"

int NSApiClient::fetchDepartures(const String& stationCode, int maxDepartures) {
    (void)stationCode;
    (void)maxDepartures;
    // For unit tests, simulate fetched departures
    departureCount = 0;
    lastRequestSuccessful = true;
    lastError = "";
    return 0;
}
