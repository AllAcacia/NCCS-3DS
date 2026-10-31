#include "unity.h"
#include "nccs_protocol.h"


void setUp(void)
{
    return;
}


void tearDown(void)
{
    return;
}


// Basic test implementation
void test_basic(void)
{
    int result = 1;
    printf("Success!\n");
    TEST_ASSERT_EQUAL(1, result);
}


int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_basic);

    return UNITY_END();
}