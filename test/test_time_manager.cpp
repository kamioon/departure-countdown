#include <unity.h>
#include "../include/time_manager.h"

TimeManager* timeManager;

void setUp(void) {
    timeManager = new TimeManager();
    timeManager->begin();
}

void tearDown(void) {
    delete timeManager;
}

// Test ISO8601 parsing
void test_parse_iso8601_valid(void) {
    String isoString = "2025-12-24T23:28:00+0100";
    unsigned long timestamp = timeManager->parseISO8601(isoString);

    // Should be a valid timestamp (not zero)
    TEST_ASSERT_GREATER_THAN(0, timestamp);

    // Rough check: should be around Dec 2025
    // Unix timestamp for 2025-12-24 is approximately 1766620800
    TEST_ASSERT_GREATER_THAN(1700000000, timestamp);  // After 2023
    TEST_ASSERT_LESS_THAN(1800000000, timestamp);     // Before 2027
}

void test_parse_iso8601_with_negative_offset(void) {
    String isoString = "2025-12-24T23:28:00-0500";
    unsigned long timestamp = timeManager->parseISO8601(isoString);
    TEST_ASSERT_GREATER_THAN(0, timestamp);
}

void test_parse_iso8601_without_offset(void) {
    String isoString = "2025-12-24T23:28:00";
    unsigned long timestamp = timeManager->parseISO8601(isoString);
    TEST_ASSERT_GREATER_THAN(0, timestamp);
}

void test_parse_iso8601_invalid_format(void) {
    String isoString = "not-a-date";
    unsigned long timestamp = timeManager->parseISO8601(isoString);
    TEST_ASSERT_EQUAL(0, timestamp);
}

void test_parse_iso8601_empty_string(void) {
    String isoString = "";
    unsigned long timestamp = timeManager->parseISO8601(isoString);
    TEST_ASSERT_EQUAL(0, timestamp);
}

// Test time formatting
void test_format_time(void) {
    // Timestamp for 2025-01-01 12:34:56
    // Note: This is timezone dependent
    unsigned long timestamp = 1735740896;

    String formatted = timeManager->formatTime(timestamp);

    // Should be in HH:MM:SS format
    TEST_ASSERT_EQUAL(8, formatted.length());
    TEST_ASSERT_EQUAL(':', formatted.charAt(2));
    TEST_ASSERT_EQUAL(':', formatted.charAt(5));
}

void test_format_time_short(void) {
    unsigned long timestamp = 1735740896;

    String formatted = timeManager->formatTimeShort(timestamp);

    // Should be in HH:MM format
    TEST_ASSERT_EQUAL(5, formatted.length());
    TEST_ASSERT_EQUAL(':', formatted.charAt(2));
}

// Test seconds difference calculation
void test_get_seconds_difference_future(void) {
    unsigned long past = 1000;
    unsigned long future = 1500;

    long diff = timeManager->getSecondsDifference(future, past);
    TEST_ASSERT_EQUAL(500, diff);
}

void test_get_seconds_difference_past(void) {
    unsigned long past = 1500;
    unsigned long future = 1000;

    // Future is in past, should return 0
    long diff = timeManager->getSecondsDifference(future, past);
    TEST_ASSERT_EQUAL(0, diff);
}

void test_get_seconds_difference_same(void) {
    unsigned long time = 1000;

    long diff = timeManager->getSecondsDifference(time, time);
    TEST_ASSERT_EQUAL(0, diff);
}

void test_get_seconds_difference_from_current(void) {
    // When past is 0 (default), should use current time
    unsigned long future = timeManager->getCurrentTime() + 100;

    long diff = timeManager->getSecondsDifference(future, 0);

    // Should be around 100 seconds
    TEST_ASSERT_GREATER_THAN(90, diff);
    TEST_ASSERT_LESS_THAN(110, diff);
}

// Test time sync status
void test_is_time_synced(void) {
    // In test mode, time should be synced after begin()
    TEST_ASSERT_TRUE(timeManager->isTimeSynced());
}

void test_get_current_time(void) {
    unsigned long timestamp = timeManager->getCurrentTime();

    // Should return a valid timestamp
    TEST_ASSERT_GREATER_THAN(0, timestamp);

    // Should be a reasonable value (after 2020, before 2030)
    TEST_ASSERT_GREATER_THAN(1600000000, timestamp);  // After Sept 2020
    TEST_ASSERT_LESS_THAN(1900000000, timestamp);     // Before May 2030
}

// Test multiple ISO8601 formats
void test_parse_various_iso_formats(void) {
    struct TestCase {
        const char* input;
        bool shouldSucceed;
    };

    TestCase testCases[] = {
        {"2025-01-01T00:00:00+0000", true},
        {"2025-12-31T23:59:59+0100", true},
        {"2024-06-15T12:30:45-0500", true},
        {"2025-01-01T10:00:00", true},
        {"invalid", false},
        {"2025-13-01T00:00:00+0000", false},  // Invalid month
        {"", false}
    };

    for (int i = 0; i < 7; i++) {
        unsigned long result = timeManager->parseISO8601(String(testCases[i].input));

        if (testCases[i].shouldSucceed) {
            TEST_ASSERT_GREATER_THAN_MESSAGE(0, result, testCases[i].input);
        } else {
            TEST_ASSERT_EQUAL_MESSAGE(0, result, testCases[i].input);
        }
    }
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_parse_iso8601_valid);
    RUN_TEST(test_parse_iso8601_with_negative_offset);
    RUN_TEST(test_parse_iso8601_without_offset);
    RUN_TEST(test_parse_iso8601_invalid_format);
    RUN_TEST(test_parse_iso8601_empty_string);
    RUN_TEST(test_format_time);
    RUN_TEST(test_format_time_short);
    RUN_TEST(test_get_seconds_difference_future);
    RUN_TEST(test_get_seconds_difference_past);
    RUN_TEST(test_get_seconds_difference_same);
    RUN_TEST(test_get_seconds_difference_from_current);
    RUN_TEST(test_is_time_synced);
    RUN_TEST(test_get_current_time);
    RUN_TEST(test_parse_various_iso_formats);

    return UNITY_END();
}
