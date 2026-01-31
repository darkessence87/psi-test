
#pragma once

#include <deque>
#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

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

            std::string error =
                std::format("[PSI-TEST] m_expected_calls ({}) MUST be equal to m_function.m_calls_count ({})",
                            m_expected_calls,
                            fn->m_calls_count);
            std::cout << error << std::endl;
            return;
        }
    }

    void reset() override
    {
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
    int get_calls_count() const
    {
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
    static int run();
    static void verify_expectations();
    static void verify_and_clear_expectations();
    static FnExpectationsList *fn_expectations();

    struct TestCase {
        std::string m_test_group;
        std::string m_test_name;
        std::function<void()> m_fn;
    };
    static void add_test(TestCase tc);

private:
    static FnExpectationsList *m_fn_expectations;
    using TestsHolder = std::deque<std::vector<TestCase>>;
    using TestsIndices = std::map<std::string, TestsHolder::reverse_iterator>;
    static TestsHolder *m_tests;
    static TestsIndices *m_tests_indices;
    static size_t m_total_tests_number;

    friend struct TestLib_Tests;
};

#define TEST(test_group, test_name)                                                                                    \
    void test_group##_##test_name##_impl();                                                                            \
    void test_group##_##test_name##_impl()

} // namespace psi::test
