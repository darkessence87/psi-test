#include "psi/test/psi_test.h"

#ifdef PSI_LOGGER
#include "psi/logger/Logger.h"
#else
#include <iostream>
#include <sstream>
#define LOG_INFO_STATIC(x)                                                                                             \
    do {                                                                                                               \
        std::ostringstream os;                                                                                         \
        os << x;                                                                                                       \
        std::cout << os.str() << std::endl;                                                                            \
    } while (0)
#endif

int main(int argc, char *argv[])
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-container"
    auto args = std::span<char *> {argv, static_cast<size_t>(argc)};
#pragma clang diagnostic pop
    auto opts = psi::test::TestLib::parse_args(args);

    if (!opts.list_tests) {
        LOG_INFO_STATIC("Start tests main");
    }

    psi::test::TestLib::init();
    int result = psi::test::TestLib::run(opts);

    psi::test::TestLib::destroy();

    if (!opts.list_tests) {
        LOG_INFO_STATIC("Exit tests main");
    }

    return result;
}
