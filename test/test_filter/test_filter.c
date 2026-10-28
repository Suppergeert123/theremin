
#include <unity.h>
#include "filter.h"

void setUp(void)
{
    FreeFilter();
}

void tearDown(void)
{
    FreeFilter();
}

// zonder init of filterwaardes
void test_FilterWithoutInit_ReturnsInput(void)
{
    TEST_ASSERT_EQUAL_UINT16(123, FilterDistance(123));
}
// test init
void test_InitFilter_ValidSize(void)
{
    TEST_ASSERT_TRUE(InitFilter(3));
    TEST_ASSERT_EQUAL_UINT8(3, GetFilterSize());
}

// invalid start waarde
void test_InitFilter_InvalidEvenSize(void)
{
    TEST_ASSERT_FALSE(InitFilter(2));
    TEST_ASSERT_EQUAL_UINT8(0, GetFilterSize());
}

//eerste waarde moet de gehele tabel vullen
void test_FirstMeasurement_IsReturnedDirectly(void)
{
    TEST_ASSERT_TRUE(InitFilter(3));

    TEST_ASSERT_EQUAL_UINT16(100, FilterDistance(100));
}

// uitschieter wordt gefilterd 
void test_MedianFilter_RejectsOutlier(void)
{
    TEST_ASSERT_TRUE(InitFilter(3));

    FilterDistance(10);  // Eerste meting vult het filter
    TEST_ASSERT_EQUAL_UINT16(10, FilterDistance(100));
}

// reset en bijvullen testen
void test_ResetFilter_RefillsWithNextValue(void)
{
    TEST_ASSERT_TRUE(InitFilter(3));

    FilterDistance(10);
    FilterDistance(20);

    ResetFilter();

    TEST_ASSERT_EQUAL_UINT16(50, FilterDistance(50));
}

// filter grootte na reset
void test_FreeFilter_ResetsSize(void)
{
    TEST_ASSERT_TRUE(InitFilter(3));

    FreeFilter();

    TEST_ASSERT_EQUAL_UINT8(0, GetFilterSize());
    TEST_ASSERT_EQUAL_UINT16(42, FilterDistance(42));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_FilterWithoutInit_ReturnsInput);
    RUN_TEST(test_InitFilter_ValidSize);
    RUN_TEST(test_InitFilter_InvalidEvenSize);
    RUN_TEST(test_FirstMeasurement_IsReturnedDirectly);
    RUN_TEST(test_MedianFilter_RejectsOutlier);
    RUN_TEST(test_ResetFilter_RefillsWithNextValue);
    RUN_TEST(test_FreeFilter_ResetsSize);

    return UNITY_END();
}
