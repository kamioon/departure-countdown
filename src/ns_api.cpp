#include "ns_api.h"
#include "time_manager.h"

const char* NSApiClient::API_BASE_URL = "https://gateway.apiportal.ns.nl/reisinformatie-api/api/v2/departures";

NSApiClient::NSApiClient() : departureCount(0), lastRequestSuccessful(false), timeManager(nullptr) {
}

NSApiClient::~NSApiClient() {
}

void NSApiClient::setTimeManager(TimeManager* tm) {
    timeManager = tm;
}

bool NSApiClient::begin(const String& apiKey) {
    if (apiKey.length() == 0) {
        lastError = "API key is empty";
        return false;
    }

    this->apiKey = apiKey;
    return true;
}

void NSApiClient::setApiKey(const String& apiKey) {
    this->apiKey = apiKey;
}

String NSApiClient::getApiUrl(const String& stationCode, int maxJourneys) {
    String url = String(API_BASE_URL);
    url += "?station=" + stationCode;
    if (maxJourneys > 0) {
        url += "&maxJourneys=" + String(maxJourneys);
    }
    return url;
}

bool NSApiClient::parseResponse(const String& json) {
    clearDepartures();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        lastError = "JSON parse error: " + String(error.c_str());
        return false;
    }

    if (!doc["payload"].is<JsonObject>()) {
        lastError = "Response missing 'payload' field";
        return false;
    }

    JsonObject payload = doc["payload"];

    if (!payload["departures"].is<JsonArray>()) {
        lastError = "Payload missing 'departures' field";
        return false;
    }

    JsonArray departuresArray = payload["departures"].as<JsonArray>();

    for (JsonObject depJson : departuresArray) {
        if (departureCount >= MAX_DEPARTURES) break;

        if (parseDeparture(depJson, departures[departureCount])) {
            departureCount++;
        }
    }

    return true;
}

bool NSApiClient::parseDeparture(JsonObject depJson, Departure& departure) {
    if (!depJson["direction"].is<String>() || !depJson["plannedDateTime"].is<String>()) {
        return false;
    }

    // Use String() constructor — ArduinoJson returns a temporary reference, not a copy
    departure.direction        = String(depJson["direction"].as<const char*>());
    departure.plannedDateTime  = String(depJson["plannedDateTime"].as<const char*>());


    if (depJson["name"].is<String>()) {
        departure.trainName = String(depJson["name"].as<const char*>());
    }

    if (depJson["actualDateTime"].is<String>()) {
        departure.actualDateTime = String(depJson["actualDateTime"].as<const char*>());
    } else {
        departure.actualDateTime = departure.plannedDateTime;
    }

    if (depJson["plannedTrack"].is<String>()) {
        departure.plannedTrack = String(depJson["plannedTrack"].as<const char*>());
    }

    if (depJson["actualTrack"].is<String>()) {
        departure.actualTrack = String(depJson["actualTrack"].as<const char*>());
    } else {
        departure.actualTrack = departure.plannedTrack;
    }

    if (depJson["cancelled"].is<bool>()) {
        departure.cancelled = depJson["cancelled"].as<bool>();
    }

    if (depJson["departureStatus"].is<String>()) {
        departure.departureStatus = String(depJson["departureStatus"].as<const char*>());
    }

    if (depJson["product"].is<JsonObject>()) {
        JsonObject product = depJson["product"];
        if (product["shortCategoryName"].is<String>()) {
            departure.trainType = String(product["shortCategoryName"].as<const char*>());
        }
    }

    if (timeManager) {
        departure.plannedTime = timeManager->parseISO8601(departure.plannedDateTime);
        departure.actualTime  = timeManager->parseISO8601(departure.actualDateTime);

        if (departure.plannedTime > 0 && departure.actualTime > 0) {
            departure.delayMinutes = (departure.actualTime - departure.plannedTime) / 60;
        } else {
            departure.delayMinutes = 0;
        }
    } else {
        Serial.println("WARNING: No TimeManager - times not parsed!");
        departure.plannedTime  = 0;
        departure.actualTime   = 0;
        departure.delayMinutes = 0;
    }

    return true;
}

const Departure& NSApiClient::getDeparture(int index) const {
    static Departure emptyDeparture;

    if (index < 0 || index >= departureCount) {
        return emptyDeparture;
    }

    return departures[index];
}

int NSApiClient::getDepartureCount() const {
    return departureCount;
}

const Departure* NSApiClient::getNextDeparture(unsigned long currentTime, int travelTimeMinutes, int bufferTimeMinutes) const {
    for (int i = 0; i < departureCount; i++) {
        const Departure& dep = departures[i];

        if (dep.cancelled) continue;

        // Always use planned time — schedule determines if a train has "left", not its delay
        unsigned long depTime = dep.plannedTime;

        if (depTime == 0) continue;

        // 30-second grace: don't skip a train the instant its scheduled time passes
        if (depTime + 30 < currentTime) continue;

        if (travelTimeMinutes > 0 || bufferTimeMinutes > 0) {
            unsigned long leaveTime = depTime - ((travelTimeMinutes + bufferTimeMinutes) * 60);
            if (leaveTime + 30 < currentTime) continue;
        }

        return &dep;
    }

    return nullptr;
}

void NSApiClient::clearDepartures() {
    departureCount = 0;
    for (int i = 0; i < 10; i++) {
        departures[i] = Departure();
    }
}

bool NSApiClient::isLastRequestSuccessful() const {
    return lastRequestSuccessful;
}

String NSApiClient::getLastError() const {
    return lastError;
}
