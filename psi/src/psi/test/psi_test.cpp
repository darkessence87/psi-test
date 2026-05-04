
#include "psi/test/psi_test.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include <chrono>
#include <format>
#include <iostream>

namespace psi::test {

static bool s_use_color = true;

struct Color {
    explicit Color(const std::string &code)
    {
        if (s_use_color) std::cout << code;
    }
    ~Color()
    {
        if (s_use_color) std::cout << "\033[0m";
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

TestLib::TestCase *TestLib::m_current_running_test = nullptr;

TestLib::Tests &TestLib::tests()
{
    static Tests* instance = new Tests();
    return *instance;
}

void TestLib::init()
{
#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif
}

void TestLib::destroy()
{
    m_current_running_test = nullptr;
    fn_expectations()->clear();
    auto &t = tests();
    t.m_tests_list.clear();
    t.m_tests_indices.clear();
    t.m_total_tests_number = 0;
    t.m_disabled_count = 0;
}

int TestLib::run()
{
    return run(CmdOptions{});
}

int TestLib::run(const std::string &filter)
{
    return run(CmdOptions{.filter = filter});
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
        static FnExpectationsList *s_list = new FnExpectationsList();
        return s_list;
    }
    return &m_current_running_test->m_fn_expectations;
}

void TestLib::add_test(const TestCase &tc)
{
    auto &tests_ref = tests();

    if (auto it = tests_ref.m_tests_indices.find(tc.m_test_group); it != tests_ref.m_tests_indices.end()) {
        it->second->emplace_back(tc);
        ++tests_ref.m_total_tests_number;
        return;
    }

    tests_ref.m_tests_list.emplace_back(std::vector<TestCase> {tc});
    tests_ref.m_tests_indices.emplace(tc.m_test_group, tests_ref.m_tests_list.rbegin());
    ++tests_ref.m_total_tests_number;
}

TestLib::TestCase *TestLib::current_running_test()
{
    return m_current_running_test;
}

static bool matches_pattern(const std::string &group, const std::string &name, const std::string &pattern)
{
    if (pattern.empty() || pattern == "*") {
        return true;
    }
    // "Group.*" → match by group name only
    if (pattern.size() > 2 && pattern.ends_with(".*")) {
        return group == pattern.substr(0, pattern.size() - 2);
    }
    // "Group.Name" → exact full name match
    const std::string full = group + "." + name;
    return full == pattern;
}

static bool is_disabled_test(const std::string &group, const std::string &name)
{
    return name.starts_with("DISABLED_") || group.starts_with("DISABLED_");
}

// Returns true when a disabled test should be skipped (no explicit filter selects it).
static bool should_skip_disabled(const std::string &group, const std::string &name, const std::string &filter)
{
    if (filter.empty() || filter == "*") {
        return true; // wildcard — skip disabled
    }
    std::string_view sv(filter);
    while (!sv.empty()) {
        auto pos = sv.find(':');
        std::string pat(sv.substr(0, pos));
        // Exact Group.Name pattern (not a wildcard) — treat as explicit selection
        const bool is_wildcard = pat.empty() || pat == "*" || pat.ends_with(".*");
        if (!is_wildcard && matches_pattern(group, name, pat)) {
            return false; // explicitly selected → don't skip
        }
        if (pos == std::string_view::npos) break;
        sv.remove_prefix(pos + 1);
    }
    return true;
}

TestLib::Tests TestLib::get_filtered_tests(const std::string &filter, bool also_run_disabled)
{
    auto &tests_ref = tests();

    TestLib::Tests result;

    // Helper: should this test case be included?
    auto include_test = [&](const TestCase &tc) -> bool {
        const bool disabled = is_disabled_test(tc.m_test_group, tc.m_test_name);
        if (disabled) {
            if (!also_run_disabled && should_skip_disabled(tc.m_test_group, tc.m_test_name, filter)) {
                ++result.m_disabled_count;
                return false;
            }
        }
        // Apply normal filter
        if (filter.empty() || filter == "*") {
            return true;
        }
        std::vector<std::string> patterns;
        {
            std::string_view sv(filter);
            while (!sv.empty()) {
                auto pos = sv.find(':');
                patterns.emplace_back(sv.substr(0, pos));
                if (pos == std::string_view::npos) break;
                sv.remove_prefix(pos + 1);
            }
        }
        for (const auto &pat : patterns) {
            if (matches_pattern(tc.m_test_group, tc.m_test_name, pat)) {
                return true;
            }
        }
        return false;
    };

    for (const auto &tg : tests_ref.m_tests_list) {
        for (const auto &tc : tg) {
            if (!include_test(tc)) {
                continue;
            }
            if (auto it = result.m_tests_indices.find(tc.m_test_group); it != result.m_tests_indices.end()) {
                it->second->emplace_back(tc);
                ++result.m_total_tests_number;
                continue;
            }
            result.m_tests_list.emplace_back(std::vector<TestCase> {tc});
            result.m_tests_indices.emplace(tc.m_test_group, result.m_tests_list.rbegin());
            ++result.m_total_tests_number;
        }
    }

    return result;
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

TestLib::CmdOptions TestLib::parse_args(std::span<char *> argv)
{
    CmdOptions opts {};

    for (size_t i = 1; i < argv.size(); ++i) {
        std::string_view arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            std::cout << "This program contains tests written using Google's C++ testing framework.\n"
                         "Run this program with the name of the test you want to run.\n\n"
                         "  --gtest_list_tests\n"
                         "    List the names of all tests instead of running them.\n"
                         "  --gtest_filter=POSITIVE_PATTERNS[-NEGATIVE_PATTERNS]\n"
                         "    Run only tests whose name matches a positive pattern.\n"
                         "  --gtest_also_run_disabled_tests\n"
                         "    Run tests prefixed with DISABLED_ that are skipped by default.\n"
                         "  --gtest_color=(yes|no|auto)\n"
                         "    Enable/disable colored output.\n";
            std::exit(0);
        } else if (arg == "--gtest_list_tests") {
            opts.list_tests = true;
        } else if (arg == "--gtest_also_run_disabled_tests") {
            opts.also_run_disabled = true;
        } else if (arg.starts_with("--gtest_color=")) {
            opts.color = (arg.substr(14) != "no");
        } else if (arg.starts_with("--gtest_filter=")) {
            std::string gf(arg.substr(15));
            if (gf == "*") {
                opts.filter = "";
            } else {
                opts.filter = gf; // pass through as-is: "Group.*", "Group.Name", "A.B:C.D"
            }
        } else if (arg.starts_with("--filter=")) {
            opts.filter = std::string(arg.substr(9));
        } else if (arg == "--filter" && i + 1 < argv.size()) {
            opts.filter = argv[++i];
        }
    }

    return opts;
}

int TestLib::run(const CmdOptions &opts)
{
    s_use_color = opts.color;
    if (opts.list_tests) {
        const auto &tests_ref = tests();
        for (const auto &test_idx : tests_ref.m_tests_indices) {
            std::cout << test_idx.first << ".\n";
            for (const auto &tc : *test_idx.second) {
                std::cout << "  " << tc.m_test_name << "\n";
            }
        }
        return 0;
    }

    int failed = 0;

    {
        auto c = GREEN();
        std::cout << "[==========]";
    }
    const auto filtered = get_filtered_tests(opts.filter, opts.also_run_disabled);
    std::cout << std::format(" Running {} test{} from {} test suite{}.",
                             filtered.m_total_tests_number,
                             filtered.m_total_tests_number != 1 ? "s" : "",
                             filtered.m_tests_list.size(),
                             filtered.m_tests_list.size() != 1 ? "s" : "")
              << std::endl;
    const auto total_start = std::chrono::high_resolution_clock::now();
    for (const auto &test_idx : filtered.m_tests_indices) {
        {
            auto c = GREEN();
            std::cout << "[----------]";
        }
        std::cout << std::format(" {} test{} from {}",
                                 test_idx.second->size(),
                                 test_idx.second->size() != 1 ? "s" : "",
                                 test_idx.first) << std::endl;
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
            const auto tc_time = std::chrono::duration_cast<std::chrono::milliseconds>(tc_end - tc_start).count();
            verify_and_clear_expectations(test_case);
            m_current_running_test = nullptr;

            const auto is_failed = test_case.m_test_result.m_is_failed;
            if (is_failed) {
                ++failed;
                auto c = RED();
                std::cout << "[  FAILED  ]";
            } else {
                auto c = GREEN();
                std::cout << "[       OK ]";
            }
            std::cout << std::format(" {}.{} ({} ms)", test_case.m_test_group, test_case.m_test_name, tc_time) << std::endl;
        }
        const auto tg_end = std::chrono::high_resolution_clock::now();
        const auto tg_time = std::chrono::duration_cast<std::chrono::milliseconds>(tg_end - tg_start).count();
        {
            auto c = GREEN();
            std::cout << "[----------]";
        }
        std::cout << std::format(" {} test{} from {} ({} ms total)",
                                 test_idx.second->size(),
                                 test_idx.second->size() != 1 ? "s" : "",
                                 test_idx.first, tg_time)
                  << std::endl
                  << std::endl;
    }
    const auto total_end = std::chrono::high_resolution_clock::now();
    const auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start).count();
    {
        auto c = GREEN();
        std::cout << "[==========]";
    }
    std::cout << std::format(" {} test{} from {} test suite{} ran. ({} ms total)",
                             filtered.m_total_tests_number,
                             filtered.m_total_tests_number != 1 ? "s" : "",
                             filtered.m_tests_list.size(),
                             filtered.m_tests_list.size() != 1 ? "s" : "",
                             total_time)
              << std::endl;
    if (failed == 0) {
        auto c = GREEN();
        std::cout << std::format("[  PASSED  ] {} test{}.\n", filtered.m_total_tests_number,
                                 filtered.m_total_tests_number != 1 ? "s" : "");
    } else {
        {
            auto c = RED();
            std::cout << std::format("[  FAILED  ] {} test{}, listed below:\n", failed,
                                     failed != 1 ? "s" : "");
        }
        for (const auto &test_idx : filtered.m_tests_indices) {
            for (const auto &tc : *test_idx.second) {
                if (tc.m_test_result.m_is_failed) {
                    auto c = RED();
                    std::cout << std::format("[  FAILED  ] {}.{}\n", tc.m_test_group, tc.m_test_name);
                }
            }
        }
        std::cout << std::format("\n {} FAILED TEST{}\n", failed, failed != 1 ? "S" : "");
    }
    if (filtered.m_disabled_count > 0) {
        std::cout << std::format("  YOU HAVE {} DISABLED TEST{}\n",
                                 filtered.m_disabled_count,
                                 filtered.m_disabled_count != 1 ? "S" : "");
    }

    return failed;
}

} // namespace psi::test
