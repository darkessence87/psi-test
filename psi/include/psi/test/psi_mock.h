
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
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
inline bool COMPARE(T1 &&arg1, T2 &&arg2, ComparisonOperation op, bool is_assert = false)
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
        std::cout << error << std::endl;
        if (is_assert) {
            throw std::runtime_error(error);
        }
    }

    return res;
}

template <typename T1, typename T2>
    requires std::integral<std::remove_cvref_t<T1>> && std::integral<std::remove_cvref_t<T2>>
inline bool EXPECT_EQ(T1 &&arg1, T2 &&arg2)
{
    return COMPARE(std::forward<T1>(arg1), std::forward<T2>(arg2), ComparisonOperation::Equal);
}

template <typename T1, typename T2>
    requires std::integral<std::remove_cvref_t<T1>> && std::integral<std::remove_cvref_t<T2>>
inline bool ASSERT_EQ(T1 &&arg1, T2 &&arg2)
{
    return COMPARE(std::forward<T1>(arg1), std::forward<T2>(arg2), ComparisonOperation::Equal, true);
}

template <typename T1, typename T2>
    requires std::is_pointer_v<std::remove_cvref_t<T1>>
             && (std::is_pointer_v<std::remove_cvref_t<T2>> || std::same_as<std::remove_cvref_t<T2>, std::nullptr_t>)
inline bool EXPECT_EQ(T1 ptr1, T2 ptr2)
{
    return ptr1 == ptr2;
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::string_view> && std::convertible_to<T2, std::string_view>
inline bool EXPECT_EQ(T1 &&s1, T2 &&s2)
{
    return std::string_view {s1} == std::string_view {s2};
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::string_view> && std::convertible_to<T2, std::string_view>
inline bool ASSERT_EQ(T1 &&s1, T2 &&s2)
{
    if (!(std::string_view {s1} == std::string_view {s2})) {
        throw std::runtime_error(std::format("{} not equal to {}", s1, s2));
    }
    return true;
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::basic_string_view<char8_t>>
             && std::convertible_to<T2, std::basic_string_view<char8_t>>
inline bool EXPECT_EQ(T1 &&a, T2 &&b)
{
    return std::basic_string_view<char8_t> {a} == std::basic_string_view<char8_t> {b};
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::wstring_view> && std::convertible_to<T2, std::wstring_view>
inline bool EXPECT_EQ(T1 &&s1, T2 &&s2)
{
    return std::wstring_view {s1} == std::wstring_view {s2};
}

template <typename T1, typename T2>
    requires std::convertible_to<T1, std::wstring_view> && std::convertible_to<T2, std::wstring_view>
inline bool ASSERT_EQ(T1 &&s1, T2 &&s2)
{
    if (!(std::wstring_view {s1} == std::wstring_view {s2})) {
        throw std::runtime_error(std::format("{} not equal to {}", s1, s2));
    }
    return true;
}

template <typename T1, typename T2>
    requires std::is_pointer_v<std::remove_cvref_t<T1>>
             && (std::is_pointer_v<std::remove_cvref_t<T2>> || std::same_as<std::remove_cvref_t<T2>, std::nullptr_t>)
inline bool EXPECT_NE(T1 ptr1, T2 ptr2)
{
    return ptr1 != ptr2;
}

template <typename T>
    requires std::integral<T>
inline bool EXPECT_GE(T &&arg1, T &&arg2)
{
    return COMPARE(std::forward<T>(arg1), std::forward<T>(arg2), ComparisonOperation::Greater);
}

template <typename T>
    requires std::integral<T>
inline bool ASSERT_GE(T &&arg1, T &&arg2)
{
    return COMPARE(std::forward<T>(arg1), std::forward<T>(arg2), ComparisonOperation::Greater, true);
}

template <typename T>
    requires std::integral<T>
inline bool EXPECT_LE(T &&arg1, T &&arg2)
{
    return COMPARE(std::forward<T>(arg1), std::forward<T>(arg2), ComparisonOperation::Less);
}

template <typename T1, typename T2>
    requires std::equality_comparable_with<T1, T2>
inline bool EXPECT_EQ(const std::vector<T1> &a, const std::vector<T2> &b)
{
    if (a.size() != b.size())
        return false;

    for (size_t i = 0; i < a.size(); ++i) {
        if (!EXPECT_EQ(a[i], b[i])) {
            return false;
        }
    }

    return true;
}

template <typename K1, typename V1, typename K2, typename V2>
    requires std::equality_comparable_with<K1, K2> && std::equality_comparable_with<V1, V2>
inline bool EXPECT_EQ(const std::map<K1, V1> &a, const std::map<K2, V2> &b)
{
    if (a.size() != b.size())
        return false;

    for (const auto& it1 : a) {
        for (const auto& it2 : b) {
            if (!EXPECT_EQ(it1.first, it2.first)) {
                return false;
            }
            if (!EXPECT_EQ(it1.second, it2.second)) {
                return false;
            }
        }
    }

    return true;
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
