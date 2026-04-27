#include <unity.h>
#include "../include/ns_api.h"

NSApiClient* apiClient;

void setUp(void) {
    apiClient = new NSApiClient();
}

void tearDown(void) {
    delete apiClient;
}

// Test initialization
void test_begin_with_valid_key(void) {
    bool result = apiClient->begin("test-api-key-12345");
    TEST_ASSERT_TRUE(result);
}

void test_begin_with_empty_key(void) {
    bool result = apiClient->begin("");
    TEST_ASSERT_FALSE(result);
}

// Test API URL generation
void test_get_api_url(void) {
    String url = NSApiClient::getApiUrl("HTNC", 5);
    TEST_ASSERT_TRUE(url.indexOf("station=HTNC") > 0);
    TEST_ASSERT_TRUE(url.indexOf("maxJourneys=5") > 0);
}

void test_get_api_url_without_max_journeys(void) {
    String url = NSApiClient::getApiUrl("UT", 0);
    TEST_ASSERT_TRUE(url.indexOf("station=UT") > 0);
    TEST_ASSERT_FALSE(url.indexOf("maxJourneys") > 0);
}

// Test departure management
void test_get_departure_count_initial(void) {
    TEST_ASSERT_EQUAL(0, apiClient->getDepartureCount());
}

void test_get_departure_invalid_index(void) {
    const Departure& dep = apiClient->getDeparture(5);
    TEST_ASSERT_EQUAL_STRING("", dep.direction.c_str());
}

void test_clear_departures(void) {
    apiClient->clearDepartures();
    TEST_ASSERT_EQUAL(0, apiClient->getDepartureCount());
}

// Test error handling
void test_last_request_status_initial(void) {
    TEST_ASSERT_FALSE(apiClient->isLastRequestSuccessful());
}

void test_set_api_key(void) {
    apiClient->setApiKey("new-api-key");
    // Can't directly test, but should not crash
    TEST_ASSERT_TRUE(true);
}

// Test getNextDeparture with empty list
void test_get_next_departure_empty(void) {
    unsigned long currentTime = 1735000000;
    const Departure* dep = apiClient->getNextDeparture(currentTime);
    TEST_ASSERT_NULL(dep);
}

// Test Departure structure
void test_departure_default_constructor(void) {
    Departure dep;
    TEST_ASSERT_EQUAL(0, dep.plannedTime);
    TEST_ASSERT_EQUAL(0, dep.actualTime);
    TEST_ASSERT_FALSE(dep.cancelled);
    TEST_ASSERT_EQUAL(0, dep.delayMinutes);
    TEST_ASSERT_EQUAL_STRING("", dep.direction.c_str());
}

// Note: Testing actual API calls requires network connectivity
// and a valid API key, so those tests are omitted for unit tests.
// Integration tests should cover API functionality.

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_begin_with_valid_key);
    RUN_TEST(test_begin_with_empty_key);
    RUN_TEST(test_get_api_url);
    RUN_TEST(test_get_api_url_without_max_journeys);
    RUN_TEST(test_get_departure_count_initial);
    RUN_TEST(test_get_departure_invalid_index);
    RUN_TEST(test_clear_departures);
    RUN_TEST(test_last_request_status_initial);
    RUN_TEST(test_set_api_key);
    RUN_TEST(test_get_next_departure_empty);
    RUN_TEST(test_departure_default_constructor);

    return UNITY_END();
}
