#include "config.h"
#include <Preferences.h>

ConfigManager::ConfigManager() {
    preferences = nullptr;
}

ConfigManager::~ConfigManager() {
    if (preferences) {
        preferences->end();
        delete preferences;
    }
}

bool ConfigManager::begin() {
    preferences = new Preferences();
    if (!preferences) {
        Serial.println("Failed to create Preferences object");
        return false;
    }

    // Try to load saved config, otherwise use defaults
    if (!load()) {
        Serial.println("No saved config found, using defaults");
        loadDefaults();
        save();
    }

    return true;
}

bool ConfigManager::load() {
    if (!preferences) return false;

    if (!preferences->begin(NAMESPACE, true)) { // Read-only
        return false;
    }

    // Load all configuration values
    preferences->getString("wifiSsid", config.wifiSsid, sizeof(config.wifiSsid));
    preferences->getString("wifiPass", config.wifiPassword, sizeof(config.wifiPassword));
    preferences->getString("nsApiKey", config.nsApiKey, sizeof(config.nsApiKey));
    preferences->getString("stationCode", config.stationCode, sizeof(config.stationCode));
    config.walkTime = preferences->getInt("walkTime", 15);
    config.bikeTime = preferences->getInt("bikeTime", 8);
    config.busTime = preferences->getInt("busTime", 12);
    config.activeMode = (TransportMode)preferences->getInt("activeMode", WALK);
    config.bufferTime = preferences->getInt("bufferTime", 2);
    config.audioAlertsEnabled = preferences->getBool("audioAlerts", false);
    config.ledAlertsEnabled = preferences->getBool("ledAlerts", true);
    config.alertAtLeaveTime = preferences->getBool("alertLeave", true);
    config.alertFiveMinBefore = preferences->getBool("alert5min", true);

    preferences->end();

    Serial.println("Configuration loaded from storage");
    return true;
}

bool ConfigManager::save() {
    if (!preferences) return false;

    if (!validate()) {
        Serial.println("Invalid configuration, cannot save");
        return false;
    }

    if (!preferences->begin(NAMESPACE, false)) { // Read-write
        Serial.println("Failed to open preferences for writing");
        return false;
    }

    // Save all configuration values
    preferences->putString("wifiSsid", config.wifiSsid);
    preferences->putString("wifiPass", config.wifiPassword);
    preferences->putString("nsApiKey", config.nsApiKey);
    preferences->putString("stationCode", config.stationCode);
    preferences->putInt("walkTime", config.walkTime);
    preferences->putInt("bikeTime", config.bikeTime);
    preferences->putInt("busTime", config.busTime);
    preferences->putInt("activeMode", config.activeMode);
    preferences->putInt("bufferTime", config.bufferTime);
    preferences->putBool("audioAlerts", config.audioAlertsEnabled);
    preferences->putBool("ledAlerts", config.ledAlertsEnabled);
    preferences->putBool("alertLeave", config.alertAtLeaveTime);
    preferences->putBool("alert5min", config.alertFiveMinBefore);

    preferences->end();

    Serial.println("Configuration saved to storage");
    return true;
}

bool ConfigManager::factoryReset() {
    if (!preferences) return false;

    if (!preferences->begin(NAMESPACE, false)) {
        return false;
    }

    bool success = preferences->clear();
    preferences->end();

    if (success) {
        loadDefaults();
        Serial.println("Factory reset completed");
    }

    return success;
}
