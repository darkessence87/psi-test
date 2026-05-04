
#include "psi/test/psi_mock.h"

namespace psi::test {

void MOCK_VERIFY_EXPECTATIONS()
{
    if (auto ptr = TestLib::fn_expectations()) {
        for (const auto &exp : *ptr) {
            exp->verify();
        }
    }
}

} // namespace psi::test
