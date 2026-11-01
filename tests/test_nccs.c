#include "unity.h"
#include "nccs_protocol.h"
#include <string.h>


char* sample_usr = "AllAcacia";


void setUp(void)
{
    s16 res = NCCS_Init(0x3000, sample_usr);
}


void tearDown(void)
{
    NCCS_Exit();
}


// TESTS
void test_barebasics(void)
{
    int result = 1;
    TEST_ASSERT_EQUAL_MESSAGE(1, result, "CMake and Unity testing is up!\n");
}


int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_barebasics);

    return UNITY_END();
}