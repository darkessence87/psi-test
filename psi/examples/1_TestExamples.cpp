
#include "psi/test/psi_mock.h"

int main()
{
    using namespace psi::test;

    TestLib::init();

    auto test_int_fn = MockedFn<std::function<int(double)>>::create();
    EXPECT_CALL(test_int_fn, 2).WithArgs(10.0).WithArgs(12.0);
    auto f = test_int_fn->fn();
    f(10.0);
    f(12.0);

    TestLib::destroy();
}
