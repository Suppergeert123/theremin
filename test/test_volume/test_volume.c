#include <unity.h>

#include "volume_calc.h"

void setUp(void) {}
void tearDown(void) {}

// test de omzet van adc waarde naar procent
static void test_VOL_end_points(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, AdcToPercent(0));
    TEST_ASSERT_EQUAL_UINT8(100, AdcToPercent(255));
}

// test het middenpunt van adc waarde naar procent
static void test_VOL_middle_is_fifty_percent(void)
{
    uint8_t percent = AdcToPercent(128);
    TEST_ASSERT_TRUE(percent >= 49);
    TEST_ASSERT_TRUE(percent <= 51);
}

// kijk of percentage omhoog blijft gaan met stijging
static void test_VOL_percent_is_in_range_and_rises(void)
{
    uint8_t previous = 0;
    for (uint16_t adc = 0; adc <= 255; adc++)
    {
        uint8_t percent = AdcToPercent((uint8_t)adc);
        TEST_ASSERT_TRUE(percent <= 100);
        TEST_ASSERT_TRUE(percent >= previous);
        previous = percent;
    }
}

// percentages omzetten naar adc waardes
static void test_VOL_percent_to_duty(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, PercentToDuty(0));
    TEST_ASSERT_EQUAL_UINT8(255, PercentToDuty(100));
    TEST_ASSERT_TRUE(PercentToDuty(50) >= 127);
    TEST_ASSERT_TRUE(PercentToDuty(50) <= 128);
}

// meer dan bovenwaarde wordt als max gezien
static void test_VOL_more_than_100_percent_is_limited(void)
{
    TEST_ASSERT_EQUAL_UINT8(255, PercentToDuty(101));
    TEST_ASSERT_EQUAL_UINT8(255, PercentToDuty(255));
}

// extra testen op percentages en of het blijft stijgen met de percentages mee
static void test_VOL_conversion_is_consistent(void)
{
    uint8_t previous = 0;
    for (uint8_t percent = 0; percent <= 100; percent++)
    {
        uint8_t duty = PercentToDuty(percent);
        TEST_ASSERT_TRUE(duty >= previous);
        TEST_ASSERT_EQUAL_UINT8(percent, AdcToPercent(duty));
        previous = duty;
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_VOL_end_points);
    RUN_TEST(test_VOL_middle_is_fifty_percent);
    RUN_TEST(test_VOL_percent_is_in_range_and_rises);
    RUN_TEST(test_VOL_percent_to_duty);
    RUN_TEST(test_VOL_more_than_100_percent_is_limited);
    RUN_TEST(test_VOL_conversion_is_consistent);
    return UNITY_END();
}
