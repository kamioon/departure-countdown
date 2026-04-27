#include "config.h"

ConfigManager::ConfigManager() {
    preferences = nullptr;
}

ConfigManager::~ConfigManager() {
}

bool ConfigManager::begin() {
    loadDefaults();
    return true;
}

bool ConfigManager::load() {
    loadDefaults();
    return true;
}

bool ConfigManager::save() {
    return validate();
}

bool ConfigManager::factoryReset() {
    loadDefaults();
    return true;
}
