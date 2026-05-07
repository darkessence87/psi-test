# Description

A lightweight, dependency-free C++ test framework with a GTest-compatible command-line interface.

### TestLib

The main test runner. Drive it from your `EntryPoint.cpp`:

```cpp
#include "psi/test/psi_test.h"

int main(int argc, char **argv)
{
    psi::test::TestLib::init();
    const auto opts = psi::test::TestLib::parse_args({argv, static_cast<size_t>(argc)});
    return psi::test::TestLib::run(opts);
}
```

### TEST macro

```cpp
TEST(GroupName, TestName)
{
    EXPECT_EQ(1, 1);
}
```

Tests prefixed with `DISABLED_` in their name are skipped unless `--gtest_also_run_disabled_tests` is passed.

### Assertions

| Macro | Behaviour |
|---|---|
| `EXPECT_EQ(a, b)` | Record failure if `a != b`, continue test |
| `EXPECT_TRUE(x)` | Record failure if `x` is false |
| `EXPECT_FALSE(x)` | Record failure if `x` is true |
| `EXPECT_CONTAINS(haystack, needle)` | Record failure if substring not found |
| `ASSERT_EQ(a, b)` | Abort test immediately if `a != b` |
| `ASSERT_TRUE(x)` | Abort test immediately if `x` is false |
| `ASSERT_FALSE(x)` | Abort test immediately if `x` is true |

### Mocking

```cpp
#include "psi/test/psi_mock.h"

// Declare a mock for std::function<int(double)>
auto fn = psi::test::MockedFn<std::function<int(double)>>::create();

// Expect exactly 2 calls, with specific arguments
EXPECT_CALL(fn, 2).WithArgs(10.0).WithArgs(12.0);

auto f = fn->fn(); // get a callable wrapper
f(10.0);
f(12.0);
```

### TestHelper

Timing utilities for microbenchmarks:

```cpp
psi::test::TestHelper::timeFn("label", []{ /* ... */ }, 1000);      // prints average µs
psi::test::TestHelper::timeFn_nano("label", []{ /* ... */ }, 1000); // prints average ns
```

### Command-line options

| Flag | Description |
|---|---|
| `--gtest_filter=PATTERN` | Run only matching tests (e.g. `Group.*`, `A.B:C.D`) |
| `--gtest_list_tests` | Print all test names and exit |
| `--gtest_also_run_disabled_tests` | Include `DISABLED_` tests |
| `--gtest_color=(yes\|no\|auto)` | Enable / disable coloured output |
| `--filter=PATTERN` | Shorthand filter flag |

# Usage examples
* [1 Mock examples](https://github.com/darkessence87/psi-test/blob/master/psi/examples/1_TestExamples.cpp)