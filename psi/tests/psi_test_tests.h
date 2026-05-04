
#pragma once

#include "psi/test/psi_test.h"

namespace psi::test {

TEST(MockedFn, create_not_null)
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    EXPECT_NE(mock, nullptr);
}

TEST(FnExpectation, correct_call_count)
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    {
        FnExpectation<int, double> exp(2, mock);
        mock->fn()(1.0);
        mock->fn()(2.0);
        // destructor calls verify(): 2 == 2, no failure
    }
    EXPECT_EQ(mock->get_calls_count(), 2);
}

TEST(FnExpectation, with_args_match)
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    {
        FnExpectation<int, double> exp(1, mock);
        exp.WithArgs(5.0);
        mock->fn()(5.0);
        // destructor verifies: 1 call with arg 5.0, matches
    }
}

TEST(TestLib, fn_expectations_not_null)
{
    EXPECT_NE(TestLib::fn_expectations(), nullptr);
}

} // namespace psi::test
