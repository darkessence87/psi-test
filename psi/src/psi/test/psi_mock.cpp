
#include "psi/test/psi_mock.h"

#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

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
