
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

FnExpectationsList *TestLib::m_fn_expectations = nullptr;
TestLib::TestsHolder *TestLib::m_tests = nullptr;
TestLib::TestsIndices *TestLib::m_tests_indices = nullptr;
size_t TestLib::m_total_tests_number = 0ull;

void TestLib::init()
{
#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif

    if (!m_fn_expectations) {
        m_fn_expectations = new FnExpectationsList();
    }

    if (!m_tests) {
        m_tests = new TestsHolder();
    }

    if (!m_tests_indices) {
        m_tests_indices = new TestsIndices();
    }
}

void TestLib::destroy()
{
    if (m_fn_expectations) {
        delete m_fn_expectations;
        m_fn_expectations = nullptr;
    }

    if (m_tests) {
        delete m_tests;
        m_tests = nullptr;
    }

    if (m_tests_indices) {
        delete m_tests_indices;
        m_tests_indices = nullptr;
    }
}

int TestLib::run()
{
    if (!m_tests || !m_tests_indices) {
        return 1;
    }

    int failed = 0;

    {
        auto c = GREEN();
        std::cout << "[==========]";
    }
    std::cout << std::format(" Running {} tests from {} groups.", m_total_tests_number, m_tests->size()) << std::endl;
    const auto total_start = std::chrono::high_resolution_clock::now();
    for (const auto &test_idx : *m_tests_indices) {
        {
            auto c = GREEN();
            std::cout << "[----------]";
        }
        std::cout << std::format(" {} tests from {}", test_idx.second->size(), test_idx.first) << std::endl;
        const auto &test_group = *test_idx.second;
        const auto tg_start = std::chrono::high_resolution_clock::now();
        for (const auto &test_case : test_group) {
            {
                auto c = GREEN();
                std::cout << "[ RUN      ]";
            }
            std::cout << std::format(" {}.{}", test_case.m_test_group, test_case.m_test_name) << std::endl;
            const auto tc_start = std::chrono::high_resolution_clock::now();
            test_case.m_fn();
            const auto tc_end = std::chrono::high_resolution_clock::now();
            const auto tc_time = std::chrono::duration_cast<std::chrono::nanoseconds>(tc_end - tc_start).count();

            /// @todo check result of test
            {
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
    std::cout << std::format(" {} tests from {} groups ran. ({} ms total)", m_total_tests_number, m_tests->size(), total_time)
              << std::endl;

    return failed;
}

void TestLib::verify_expectations()
{
    if (!m_fn_expectations) {
        return;
    }

    for (const auto &exp : *m_fn_expectations) {
        exp->verify();
    }
}

void TestLib::verify_and_clear_expectations()
{
    if (!m_fn_expectations) {
        return;
    }

    for (const auto &exp : *m_fn_expectations) {
        exp->verify();
        exp->reset();
    }

    m_fn_expectations->clear();
}

FnExpectationsList *TestLib::fn_expectations()
{
    return m_fn_expectations;
}

void TestLib::add_test(TestCase tc)
{
    if (!m_tests || !m_tests_indices) {
        return;
    }

    if (auto it = m_tests_indices->find(tc.m_test_group); it != m_tests_indices->end()) {
        it->second->emplace_back(tc);
        ++m_total_tests_number;
        return;
    }

    m_tests->emplace_back(std::vector<TestCase> {tc});
    m_tests_indices->emplace(tc.m_test_group, m_tests->rbegin());
    ++m_total_tests_number;
}

} // namespace psi::test
