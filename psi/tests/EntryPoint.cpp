
#include "psi_mock_tests.h"
#include "psi_test_tests.h"

int main()
{
    using namespace psi::test;
    {
        MockedFn_Tests test;
        test.create_test();
        test.fn_test();
        test.f_test();
    }

    {
        FnExpectation_Tests test;
        test.ctor_test();
        test.dtor_test();
        test.verify_test();
    }

    {
        TestLib_Tests test;
        test.init_test();
        test.verify_expectations_test();
        test.fn_expectations_test();
    }

    EXPECT_EQ_test();
    EXPECT_GE_test();
    EXPECT_LE_test();
    EXPECT_CALL_test();
    MOCK_VERIFY_EXPECTATIONS_test();
}
