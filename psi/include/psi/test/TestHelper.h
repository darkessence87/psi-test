#pragma once

#include <chrono>
#include <iostream>

namespace psi::test {

class TestHelper
{
public:
    static void timeFn(const auto &name, auto &&fn, int N)
    {
        using namespace std::chrono;

        const auto &start = high_resolution_clock::now();
        for (int i = 0; i < N; ++i) {
            fn();
        }
        const auto &end = high_resolution_clock::now();
        const auto &totalTime = (end - start).count();
        std::cout << "[" << name << "] average fn() us: " << std::fixed << std::setprecision(3)
                  << totalTime / 1000.0 / N << std::endl;
    }
};

} // namespace psi::test
