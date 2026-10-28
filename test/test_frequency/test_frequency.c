#include <unity.h>

#include "frequency.h"

void setUp(void) {}
void tearDown(void) {}

// test de omzet van afstand cm naar freq Hz
static void test_FRQ_end_points(void)
{
    TEST_ASSERT_EQUAL_UINT16(1400, CalculateFrequency(0));
    TEST_ASSERT_EQUAL_UINT16(230, CalculateFrequency(65));
}

// testen op basis van meegegeven data Excel bestand
static void test_FRQ_reference_values(void)
{
    TEST_ASSERT_EQUAL_UINT16(1310, CalculateFrequency(5));
    TEST_ASSERT_EQUAL_UINT16(1220, CalculateFrequency(10));
    TEST_ASSERT_EQUAL_UINT16(1040, CalculateFrequency(20));
    TEST_ASSERT_EQUAL_UINT16(770, CalculateFrequency(35));
    TEST_ASSERT_EQUAL_UINT16(500, CalculateFrequency(50));
    TEST_ASSERT_EQUAL_UINT16(320, CalculateFrequency(60));
}

// edge cases voor omrekenen
static void test_FRQ_beyond_range_gives_lowest_frequency(void)
{
    TEST_ASSERT_EQUAL_UINT16(230, CalculateFrequency(66));
    TEST_ASSERT_EQUAL_UINT16(230, CalculateFrequency(200));
    TEST_ASSERT_EQUAL_UINT16(230, CalculateFrequency(65535));
}

// alle waardes tussen 230 en 1400 HZ
static void test_FRQ_always_between_230_and_1400(void)
{
    for (uint16_t d = 0; d <= 1000; d++)
    {
        uint16_t f = CalculateFrequency(d);
        TEST_ASSERT_TRUE(f >= 230);
        TEST_ASSERT_TRUE(f <= 1400);
    }
}

// grotere waarde geeft nooit hogere freq
static void test_FRQ_never_rises_with_distance(void)
{
    for (uint16_t d = 1; d <= 100; d++)
    {
        TEST_ASSERT_TRUE(CalculateFrequency(d) <= CalculateFrequency(d - 1));
    }
}

// is afstand in range
static void test_FRQ_distance_range(void)
{
    TEST_ASSERT_TRUE(IsDistanceInRange(0));
    TEST_ASSERT_TRUE(IsDistanceInRange(1));
    TEST_ASSERT_TRUE(IsDistanceInRange(2));
    TEST_ASSERT_TRUE(IsDistanceInRange(65));
    TEST_ASSERT_FALSE(IsDistanceInRange(66));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_FRQ_end_points);
    RUN_TEST(test_FRQ_reference_values);
    RUN_TEST(test_FRQ_beyond_range_gives_lowest_frequency);
    RUN_TEST(test_FRQ_always_between_230_and_1400);
    RUN_TEST(test_FRQ_never_rises_with_distance);
    RUN_TEST(test_FRQ_distance_range);
    return UNITY_END();
}
