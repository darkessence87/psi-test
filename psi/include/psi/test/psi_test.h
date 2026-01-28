
#pragma once

#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace psi::test {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
struct IFnExpectation {
    IFnExpectation() = default;
    virtual ~IFnExpectation() = default;
    IFnExpectation(const IFnExpectation &) = delete;
    IFnExpectation &operator=(const IFnExpectation &) = delete;
    virtual void verify() const = 0;
    virtual void reset() = 0;
};

template <typename C>
struct MockedFn;

template <typename C>
struct TestFn;
#pragma clang diagnostic pop

template <typename R, typename... Args>
struct TestFn<std::function<R(Args...)>> {
    TestFn() = default;
    virtual ~TestFn() = default;
    TestFn(const TestFn &) = default;
    TestFn &operator=(const TestFn &) = default;

    using Fn = std::function<R(Args...)>;

    static std::shared_ptr<MockedFn<std::function<R(Args...)>>> create()
    {
        return std::make_shared<MockedFn<std::function<R(Args...)>>>();
    }

    Fn fn() const
    {
        return [this](Args &&...args) -> R { return f(std::forward<Args>(args)...); };
    }

private:
    virtual R f(Args &&...) const = 0;
};

template <typename R, typename... Args>
struct FnExpectation : public IFnExpectation {
    using Fn = MockedFn<std::function<R(Args...)>>;

    FnExpectation(int expected_calls, std::shared_ptr<Fn> fn)
        : m_expected_calls(expected_calls)
        , m_function(fn)
    {
    }

    ~FnExpectation() override
    {
        verify();
    }

    void verify() const override
    {
        if (auto fn = m_function.lock()) {
            if (m_expected_calls == fn->m_calls_count) {
                return;
            }

            std::string error = std::format("[PSI-TEST] m_expected_calls ({}) MUST be equal to m_function.m_calls_count ({})",
                                            m_expected_calls,
                                            fn->m_calls_count);
            std::cout << error << std::endl;
            return;
        }
    }

    void reset() override {
        m_function.reset();
    }

private:
    int m_expected_calls = 0;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
    std::weak_ptr<Fn> m_function;
#pragma clang diagnostic pop

    friend struct FnExpectation_Tests;
};

template <typename R, typename... Args>
struct MockedFn<std::function<R(Args...)>> : TestFn<std::function<R(Args...)>> {
    int get_calls_count() const {
        return m_calls_count;
    }

private:
    R f(Args &&...) const override
    {
        ++m_calls_count;
        return R {};
    }

private:
    mutable int m_calls_count = 0;

    friend FnExpectation<R, Args...>;
    friend struct MockedFn_Tests;
};

using FnExpectationsList = std::vector<std::unique_ptr<IFnExpectation>>;

struct TestLib {
    static void init();
    static void destroy();
    static void verify_expectations();
    static void verify_and_clear_expectations();
    static FnExpectationsList *fn_expectations();

private:
    static FnExpectationsList* m_fn_expectations;

    friend struct TestLib_Tests;
};

} // namespace psi::test
