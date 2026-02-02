
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "psi_test.h"

namespace psi::test {

enum class ComparisonOperation : uint8_t
{
    Equal,
    Greater,
    Less,
};

template <typename T1, typename T2>
    requires std::integral<std::remove_cvref_t<T1>> && std::integral<std::remove_cvref_t<T2>>
inline void COMPARE(T1 &&arg1, T2 &&arg2, ComparisonOperation op, bool is_assert = false)
{
    decltype(arg1) a1 = std::forward<T1>(arg1);
    decltype(arg2) a2 = std::forward<T2>(arg2);
    bool res = false;
    std::string error = "";

    switch (op) {
    case ComparisonOperation::Equal:
        res = a1 == a2;
        error = std::format("[PSI-TEST] arg1 ({}) MUST be equal to arg2 ({})", a1, a2);
        break;
    case ComparisonOperation::Greater:
        res = a1 > a2;
        error = std::format("[PSI-TEST] arg1 ({}) MUST be greater than arg2 ({})", a1, a2);
        break;
    case ComparisonOperation::Less:
        res = a1 < a2;
        error = std::format("[PSI-TEST] arg1 ({}) MUST be less than arg2 ({})", a1, a2);
        break;
    }

    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            test->fail_test(error, is_assert);
        }
    }
}

template <typename T1, typename T2>
    requires std::integral<std::remove_cvref_t<T1>> && std::integral<std::remove_cvref_t<T2>>
inline void EXPECT_EQ(T1 &&arg1, T2 &&arg2)
{
    COMPARE(std::forward<T1>(arg1), std::forward<T2>(arg2), ComparisonOperation::Equal);
}

template <typename T1, typename T2>
    requires std::integral<std::remove_cvref_t<T1>> && std::integral<std::remove_cvref_t<T2>>
inline void ASSERT_EQ(T1 &&arg1, T2 &&arg2)
{
    COMPARE(std::forward<T1>(arg1), std::forward<T2>(arg2), ComparisonOperation::Equal, true);
}

template <typename T1, typename T2>
    requires std::is_pointer_v<std::remove_cvref_t<T1>>
             && (std::is_pointer_v<std::remove_cvref_t<T2>> || std::same_as<std::remove_cvref_t<T2>, std::nullptr_t>)
inline void EXPECT_EQ(T1 ptr1, T2 ptr2)
{
    const auto res = ptr1 == ptr2;
    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const void* p1 = ptr1;
            const void* p2 = ptr2;
            const auto error = std::format("{} not equal to {}", p1, p2);
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
            const auto error = std::format("{} not equal to {}", s1, s2);
            test->fail_test(error);
        }
    }
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::string_view> && std::convertible_to<T2, std::string_view>
inline void ASSERT_EQ(T1 &&s1, T2 &&s2)
{
    const auto res = !(std::string_view {s1} == std::string_view {s2});
    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::format("{} not equal to {}", s1, s2);
            test->fail_test(error);
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
            const auto error = std::format("{} not equal to {}", s1, s2);
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
            const auto s1 = std::wstring(std::wstring_view{a});
            const auto s2 = std::wstring(std::wstring_view{b});
            test->fail_test(std::format(L"{} not equal to {}", s1, s2));
        }
    }
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::wstring_view> && std::convertible_to<T2, std::wstring_view>
inline void ASSERT_EQ(T1 &&s1, T2 &&s2)
{
    const auto res = !(std::wstring_view {s1} == std::wstring_view {s2});
    if (!res) {
        if (auto test = TestLib::current_running_test()) {
            const auto error = std::format("{} not equal to {}", s1, s2);
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
            const void* p1 = ptr1;
            const void* p2 = ptr2;
            const auto error = std::format("{} not equal to {}", p1, p2);
            test->fail_test(error, true);
        }
    }
}

template <typename T>
    requires std::integral<T>
inline void EXPECT_GE(T &&arg1, T &&arg2)
{
    COMPARE(std::forward<T>(arg1), std::forward<T>(arg2), ComparisonOperation::Greater);
}

template <typename T>
    requires std::integral<T>
inline void ASSERT_GE(T &&arg1, T &&arg2)
{
    COMPARE(std::forward<T>(arg1), std::forward<T>(arg2), ComparisonOperation::Greater, true);
}

template <typename T>
    requires std::integral<T>
inline void EXPECT_LE(T &&arg1, T &&arg2)
{
    COMPARE(std::forward<T>(arg1), std::forward<T>(arg2), ComparisonOperation::Less);
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

    for (const auto &it1 : a) {
        for (const auto &it2 : b) {
            EXPECT_EQ(it1.first, it2.first);
            EXPECT_EQ(it1.second, it2.second);
        }
    }
}

template <typename R, typename... Args>
inline void EXPECT_CALL(std::shared_ptr<MockedFn<std::function<R(Args...)>>> fn, int expected_calls_number)
{
    auto exp = std::make_unique<FnExpectation<R, Args...>>(expected_calls_number, fn);
    if (auto ptr = TestLib::fn_expectations()) {
        ptr->emplace_back(std::move(exp));
    }
}

inline void MOCK_VERIFY_EXPECTATIONS();

} // namespace psi::test
