#include "ns_api.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

int NSApiClient::fetchDepartures(const String& stationCode, int maxDepartures) {
    if (apiKey.length() == 0) {
        lastError = "API key not set";
        lastRequestSuccessful = false;
        return -1;
    }

    if (stationCode.length() < 2) {
        lastError = "Invalid station code";
        lastRequestSuccessful = false;
        return -1;
    }

    // Create secure WiFi client for HTTPS
    WiFiClientSecure *client = new WiFiClientSecure;
    if (!client) {
        lastError = "Failed to create secure client";
        lastRequestSuccessful = false;
        return -1;
    }

    // Skip certificate validation (for testing)
    // In production, you should use proper certificates
    client->setInsecure();

    // Set longer timeout for SSL handshake (helps with slow DNS)
    client->setTimeout(15);  // 15 seconds for SSL handshake

    int requestCount = maxDepartures;
    if (requestCount > MAX_DEPARTURES) {
        requestCount = MAX_DEPARTURES;
    }

    HTTPClient http;
    String url = getApiUrl(stationCode, requestCount);

    Serial.println("Fetching departures from: " + url);

    if (!http.begin(*client, url)) {
        lastError = "Failed to begin HTTP request";
        lastRequestSuccessful = false;
        delete client;
        return -1;
    }

    http.addHeader("Ocp-Apim-Subscription-Key", apiKey);
    http.setTimeout(REQUEST_TIMEOUT);

    int httpCode = http.GET();

    if (httpCode != 200) {
        lastError = "HTTP error: " + String(httpCode);
        lastRequestSuccessful = false;
        http.end();
        delete client;
        Serial.println("HTTP GET failed: " + lastError);
        return -1;
    }

    // Get response payload
    String payload = http.getString();

    // Parse BEFORE closing HTTP connection to ensure data validity
    bool parseSuccess = parseResponse(payload);

    // Now safe to cleanup HTTP resources
    http.end();
    delete client;

    if (!parseSuccess) {
        lastRequestSuccessful = false;
        Serial.println("Failed to parse response: " + lastError);
        return -1;
    }

    lastRequestSuccessful = true;
    lastError = "";

    Serial.print("Successfully fetched ");
    Serial.print(departureCount);
    Serial.println(" departures");

    return departureCount;
}
