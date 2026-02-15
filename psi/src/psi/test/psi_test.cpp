
#include "psi/test/psi_test.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include <chrono>

namespace psi::test {

struct Color {
    explicit Color(const std::string &code)
    {
        std::cout << code;
    }
    ~Color()
    {
        std::cout << "\033[0m";
    }
};

[[nodiscard]] inline Color GREEN()
{
    return Color("\033[32m");
}

[[nodiscard]] inline Color RED()
{
    return Color("\033[31m");
}

TestLib::Tests *TestLib::m_tests = nullptr;
TestLib::TestCase *TestLib::m_current_running_test = nullptr;

void TestLib::init()
{
#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif

    if (!m_tests) {
        m_tests = new Tests();
    }
}

void TestLib::destroy()
{
    if (m_tests) {
        delete m_tests;
        m_tests = nullptr;
    }

    m_current_running_test = nullptr;
}

int TestLib::run(const std::string &filter)
{
    if (!m_tests) {
        return 1;
    }

    int failed = 0;

    {
        auto c = GREEN();
        std::cout << "[==========]";
    }
    const auto &tests = get_filtered_tests(filter);
    std::cout << std::format(" Running {} tests from {} groups.", tests.m_total_tests_number, tests.m_tests_list.size())
              << std::endl;
    const auto total_start = std::chrono::high_resolution_clock::now();
    for (const auto &test_idx : tests.m_tests_indices) {
        {
            auto c = GREEN();
            std::cout << "[----------]";
        }
        std::cout << std::format(" {} tests from {}", test_idx.second->size(), test_idx.first) << std::endl;
        auto &test_group = *test_idx.second;
        const auto tg_start = std::chrono::high_resolution_clock::now();
        for (auto &test_case : test_group) {
            {
                auto c = GREEN();
                std::cout << "[ RUN      ]";
            }
            std::cout << std::format(" {}.{}", test_case.m_test_group, test_case.m_test_name) << std::endl;
            m_current_running_test = &test_case;

            const auto tc_start = std::chrono::high_resolution_clock::now();
            test_case.m_fn();
            const auto tc_end = std::chrono::high_resolution_clock::now();
            const auto tc_time = std::chrono::duration_cast<std::chrono::nanoseconds>(tc_end - tc_start).count();
            verify_and_clear_expectations(test_case);

            const auto is_failed = test_case.m_test_result.m_is_failed;
            if (is_failed) {
                auto c = RED();
                std::cout << "[ FAILED   ]";
            } else {
                auto c = GREEN();
                std::cout << "[       OK ]";
            }
            std::cout << std::format(" {}.{} ({} ns)", test_case.m_test_group, test_case.m_test_name, tc_time) << std::endl;
        }
        const auto tg_end = std::chrono::high_resolution_clock::now();
        const auto tg_time = std::chrono::duration_cast<std::chrono::nanoseconds>(tg_end - tg_start).count();
        {
            auto c = GREEN();
            std::cout << "[----------]";
        }
        std::cout << std::format(" {} tests from {} ({} ns total)", test_idx.second->size(), test_idx.first, tg_time)
                  << std::endl
                  << std::endl;
    }
    const auto total_end = std::chrono::high_resolution_clock::now();
    const auto total_time = std::chrono::duration_cast<std::chrono::nanoseconds>(total_end - total_start).count();
    {
        auto c = GREEN();
        std::cout << "[==========]";
    }
    std::cout << std::format(" {} tests from {} groups ran. ({} ms total)",
                             tests.m_total_tests_number,
                             tests.m_tests_list.size(),
                             total_time)
              << std::endl;

    return failed;
}

void TestLib::verify_expectations()
{
    if (auto test = current_running_test()) {
        for (const auto &exp : test->m_fn_expectations) {
            exp->verify();
        }
    }
}

void TestLib::verify_and_clear_expectations()
{
    if (auto test = current_running_test()) {
        for (const auto &exp : test->m_fn_expectations) {
            exp->verify();
            exp->reset();
        }
        test->m_fn_expectations.clear();
    }
}

void TestLib::verify_expectations(TestCase &tc)
{
    for (const auto &exp : tc.m_fn_expectations) {
        exp->verify();
    }
}

void TestLib::verify_and_clear_expectations(TestCase &tc)
{
    for (const auto &exp : tc.m_fn_expectations) {
        exp->verify();
        exp->reset();
    }

    tc.m_fn_expectations.clear();
}

FnExpectationsList *TestLib::fn_expectations()
{
    if (!m_current_running_test) {
        return nullptr;
    }
    return &m_current_running_test->m_fn_expectations;
}

void TestLib::add_test(const TestCase &tc)
{
    if (!m_tests) {
        return;
    }

    if (auto it = m_tests->m_tests_indices.find(tc.m_test_group); it != m_tests->m_tests_indices.end()) {
        it->second->emplace_back(tc);
        ++m_tests->m_total_tests_number;
        return;
    }

    m_tests->m_tests_list.emplace_back(std::vector<TestCase> {tc});
    m_tests->m_tests_indices.emplace(tc.m_test_group, m_tests->m_tests_list.rbegin());
    ++m_tests->m_total_tests_number;
}

TestLib::TestCase *TestLib::current_running_test()
{
    return m_current_running_test;
}

TestLib::Tests TestLib::get_filtered_tests(const std::string &filter)
{
    if (!m_tests) {
        return {};
    }

    if (filter.empty()) {
        return *m_tests;
    }

    TestLib::Tests tests;

    for (const auto &tg : m_tests->m_tests_list) {
        for (const auto &tc : tg) {
            bool skip_test = true;
            if (tc.m_test_group.find(filter) != std::string::npos) {
                skip_test = false;
            }
            if (tc.m_test_name.find(filter) != std::string::npos) {
                skip_test = false;
            }
            if (skip_test) {
                continue;
            }

            if (auto it = tests.m_tests_indices.find(tc.m_test_group); it != tests.m_tests_indices.end()) {
                it->second->emplace_back(tc);
                ++tests.m_total_tests_number;
                continue;
            }

            tests.m_tests_list.emplace_back(std::vector<TestCase> {tc});
            tests.m_tests_indices.emplace(tc.m_test_group, tests.m_tests_list.rbegin());
            ++tests.m_total_tests_number;
        }
    }

    return tests;
}

void TestLib::TestCase::fail_test(const std::string &msg, bool is_assert)
{
    m_test_result.m_is_failed = true;
    std::cout << msg << std::endl;
    if (is_assert) {
        throw std::runtime_error(msg);
    }
}

void TestLib::TestCase::fail_test(const std::wstring &msg, bool is_assert)
{
    m_test_result.m_is_failed = true;
    std::wcout << msg << std::endl;
    if (is_assert) {
        /// @todo convert wchar_t* to char*
        throw std::runtime_error("wchar failed");
    }
}

TestLib::CmdOptions TestLib::parse_args(std::span<char*> argv)
{
    CmdOptions opts {};

    for (size_t i = 1; i < argv.size(); ++i) {
        std::string_view arg = argv[i];

        if (arg.starts_with("--filter=")) {
            opts.filter = std::string(arg.substr(9));
        } else if (arg == "--filter" && i + 1 < argv.size()) {
            opts.filter = argv[++i];
        }
    }

    return opts;
}

} // namespace psi::test
