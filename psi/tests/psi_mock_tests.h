
#pragma once

#include "psi/test/psi_mock.h"

namespace psi::test {

#define terminate_test(error)                                                                                          \
    std::cout << __FILE__ << ":" << __LINE__ << " " << error << std::endl;                                             \
    std::terminate()

void EXPECT_EQ_test();
void EXPECT_GE_test();
void EXPECT_LE_test();
void EXPECT_CALL_test();
void MOCK_VERIFY_EXPECTATIONS_test();

static void run_test(std::function<void()> test_fn, const std::string &test_name)
{
    TestLib::init();
    TestLib::add_test({"PsiMock_Tests", test_name, test_fn});
    TestLib::run();
    TestLib::destroy();
}

void EXPECT_EQ_test()
{
    auto test_fn = []() {
        EXPECT_EQ(10, 10);
        EXPECT_EQ(10, 11);
        EXPECT_EQ(11, 10);
    };
    run_test(test_fn, "EXPECT_EQ_test");
}

void EXPECT_GE_test()
{
    auto test_fn = []() {
        EXPECT_GE(10, 10);
        EXPECT_GE(10, 11);
        EXPECT_GE(11, 10);
    };
    run_test(test_fn, "EXPECT_GE_test");
}

void EXPECT_LE_test()
{
    auto test_fn = []() {
        EXPECT_LE(10, 10);
        EXPECT_LE(10, 11);
        EXPECT_LE(11, 10);
    };
    run_test(test_fn, "EXPECT_LE_test");
}

void EXPECT_CALL_test()
{
    auto test_fn = []() {
        auto test_int_fn = MockedFn<std::function<int(double)>>::create();
        EXPECT_CALL(test_int_fn, 1);
        test_int_fn->fn()(10.0);
        TestLib::verify_expectations();
    };
    run_test(test_fn, "EXPECT_CALL_test");
}

void MOCK_VERIFY_EXPECTATIONS_test() {}

} // namespace psi::test
