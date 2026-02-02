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
        const double totalTime = static_cast<double>((end - start).count()) / 1000.0 / N;
        std::cout << "[" << name << "] average fn() us: " << std::fixed << std::setprecision(3) << totalTime << std::endl;
    }
};

} // namespace psi::test
