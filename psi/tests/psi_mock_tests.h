
#pragma once

#include "psi/test/psi_mock.h"

namespace psi::test {

TEST(MockedFn, create)
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    EXPECT_NE(mock, nullptr);
}

TEST(MockedFn, counts_calls)
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    mock->fn()(1.0);
    mock->fn()(2.0);
    EXPECT_EQ(mock->get_calls_count(), 2);
}

TEST(MockedFn, default_return_value)
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    EXPECT_EQ(mock->fn()(42.0), 0);
}

TEST(EXPECT_CALL, exact_count)
{
    auto mock = MockedFn<std::function<int(double)>>::create();
    EXPECT_CALL(mock, 2);
    mock->fn()(1.0);
    mock->fn()(2.0);
}

TEST(EXPECT_CALL, with_args)
{
    auto mock = MockedFn<std::function<int(double)>>::create();
    EXPECT_CALL(mock, 1).WithArgs(10.0);
    mock->fn()(10.0);
}

} // namespace psi::test
