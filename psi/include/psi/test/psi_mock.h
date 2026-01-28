
#pragma once

#include <functional>
#include <memory>

#include "psi_test.h"

namespace psi::test {

enum class ComparisonOperation : uint8_t
{
    Equal,
    Greater,
    Less,
};

template <typename T>
requires std::integral<T>
inline bool COMPARE(T &&arg1, T &&arg2, ComparisonOperation op)
{
    decltype(arg1) a1 = std::forward<T>(arg1);
    decltype(arg2) a2 = std::forward<T>(arg2);
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
    }

    return res;
}

template <typename T>
requires std::integral<T>
inline bool EXPECT_EQ(T &&arg1, T &&arg2)
{
    return COMPARE(std::forward<T>(arg1), std::forward<T>(arg2), ComparisonOperation::Equal);
}

template <typename T>
requires std::integral<T>
inline bool EXPECT_GE(T &&arg1, T &&arg2)
{
    return COMPARE(std::forward<T>(arg1), std::forward<T>(arg2), ComparisonOperation::Greater);
}

template <typename T>
requires std::integral<T>
inline bool EXPECT_LE(T &&arg1, T &&arg2)
{
    return COMPARE(std::forward<T>(arg1), std::forward<T>(arg2), ComparisonOperation::Less);
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
