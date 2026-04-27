#include <unity.h>
#include "../include/countdown_calc.h"
#include "../include/config.h"
#include "../include/time_manager.h"

ConfigManager* configManager;
TimeManager* timeManager;
CountdownCalculator* calculator;

void setUp(void) {
    configManager = new ConfigManager();
    configManager->begin();

    timeManager = new TimeManager();
    timeManager->begin();

    calculator = new CountdownCalculator();
    calculator->begin(configManager, timeManager);
}

void tearDown(void) {
    delete calculator;
    delete timeManager;
    delete configManager;
}

// Test calculate leave time
void test_calculate_leave_time(void) {
    // Departure at timestamp 1000
    // Travel time: 15 min = 900 seconds
    // Buffer time: 2 min = 120 seconds
    // Leave time should be: 1000 - 900 - 120 = -20 (invalid, returns 0)

    unsigned long departureTime = 10000;
    int travelTime = 10;  // 10 minutes = 600 seconds
    int bufferTime = 2;   // 2 minutes = 120 seconds

    unsigned long leaveTime = CountdownCalculator::calculateLeaveTime(
        departureTime, travelTime, bufferTime);

    // Should be 10000 - 600 - 120 = 9280
    TEST_ASSERT_EQUAL(9280, leaveTime);
}

void test_calculate_leave_time_large_values(void) {
    unsigned long departureTime = 1735000000;  // ~2024-12-24
    int travelTime = 15;  // 900 seconds
    int bufferTime = 2;   // 120 seconds

    unsigned long leaveTime = CountdownCalculator::calculateLeaveTime(
        departureTime, travelTime, bufferTime);

    TEST_ASSERT_EQUAL(1735000000 - 1020, leaveTime);
}

// Test format countdown
void test_format_countdown_zero(void) {
    String result = CountdownCalculator::formatCountdown(0);
    TEST_ASSERT_EQUAL_STRING("00:00", result.c_str());
}

void test_format_countdown_minutes_seconds(void) {
    // 125 seconds = 2 minutes 5 seconds
    String result = CountdownCalculator::formatCountdown(125);
    TEST_ASSERT_EQUAL_STRING("02:05", result.c_str());
}

void test_format_countdown_hours(void) {
    // 3665 seconds = 61 minutes 5 seconds
    String result = CountdownCalculator::formatCountdown(3665);
    TEST_ASSERT_EQUAL_STRING("61:05", result.c_str());
}

void test_format_countdown_long(void) {
    // 3665 seconds = 1:01:05
    String result = CountdownCalculator::formatCountdownLong(3665);
    TEST_ASSERT_EQUAL_STRING("01:01:05", result.c_str());
}

void test_format_countdown_negative(void) {
    // Negative values should be treated as zero
    String result = CountdownCalculator::formatCountdown(-100);
    TEST_ASSERT_EQUAL_STRING("00:00", result.c_str());
}

// Test countdown state determination
void test_get_countdown_state_safe(void) {
    // > 10 minutes = 600 seconds
    CountdownState state = CountdownCalculator::getCountdownState(700);
    TEST_ASSERT_EQUAL(COUNTDOWN_SAFE, state);
}

void test_get_countdown_state_ready(void) {
    // 5-10 minutes
    CountdownState state = CountdownCalculator::getCountdownState(400);  // 6:40
    TEST_ASSERT_EQUAL(COUNTDOWN_READY, state);
}

void test_get_countdown_state_time_to_go(void) {
    // 2-5 minutes
    CountdownState state = CountdownCalculator::getCountdownState(200);  // 3:20
    TEST_ASSERT_EQUAL(COUNTDOWN_TIME_TO_GO, state);
}

void test_get_countdown_state_urgent(void) {
    // < 2 minutes
    CountdownState state = CountdownCalculator::getCountdownState(90);  // 1:30
    TEST_ASSERT_EQUAL(COUNTDOWN_URGENT, state);
}

void test_get_countdown_state_departed(void) {
    // Negative time
    CountdownState state = CountdownCalculator::getCountdownState(-10);
    TEST_ASSERT_EQUAL(COUNTDOWN_DEPARTED, state);
}

// Test state names and messages
void test_get_state_name(void) {
    TEST_ASSERT_EQUAL_STRING("Safe",
        CountdownCalculator::getStateName(COUNTDOWN_SAFE).c_str());
    TEST_ASSERT_EQUAL_STRING("Get Ready",
        CountdownCalculator::getStateName(COUNTDOWN_READY).c_str());
    TEST_ASSERT_EQUAL_STRING("Leave Now!",
        CountdownCalculator::getStateName(COUNTDOWN_URGENT).c_str());
}

void test_get_state_message(void) {
    TEST_ASSERT_EQUAL_STRING("Plenty of time",
        CountdownCalculator::getStateMessage(COUNTDOWN_SAFE).c_str());
    TEST_ASSERT_EQUAL_STRING("LEAVE NOW!",
        CountdownCalculator::getStateMessage(COUNTDOWN_URGENT).c_str());
}

// Test full countdown calculation
void test_calculate_with_departure_time(void) {
    // Set transport mode to WALK (15 min)
    configManager->setActiveMode(WALK);

    // Current time (from TimeManager in test mode)
    unsigned long currentTime = timeManager->getCurrentTime();

    // Departure in 20 minutes
    unsigned long departureTime = currentTime + (20 * 60);

    CountdownInfo info = calculator->calculate(departureTime);

    // Should have valid state
    TEST_ASSERT_NOT_EQUAL(COUNTDOWN_ERROR, info.state);
    TEST_ASSERT_NOT_EQUAL(COUNTDOWN_IDLE, info.state);

    // Leave time should be departure - 15 min (walk) - 2 min (buffer) = 17 min before
    // So seconds until leave should be around 3 minutes (20 - 17)
    TEST_ASSERT_GREATER_THAN(0, info.secondsUntilLeave);
    TEST_ASSERT_LESS_THAN(300, info.secondsUntilLeave);  // Less than 5 minutes

    // State should be URGENT or TIME_TO_GO
    TEST_ASSERT_TRUE(info.state == COUNTDOWN_URGENT ||
                     info.state == COUNTDOWN_TIME_TO_GO);
}

void test_calculate_with_long_departure_time(void) {
    configManager->setActiveMode(BIKE);  // 8 min

    unsigned long currentTime = timeManager->getCurrentTime();
    unsigned long departureTime = currentTime + (60 * 60);  // 1 hour

    CountdownInfo info = calculator->calculate(departureTime);

    TEST_ASSERT_EQUAL(COUNTDOWN_SAFE, info.state);
    TEST_ASSERT_GREATER_THAN(3000, info.secondsUntilLeave);  // > 50 minutes
}

// Test edge cases
void test_calculate_with_past_departure(void) {
    unsigned long currentTime = timeManager->getCurrentTime();
    unsigned long departureTime = currentTime - 100;  // 100 seconds ago

    CountdownInfo info = calculator->calculate(departureTime);

    TEST_ASSERT_EQUAL(COUNTDOWN_DEPARTED, info.state);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_calculate_leave_time);
    RUN_TEST(test_calculate_leave_time_large_values);
    RUN_TEST(test_format_countdown_zero);
    RUN_TEST(test_format_countdown_minutes_seconds);
    RUN_TEST(test_format_countdown_hours);
    RUN_TEST(test_format_countdown_long);
    RUN_TEST(test_format_countdown_negative);
    RUN_TEST(test_get_countdown_state_safe);
    RUN_TEST(test_get_countdown_state_ready);
    RUN_TEST(test_get_countdown_state_time_to_go);
    RUN_TEST(test_get_countdown_state_urgent);
    RUN_TEST(test_get_countdown_state_departed);
    RUN_TEST(test_get_state_name);
    RUN_TEST(test_get_state_message);
    RUN_TEST(test_calculate_with_departure_time);
    RUN_TEST(test_calculate_with_long_departure_time);
    RUN_TEST(test_calculate_with_past_departure);

    return UNITY_END();
}
