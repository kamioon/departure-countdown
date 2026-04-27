#include "config.h"

void ConfigManager::loadDefaults() {
    config = Config(); // Use default constructor values
}

Config& ConfigManager::getConfig() {
    return config;
}

void ConfigManager::setNsApiKey(const String& apiKey) {
    strncpy(config.nsApiKey, apiKey.c_str(), sizeof(config.nsApiKey) - 1);
    config.nsApiKey[sizeof(config.nsApiKey) - 1] = '\0';
}

void ConfigManager::setWiFiCredentials(const String& ssid, const String& password) {
    strncpy(config.wifiSsid, ssid.c_str(), sizeof(config.wifiSsid) - 1);
    config.wifiSsid[sizeof(config.wifiSsid) - 1] = '\0';
    strncpy(config.wifiPassword, password.c_str(), sizeof(config.wifiPassword) - 1);
    config.wifiPassword[sizeof(config.wifiPassword) - 1] = '\0';
}

void ConfigManager::setStationCode(const String& code) {
    strncpy(config.stationCode, code.c_str(), sizeof(config.stationCode) - 1);
    config.stationCode[sizeof(config.stationCode) - 1] = '\0';
}

void ConfigManager::setTravelTime(TransportMode mode, int minutes) {
    if (minutes < 0) return;

    switch (mode) {
        case WALK:
            config.walkTime = minutes;
            break;
        case BIKE:
            config.bikeTime = minutes;
            break;
        case BUS:
            config.busTime = minutes;
            break;
    }
}

void ConfigManager::setActiveMode(TransportMode mode) {
    config.activeMode = mode;
}

void ConfigManager::setBufferTime(int minutes) {
    if (minutes >= 0) {
        config.bufferTime = minutes;
    }
}

int ConfigManager::getActiveTravelTime() {
    return getTravelTime(config.activeMode);
}

int ConfigManager::getTravelTime(TransportMode mode) {
    switch (mode) {
        case WALK:
            return config.walkTime;
        case BIKE:
            return config.bikeTime;
        case BUS:
            return config.busTime;
        default:
            return 0;
    }
}

void ConfigManager::setAudioAlerts(bool enabled) {
    config.audioAlertsEnabled = enabled;
}

void ConfigManager::setLedAlerts(bool enabled) {
    config.ledAlertsEnabled = enabled;
}

bool ConfigManager::validate() {
    size_t ssidLen = strlen(config.wifiSsid);
    size_t passLen = strlen(config.wifiPassword);

    if (ssidLen > 32) {
        Serial.println("Invalid WiFi SSID length");
        return false;
    }

    if (passLen > 64) {
        Serial.println("Invalid WiFi password length");
        return false;
    }

    if (ssidLen == 0 && passLen > 0) {
        Serial.println("WiFi password set without SSID");
        return false;
    }

    // Validate station code (should be 2-4 characters)
    if (strlen(config.stationCode) < 2 || strlen(config.stationCode) > 4) {
        Serial.println("Invalid station code");
        return false;
    }

    // Validate travel times (should be positive and reasonable)
    if (config.walkTime < 0 || config.walkTime > 120) {
        Serial.println("Invalid walk time");
        return false;
    }

    if (config.bikeTime < 0 || config.bikeTime > 120) {
        Serial.println("Invalid bike time");
        return false;
    }

    if (config.busTime < 0 || config.busTime > 120) {
        Serial.println("Invalid bus time");
        return false;
    }

    // Validate buffer time (0-30 minutes)
    if (config.bufferTime < 0 || config.bufferTime > 30) {
        Serial.println("Invalid buffer time");
        return false;
    }

    return true;
}

String ConfigManager::getModeName(TransportMode mode) {
    switch (mode) {
        case WALK:
            return "Walk";
        case BIKE:
            return "Bike";
        case BUS:
            return "Bus";
        default:
            return "Unknown";
    }
}

String ConfigManager::getModeIcon(TransportMode mode) {
    switch (mode) {
        case WALK:
            return "W";
        case BIKE:
            return "B";
        case BUS:
            return "U";
        default:
            return "?";
    }
}
