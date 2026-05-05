#include <unity.h>
#include "../include/config.h"

ConfigManager* configManager;

void setUp(void) {
    configManager = new ConfigManager();
    configManager->begin();
}

void tearDown(void) {
    delete configManager;
}

// Test default configuration
void test_default_config(void) {
    Config& config = configManager->getConfig();

    TEST_ASSERT_EQUAL_STRING("", config.wifiSsid);
    TEST_ASSERT_EQUAL_STRING("", config.wifiPassword);
    TEST_ASSERT_EQUAL_STRING("", config.stationCode);
    TEST_ASSERT_EQUAL(15, config.walkTime);
    TEST_ASSERT_EQUAL(8, config.bikeTime);
    TEST_ASSERT_EQUAL(12, config.busTime);
    TEST_ASSERT_EQUAL(WALK, config.activeMode);
    TEST_ASSERT_EQUAL(2, config.bufferTime);
    TEST_ASSERT_TRUE(config.audioAlertsEnabled);
    TEST_ASSERT_TRUE(config.ledAlertsEnabled);
}

// Test setters
void test_set_station_code(void) {
    configManager->setStationCode("UT");
    TEST_ASSERT_EQUAL_STRING("UT", configManager->getConfig().stationCode);
}

void test_set_travel_times(void) {
    configManager->setTravelTime(WALK, 20);
    configManager->setTravelTime(BIKE, 10);
    configManager->setTravelTime(BUS, 15);

    TEST_ASSERT_EQUAL(20, configManager->getTravelTime(WALK));
    TEST_ASSERT_EQUAL(10, configManager->getTravelTime(BIKE));
    TEST_ASSERT_EQUAL(15, configManager->getTravelTime(BUS));
}

void test_set_active_mode(void) {
    configManager->setActiveMode(BIKE);
    TEST_ASSERT_EQUAL(BIKE, configManager->getConfig().activeMode);
    TEST_ASSERT_EQUAL(8, configManager->getActiveTravelTime());

    configManager->setActiveMode(BUS);
    TEST_ASSERT_EQUAL(BUS, configManager->getConfig().activeMode);
    TEST_ASSERT_EQUAL(12, configManager->getActiveTravelTime());
}

void test_set_buffer_time(void) {
    configManager->setBufferTime(5);
    TEST_ASSERT_EQUAL(5, configManager->getConfig().bufferTime);
}

void test_set_api_key(void) {
    configManager->setNsApiKey("test-api-key-123");
    TEST_ASSERT_EQUAL_STRING("test-api-key-123", configManager->getConfig().nsApiKey);
}

void test_set_wifi_credentials(void) {
    configManager->setWiFiCredentials("MyWifi", "SuperSecret");
    TEST_ASSERT_EQUAL_STRING("MyWifi", configManager->getConfig().wifiSsid);
    TEST_ASSERT_EQUAL_STRING("SuperSecret", configManager->getConfig().wifiPassword);
}

// Test validation
void test_validate_valid_config(void) {
    TEST_ASSERT_TRUE(configManager->validate());
}

void test_validate_invalid_station_code(void) {
    configManager->setStationCode("X");  // Too short
    TEST_ASSERT_FALSE(configManager->validate());

    configManager->setStationCode("TOOLONG");  // Too long
    TEST_ASSERT_FALSE(configManager->validate());
}

void test_validate_invalid_travel_time(void) {
    configManager->setTravelTime(WALK, 150);  // Too long
    TEST_ASSERT_FALSE(configManager->validate());

    configManager->setTravelTime(WALK, -5);  // Negative
    TEST_ASSERT_FALSE(configManager->validate());
}

// Test mode name/icon helpers
void test_get_mode_name(void) {
    TEST_ASSERT_EQUAL_STRING("Walk", ConfigManager::getModeName(WALK).c_str());
    TEST_ASSERT_EQUAL_STRING("Bike", ConfigManager::getModeName(BIKE).c_str());
    TEST_ASSERT_EQUAL_STRING("Bus", ConfigManager::getModeName(BUS).c_str());
}

void test_get_mode_icon(void) {
    TEST_ASSERT_EQUAL_STRING("W", ConfigManager::getModeIcon(WALK).c_str());
    TEST_ASSERT_EQUAL_STRING("B", ConfigManager::getModeIcon(BIKE).c_str());
    TEST_ASSERT_EQUAL_STRING("U", ConfigManager::getModeIcon(BUS).c_str());
}

// Test factory reset
void test_factory_reset(void) {
    // Modify configuration
    configManager->setStationCode("ASD");
    configManager->setTravelTime(WALK, 25);
    configManager->setActiveMode(BIKE);

    // Factory reset
    configManager->factoryReset();

    // Should be back to defaults
    Config& config = configManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("", config.stationCode);
    TEST_ASSERT_EQUAL(15, config.walkTime);
    TEST_ASSERT_EQUAL(WALK, config.activeMode);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_default_config);
    RUN_TEST(test_set_station_code);
    RUN_TEST(test_set_travel_times);
    RUN_TEST(test_set_active_mode);
    RUN_TEST(test_set_buffer_time);
    RUN_TEST(test_set_api_key);
    RUN_TEST(test_set_wifi_credentials);
    RUN_TEST(test_validate_valid_config);
    RUN_TEST(test_validate_invalid_station_code);
    RUN_TEST(test_validate_invalid_travel_time);
    RUN_TEST(test_get_mode_name);
    RUN_TEST(test_get_mode_icon);
    RUN_TEST(test_factory_reset);

    return UNITY_END();
}
