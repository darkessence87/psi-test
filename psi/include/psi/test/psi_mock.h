
#pragma once

#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "psi_test.h"

namespace psi::test {

namespace detail {
inline std::string ptr_to_str(const void *p) noexcept
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%p", p);
    return std::string(buf);
}
} // namespace detail

enum class ComparisonOperation : uint8_t
{
    Equal,
    Greater,
    GreaterOrEqual,
    Less,
    LessOrEqual,
};

template <typename T>
concept non_bool_integral = std::integral<T> && !std::same_as<T, bool>;

template <typename T>
concept bool_integral = std::integral<T> && std::same_as<T, bool>;

template <typename T1, typename T2>
    requires non_bool_integral<std::remove_cvref_t<T1>> && non_bool_integral<std::remove_cvref_t<T2>>
inline void COMPARE(T1 &&arg1, T2 &&arg2, ComparisonOperation op, bool is_assert = false)
{
    bool res = false;
    std::string error;

    switch (op) {
    case ComparisonOperation::Equal:
        res = std::cmp_equal(arg1, arg2);
        error = "[PSI-TEST] arg1 (" + std::to_string(arg1) + ") MUST be equal to arg2 (" + std::to_string(arg2) + ")";
        break;
    case ComparisonOperation::Greater:
        res = std::cmp_greater(arg1, arg2);
        error = "[PSI-TEST] arg1 (" + std::to_string(arg1) + ") MUST be greater than arg2 (" + std::to_string(arg2) + ")";
        break;
    case ComparisonOperation::GreaterOrEqual:
        res = std::cmp_greater_equal(arg1, arg2);
        error = "[PSI-TEST] arg1 (" + std::to_string(arg1) + ") MUST be greater than or equal to arg2 (" + std::to_string(arg2) + ")";
        break;
    case ComparisonOperation::Less:
        res = std::cmp_less(arg1, arg2);
        error = "[PSI-TEST] arg1 (" + std::to_string(arg1) + ") MUST be less than arg2 (" + std::to_string(arg2) + ")";
        break;
    case ComparisonOperation::LessOrEqual:
        res = std::cmp_less_equal(arg1, arg2);
        error = "[PSI-TEST] arg1 (" + std::to_string(arg1) + ") MUST be less than or equal to arg2 (" + std::to_string(arg2) + ")";
        break;
    }

    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            test->fail_test(error, is_assert);
        }
    }
}

template <typename T1, typename T2>
    requires non_bool_integral<std::remove_cvref_t<T1>> && non_bool_integral<std::remove_cvref_t<T2>>
inline void EXPECT_EQ(T1 &&arg1, T2 &&arg2)
{
    COMPARE(std::forward<T1>(arg1), std::forward<T2>(arg2), ComparisonOperation::Equal);
}

template <typename T1, typename T2>
    requires bool_integral<std::remove_cvref_t<T1>> && bool_integral<std::remove_cvref_t<T2>>
inline void EXPECT_EQ(T1 &&a1, T2 &&a2)
{
    if (a1 != a2) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::string(a1 ? "true" : "false") + " not equal to " + (a2 ? "true" : "false");
            test->fail_test(error);
        }
    }
}

template <typename T>
inline void EXPECT_TRUE(T arg)
{
    if (!arg) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::to_string(arg) + " not TRUE";
            test->fail_test(error);
        }
    }
}

template <typename T>
inline void EXPECT_FALSE(T arg)
{
    if (arg) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::to_string(arg) + " not FALSE";
            test->fail_test(error);
        }
    }
}

template <typename T>
inline void ASSERT_TRUE(T arg)
{
    if (!arg) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::to_string(arg) + " not TRUE";
            test->fail_test(error, true);
        }
    }
}

template <typename T>
inline void ASSERT_FALSE(T arg)
{
    if (arg) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::to_string(arg) + " not FALSE";
            test->fail_test(error, true);
        }
    }
}

template <typename T1, typename T2>
    requires non_bool_integral<std::remove_cvref_t<T1>> && non_bool_integral<std::remove_cvref_t<T2>>
inline void ASSERT_EQ(T1 &&arg1, T2 &&arg2)
{
    COMPARE(std::forward<T1>(arg1), std::forward<T2>(arg2), ComparisonOperation::Equal, true);
}

template <typename T1, typename T2>
    requires bool_integral<std::remove_cvref_t<T1>> && bool_integral<std::remove_cvref_t<T2>>
inline void ASSERT_EQ(T1 &&a1, T2 &&a2)
{
    if (a1 != a2) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::string(a1 ? "true" : "false") + " not equal to " + (a2 ? "true" : "false");
            test->fail_test(error, true);
        }
    }
}

template <typename T1, typename T2>
    requires std::is_pointer_v<std::remove_cvref_t<T1>>
             && (std::is_pointer_v<std::remove_cvref_t<T2>> || std::same_as<std::remove_cvref_t<T2>, std::nullptr_t>)
inline void EXPECT_EQ(T1 ptr1, T2 ptr2)
{
    const auto res = ptr1 == ptr2;
    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const void *p1 = ptr1;
            const void *p2 = ptr2;
            const auto error = detail::ptr_to_str(p1) + " not equal to " + detail::ptr_to_str(p2);
            test->fail_test(error);
        }
    }
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::string_view> && std::convertible_to<T2, std::string_view>
inline void EXPECT_EQ(T1 &&s1, T2 &&s2)
{
    const auto res = std::string_view {s1} == std::string_view {s2};
    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::string(std::string_view(s1)) + " not equal to " + std::string(std::string_view(s2));
            test->fail_test(error);
        }
    }
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::string_view> && std::convertible_to<T2, std::string_view>
inline void EXPECT_CONTAINS(T1 &&haystack, T2 &&needle)
{
    const std::string_view h {haystack};
    const std::string_view n {needle};
    if (h.find(n) == std::string_view::npos) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::string("\"" ) + std::string(h) + "\" does not contain \"" + std::string(n) + "\"";
            test->fail_test(error);
        }
    }
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::string_view> && std::convertible_to<T2, std::string_view>
inline void ASSERT_EQ(T1 &&s1, T2 &&s2)
{
    const auto res = std::string_view {s1} == std::string_view {s2};
    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::string(std::string_view(s1)) + " not equal to " + std::string(std::string_view(s2));
            test->fail_test(error, true);
        }
    }
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::basic_string_view<char8_t>>
             && std::convertible_to<T2, std::basic_string_view<char8_t>>
inline void EXPECT_EQ(T1 &&a, T2 &&b)
{
    const auto res = std::basic_string_view<char8_t> {a} == std::basic_string_view<char8_t> {b};
    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const auto s1 = std::string(reinterpret_cast<const char *>(a.data()), a.size());
            const auto s2 = std::string(reinterpret_cast<const char *>(b.data()), b.size());
            const auto error = s1 + " not equal to " + s2;
            test->fail_test(error);
        }
    }
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::wstring_view> && std::convertible_to<T2, std::wstring_view>
inline void EXPECT_EQ(const T1 &a, const T2 &b)
{
    const auto res = std::wstring_view {a} == std::wstring_view {b};
    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const auto s1 = std::wstring(std::wstring_view {a});
            const auto s2 = std::wstring(std::wstring_view {b});
            test->fail_test(s1 + L" not equal to " + s2);
        }
    }
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::wstring_view> && std::convertible_to<T2, std::wstring_view>
inline void ASSERT_EQ(T1 &&s1, T2 &&s2)
{
    const auto res = std::wstring_view {s1} == std::wstring_view {s2};
    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::wstring(s1) + L" not equal to " + std::wstring(s2);
            test->fail_test(error, true);
        }
    }
}

template <typename T1, typename T2>
    requires std::is_pointer_v<std::remove_cvref_t<T1>>
             && (std::is_pointer_v<std::remove_cvref_t<T2>> || std::same_as<std::remove_cvref_t<T2>, std::nullptr_t>)
inline void EXPECT_NE(T1 ptr1, T2 ptr2)
{
    const auto res = ptr1 != ptr2;
    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const void *p1 = ptr1;
            const void *p2 = ptr2;
            const auto error = detail::ptr_to_str(p1) + " not equal to " + detail::ptr_to_str(p2);
            test->fail_test(error);
        }
    }
}

template <typename T1, typename T2>
inline void EXPECT_NE(const std::shared_ptr<T1> &ptr1, const std::shared_ptr<T2> &ptr2)
{
    const bool res = ptr1 != ptr2;

    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const void *p1 = ptr1.get();
            const void *p2 = ptr2.get();
            const auto error = detail::ptr_to_str(p1) + " not equal to " + detail::ptr_to_str(p2);
            test->fail_test(error);
        }
    }
}

template <typename T>
inline void EXPECT_NE(const std::shared_ptr<T> &ptr, std::nullptr_t)
{
    const bool res = ptr != nullptr;

    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const void *p = ptr.get();
            const auto error = detail::ptr_to_str(p) + " not equal to nullptr";
            test->fail_test(error);
        }
    }
}

template <typename T>
inline void EXPECT_NE(std::nullptr_t, const std::shared_ptr<T> &ptr)
{
    EXPECT_NE(ptr, nullptr);
}

template <typename T>
    requires std::integral<T>
inline void EXPECT_GE(T &&arg1, T &&arg2)
{
    COMPARE(std::forward<T>(arg1), std::forward<T>(arg2), ComparisonOperation::GreaterOrEqual);
}

template <typename T>
    requires std::integral<T>
inline void ASSERT_GE(T &&arg1, T &&arg2)
{
    COMPARE(std::forward<T>(arg1), std::forward<T>(arg2), ComparisonOperation::GreaterOrEqual, true);
}

template <typename T1, typename T2>
    requires std::integral<std::remove_cvref_t<T1>> && std::integral<std::remove_cvref_t<T2>>
inline void EXPECT_LE(T1 &&arg1, T2 &&arg2)
{
    COMPARE(std::forward<T1>(arg1), std::forward<T2>(arg2), ComparisonOperation::LessOrEqual);
}

template <typename T1, typename T2>
    requires std::equality_comparable_with<T1, T2>
inline void EXPECT_EQ(const std::vector<T1> &a, const std::vector<T2> &b)
{
    auto test = TestLib::current_running_test();

    if (a.size() != b.size()) {
        if (test) {
            test->fail_test("std::vector<T1> and std::vector<T2> sizes are not equal");
        }
        return;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i], b[i]);
    }
}

template <typename K1, typename V1, typename K2, typename V2>
    requires std::equality_comparable_with<K1, K2> && std::equality_comparable_with<V1, V2>
inline void EXPECT_EQ(const std::map<K1, V1> &a, const std::map<K2, V2> &b)
{
    auto test = TestLib::current_running_test();

    if (a.size() != b.size()) {
        if (test) {
            test->fail_test("std::map<K1, V1> and std::map<K2, V2> sizes are not equal");
        }
        return;
    }

    auto it1 = a.begin();
    auto it2 = b.begin();
    while (it1 != a.end() && it2 != b.end()) {
        EXPECT_EQ(it1->first, it2->first);
        EXPECT_EQ(it1->second, it2->second);
        ++it1;
        ++it2;
    }
}

template <typename R, typename... Args>
inline FnExpectation<R, Args...> &EXPECT_CALL(std::shared_ptr<MockedFn<std::function<R(Args...)>>> fn,
                                              int expected_calls_number)
{
    auto exp = std::make_shared<FnExpectation<R, Args...>>(expected_calls_number, fn);
    auto exp_ptr = exp.get();
    if (auto ptr = TestLib::fn_expectations()) {
        ptr->emplace_back(std::move(exp));
    }
    return *exp_ptr;
}

inline void MOCK_VERIFY_EXPECTATIONS();

} // namespace psi::test
