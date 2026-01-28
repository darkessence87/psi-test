
#include "psi/test/psi_mock.h"

int main()
{
    using namespace psi::test;

    TestLib::init();

    auto test_int_fn = MockedFn<std::function<int(double)>>::create();
    EXPECT_CALL(test_int_fn, 1);
    auto f = test_int_fn->fn();
    f(10.0);
    f(12.0);
    TestLib::verify_and_clear_expectations();

    TestLib::destroy();
}
