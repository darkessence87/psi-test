
#pragma once

#include <deque>
#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <utility>

namespace psi::test {

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

using FnExpectationsList = std::vector<std::shared_ptr<IFnExpectation>>;

struct TestLib {
    static void init();
    static void destroy();
    static int run(const std::string &filter = "");
    static FnExpectationsList *fn_expectations();
    static void verify_expectations();
    static void verify_and_clear_expectations();

    struct TestFailure {
        std::string m_file;
        int m_line;
        std::string m_expression;
        std::string m_actual;
        std::string m_expected;
    };
    struct TestResult {
        std::string m_test_name;
        bool m_is_failed = false;
        std::vector<TestFailure> m_failures;
    };
    struct TestCase {
        std::string m_test_group;
        std::string m_test_name;
        std::function<void()> m_fn;
        FnExpectationsList m_fn_expectations;
        TestResult m_test_result = {};
        void fail_test(const std::string &msg, bool is_assert = false);
        void fail_test(const std::wstring &msg, bool is_assert = false);
    };
    static void add_test(const TestCase &tc);
    static TestCase *current_running_test();

    struct CmdOptions {
        std::string filter;
        bool list_tests = false;
        bool color = true;
    };

    static int run(const CmdOptions &opts);
    static CmdOptions parse_args(std::span<char *> argv);

private:
    struct Tests {
        using TestsHolder = std::deque<std::vector<TestCase>>;
        using TestsIndices = std::map<std::string, TestsHolder::reverse_iterator>;
        TestsHolder m_tests_list;
        TestsIndices m_tests_indices;
        size_t m_total_tests_number = 0;
    };

    static Tests get_filtered_tests(const std::string &filter);
    static void verify_expectations(TestCase &tc);
    static void verify_and_clear_expectations(TestCase &tc);

private:
    static Tests &tests();
    static TestCase *m_current_running_test;

    friend struct TestLib_Tests;
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

    FnExpectation &WithArgs(Args... args)
    {
        m_expected_calls_args.push_back(std::make_tuple(std::decay_t<Args>(args)...));
        return *this;
    }

    FnExpectation &WithArgContains(size_t arg_index, std::string_view substring)
    {
        m_arg_contains_checks.emplace_back(arg_index, std::string(substring));
        return *this;
    }

    void verify() const override
    {
        if (!m_function) {
            return;
        }

        auto test = TestLib::current_running_test();
        if (!test) {
            return;
        }

        if (!m_expected_calls_args.empty()) {
            if (m_function->m_calls.size() != m_expected_calls_args.size()) {
                test->fail_test("[PSI-TEST] Args count mismatch");
            } else {
                for (size_t i = 0; i < m_expected_calls_args.size(); ++i) {
                    if (m_function->m_calls[i] != m_expected_calls_args[i]) {
                        test->fail_test(std::format("[PSI-TEST] Args mismatch at call {}", i));
                    }
                }
            }
        }

        if (!m_arg_contains_checks.empty()) {
            for (size_t call_i = 0; call_i < m_function->m_calls.size(); ++call_i) {
                for (const auto &[arg_index, substring] : m_arg_contains_checks) {
                    const auto str = get_string_arg(m_function->m_calls[call_i], arg_index);
                    if (!str.has_value()) {
                        test->fail_test(std::format("[PSI-TEST] WithArgContains: arg {} is not a string", arg_index));
                    } else if (str->find(substring) == std::string::npos) {
                        test->fail_test(std::format("[PSI-TEST] WithArgContains: \"{}\" does not contain \"{}\"",
                                                    *str,
                                                    substring));
                    }
                }
            }
        }

        if (m_expected_calls != m_function->m_calls_count) {
            test->fail_test(
                std::format("[PSI-TEST] m_expected_calls ({}) MUST be equal to m_function.m_calls_count ({})",
                            m_expected_calls,
                            m_function->m_calls_count));
        }
    }

    void reset() override
    {
        if (m_function) {
            m_function->m_calls_count = 0;
            m_function->m_calls.clear();
        }
        m_function.reset();
    }

private:
    template <typename Tuple>
    static std::optional<std::string> get_string_arg(const Tuple &t, size_t index)
    {
        std::optional<std::string> result;
        size_t i = 0;
        std::apply(
            [&](const auto &...args) {
                auto check = [&](const auto &arg) {
                    if (i == index) {
                        if constexpr (std::convertible_to<std::decay_t<decltype(arg)>, std::string_view>) {
                            result = std::string(std::string_view(arg));
                        }
                    }
                    ++i;
                };
                (check(args), ...);
            },
            t);
        return result;
    }

    int m_expected_calls = 0;
    std::shared_ptr<Fn> m_function;
    std::vector<std::tuple<std::decay_t<Args>...>> m_expected_calls_args;
    std::vector<std::pair<size_t, std::string>> m_arg_contains_checks;

    friend struct FnExpectation_Tests;
};

template <typename R, typename... Args>
struct MockedFn<std::function<R(Args...)>> : TestFn<std::function<R(Args...)>> {
    int get_calls_count() const
    {
        return m_calls_count;
    }

private:
    R f(Args &&...args) const override
    {
        ++m_calls_count;
        m_calls.emplace_back(std::forward<Args>(args)...);
        return R {};
    }

private:
    mutable int m_calls_count = 0;
    mutable std::vector<std::tuple<std::decay_t<Args>...>> m_calls;

    friend FnExpectation<R, Args...>;
    friend struct MockedFn_Tests;
};

#define TEST(test_group, test_name)                                                                                    \
    static void test_group##_##test_name##_impl();                                                                     \
    namespace {                                                                                                        \
    struct test_group##_##test_name##_registrar {                                                                      \
        test_group##_##test_name##_registrar()                                                                         \
        {                                                                                                              \
            psi::test::TestLib::add_test({#test_group, #test_name, &test_group##_##test_name##_impl, {}});             \
        }                                                                                                              \
    };                                                                                                                 \
    static test_group##_##test_name##_registrar test_group##_##test_name##_registrar_instance;                         \
    }                                                                                                                  \
    static void test_group##_##test_name##_impl()

} // namespace psi::test
