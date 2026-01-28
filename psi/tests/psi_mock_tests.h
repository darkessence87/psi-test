
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

void EXPECT_EQ_test()
{
    bool res = EXPECT_EQ(10, 10);
    if (!res) {
        terminate_test("EXPECT_EQ_test failed. Values are not equal");
    }

    res = EXPECT_EQ(10, 11);
    if (res) {
        terminate_test("EXPECT_EQ_test failed. Values are not equal");
    }

    res = EXPECT_EQ(11, 10);
    if (res) {
        terminate_test("EXPECT_EQ_test failed. Values are not equal");
    }
}

void EXPECT_GE_test()
{
    bool res = EXPECT_GE(10, 10);
    if (res) {
        terminate_test("EXPECT_GE_test failed. Value 1 is not greater than value 2");
    }

    res = EXPECT_GE(10, 11);
    if (res) {
        terminate_test("EXPECT_GE_test failed. Value 1 is not greater than value 2");
    }

    res = EXPECT_GE(11, 10);
    if (!res) {
        terminate_test("EXPECT_GE_test failed. Value 1 is not greater than value 2");
    }
}

void EXPECT_LE_test()
{
    bool res = EXPECT_LE(10, 10);
    if (res) {
        terminate_test("EXPECT_LE_test failed. Value 1 is not less than value 2");
    }

    res = EXPECT_LE(10, 11);
    if (!res) {
        terminate_test("EXPECT_LE_test failed. Value 1 is not less than value 2");
    }

    res = EXPECT_LE(11, 10);
    if (res) {
        terminate_test("EXPECT_LE_test failed. Value 1 is not less than value 2");
    }
}

void EXPECT_CALL_test()
{
    TestLib::init();

    auto test_int_fn = MockedFn<std::function<int(double)>>::create();
    auto ptr = TestLib::fn_expectations();
    if (!ptr) {
        terminate_test("EXPECT_CALL_test failed. TestLib::fn_expectations is null");
    }
    if (!ptr->empty()) {
        terminate_test("EXPECT_CALL_test failed. TestLib::fn_expectations is not empty");
    }
    EXPECT_CALL(test_int_fn, 1);
    if (ptr->empty()) {
        terminate_test("EXPECT_CALL_test failed. TestLib::fn_expectations is empty");
    }
    EXPECT_CALL(test_int_fn, 1);
    if (ptr->size() != 2) {
        terminate_test("EXPECT_CALL_test failed. TestLib::fn_expectations is not equal to 2");
    }
    
    test_int_fn->fn()(10.0);
    TestLib::destroy();
}

void MOCK_VERIFY_EXPECTATIONS_test() {}

} // namespace psi::test
