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

    // Use dynamic JSON document for large responses
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        lastError = "JSON parse error: " + String(error.c_str());
        return false;
    }

    // Check if payload exists (ArduinoJson v7 syntax)
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

    // Parse each departure
    for (JsonObject depJson : departuresArray) {
        if (departureCount >= MAX_DEPARTURES) {
            break;  // Max cached departures
        }

        if (parseDeparture(depJson, departures[departureCount])) {
            departureCount++;
        }
    }

    return true;
}

bool NSApiClient::parseDeparture(JsonObject depJson, Departure& departure) {
    // Extract required fields (ArduinoJson v7 syntax)
    if (!depJson["direction"].is<String>() || !depJson["plannedDateTime"].is<String>()) {
        return false;  // Skip invalid departures
    }

    // Use String() constructor to ensure proper copy (not reference)
    departure.direction = String(depJson["direction"].as<const char*>());
    departure.plannedDateTime = String(depJson["plannedDateTime"].as<const char*>());

    // Filter: Only show departures to Den Haag Centraal or Leiden Centraal
    if (departure.direction != "Den Haag Centraal" && departure.direction != "Leiden Centraal") {
        return false;  // Skip this departure
    }

    // Optional fields
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

    // Extract train type from product info
    if (depJson["product"].is<JsonObject>()) {
        JsonObject product = depJson["product"];
        if (product["shortCategoryName"].is<String>()) {
            departure.trainType = String(product["shortCategoryName"].as<const char*>());
        }
    }

    // Parse timestamps using TimeManager
    if (timeManager) {
        departure.plannedTime = timeManager->parseISO8601(departure.plannedDateTime);
        departure.actualTime = timeManager->parseISO8601(departure.actualDateTime);

        // Debug: Show parsed times
        Serial.print("  Parsed planned: ");
        Serial.print(departure.plannedTime);
        Serial.print(" (");
        Serial.print(departure.plannedDateTime);
        Serial.print("), actual: ");
        Serial.print(departure.actualTime);
        Serial.print(" (");
        Serial.print(departure.actualDateTime);
        Serial.println(")");

        // Calculate delay
        if (departure.plannedTime > 0 && departure.actualTime > 0) {
            departure.delayMinutes = (departure.actualTime - departure.plannedTime) / 60;
        } else {
            departure.delayMinutes = 0;
        }
    } else {
        Serial.println("  WARNING: No TimeManager - times not parsed!");
        // No TimeManager available, store 0
        departure.plannedTime = 0;
        departure.actualTime = 0;
        departure.delayMinutes = 0;
    }

    return true;
}

int NSApiClient::calculateDelay(unsigned long planned, unsigned long actual) {
    if (actual <= planned) {
        return 0;
    }

    return (int)((actual - planned) / 60);  // Convert seconds to minutes
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

        // Skip cancelled trains
        if (dep.cancelled) {
            continue;
        }

        // ALWAYS use planned time to determine if train has departed
        // (We need to show next train based on schedule, not delays)
        unsigned long depTime = dep.plannedTime;

        // If planned time not available, skip this departure
        if (depTime == 0) {
            continue;
        }

        // Skip if already departed (with 30 second grace period)
        if (depTime + 30 < currentTime) {
            Serial.print("[SKIP] Train to ");
            Serial.print(dep.direction);
            Serial.print(" already departed (");
            Serial.print(depTime);
            Serial.print(" < ");
            Serial.print(currentTime);
            Serial.println(")");
            continue;
        }

        // If travel time provided, check if we can still catch this train
        if (travelTimeMinutes > 0 || bufferTimeMinutes > 0) {
            int totalMinutes = travelTimeMinutes + bufferTimeMinutes;
            unsigned long leaveTime = depTime - (totalMinutes * 60);

            // Skip if leave time has passed (with 30 second grace period)
            if (leaveTime + 30 < currentTime) {
                Serial.print("[SKIP] Train to ");
                Serial.print(dep.direction);
                Serial.print(" - leave time passed (needed to leave at ");
                Serial.print(leaveTime);
                Serial.print(", current: ");
                Serial.print(currentTime);
                Serial.println(")");
                continue;
            }
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
