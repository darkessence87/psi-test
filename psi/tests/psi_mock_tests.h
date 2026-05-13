
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

TEST(psi_mock, EXPECT_CONTAINS_pass)
{
    EXPECT_CONTAINS(std::string("hello world"), std::string("world"));
}

TEST(psi_mock, EXPECT_EQ_vector_equal)
{
    std::vector<int> a {1, 2, 3};
    std::vector<int> b {1, 2, 3};
    EXPECT_EQ(a, b);
}

TEST(psi_mock, EXPECT_EQ_map_equal)
{
    std::map<int, int> a {{1, 10}, {2, 20}};
    std::map<int, int> b {{1, 10}, {2, 20}};
    EXPECT_EQ(a, b);
}

TEST(psi_mock, ASSERT_GE_pass)
{
    ASSERT_GE(5, 3);
}

TEST(psi_mock, ASSERT_EQ_wstring_pass)
{
    std::wstring a = L"hello";
    std::wstring b = L"hello";
    ASSERT_EQ(a, b);
}

TEST(psi_mock, EXPECT_NE_pointers_pass)
{
    int x = 1, y = 2;
    EXPECT_NE(&x, &y);
}

TEST(psi_mock, EXPECT_NE_nullptr_shared_ptr_pass)
{
    auto sp = std::make_shared<int>(42);
    EXPECT_NE(nullptr, sp);
}

TEST(psi_mock, EXPECT_EQ_char8t_pass)
{
    std::u8string a = u8"abc";
    std::u8string b = u8"abc";
    EXPECT_EQ(a, b);
}

TEST(psi_mock, detail_ptr_to_str)
{
    int x = 0;
    const auto s = detail::ptr_to_str(&x);
    EXPECT_TRUE(!s.empty());
    const auto s2 = detail::ptr_to_str(nullptr);
    EXPECT_TRUE(!s2.empty());
}

TEST(psi_mock, EXPECT_FALSE_pass)
{
    EXPECT_FALSE(false);
}

TEST(psi_mock, ASSERT_FALSE_pass)
{
    ASSERT_FALSE(false);
}

TEST(psi_mock, EXPECT_NE_shared_ptr_different)
{
    auto sp1 = std::make_shared<int>(1);
    auto sp2 = std::make_shared<int>(2);
    EXPECT_NE(sp1, sp2);
}

TEST(psi_mock, ASSERT_TRUE_pass)
{
    ASSERT_TRUE(true);
}

TEST(psi_mock, ASSERT_EQ_bool_pass)
{
    ASSERT_EQ(true, true);
    bool x = false;
    ASSERT_EQ(x, false);
}

TEST(psi_mock, ASSERT_EQ_string_pass)
{
    std::string a = "hello";
    std::string b = "hello";
    ASSERT_EQ(a, b);
}

} // namespace psi::test
