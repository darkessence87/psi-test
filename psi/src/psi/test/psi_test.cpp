
#include "psi/test/psi_test.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

namespace psi::test {

FnExpectationsList *TestLib::m_fn_expectations = nullptr;

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
}

void TestLib::destroy()
{
    if (m_fn_expectations) {
        delete m_fn_expectations;
        m_fn_expectations = nullptr;
    }
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

} // namespace psi::test
