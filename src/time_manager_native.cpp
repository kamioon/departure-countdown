#include "time_manager.h"

TimeManager::TimeManager() : lastSyncTime(0), timeSynced(false) {
    ntpUDP = nullptr;
    timeClient = nullptr;
    rtc = nullptr;
}

TimeManager::~TimeManager() {
}

bool TimeManager::begin() {
    timeSynced = true;
    lastSyncTime = 0;
    return true;
}

bool TimeManager::update() {
    return true;
}

bool TimeManager::forceSync() {
    timeSynced = true;
    lastSyncTime = 0;
    return true;
}

unsigned long TimeManager::getCurrentTime() {
    return static_cast<unsigned long>(time(nullptr));
}

void TimeManager::syncRTCFromNTP() {
}
