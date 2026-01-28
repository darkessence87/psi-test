
#pragma once

#include "psi/test/psi_test.h"

namespace psi::test {

#define terminate_test(error)                                                                                          \
    std::cout << __FILE__ << ":" << __LINE__ << " " << error << std::endl;                                             \
    std::terminate()

struct MockedFn_Tests {
    void create_test();
    void fn_test();
    void f_test();
};

struct FnExpectation_Tests {
    void ctor_test();
    void dtor_test();
    void verify_test();
};

struct TestLib_Tests {
    void init_test();
    void destroy_test();
    void verify_expectations_test();
    void verify_and_clear_expectations_test();
    void fn_expectations_test();
};

void MockedFn_Tests::create_test()
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    if (!mock) {
        terminate_test("MockedFn_Tests::create_test failed. Mock is null.");
    }
}

void MockedFn_Tests::fn_test()
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    auto mock_fn = mock->fn();
    int a = mock_fn(10.0);
    int b = mock_fn(20.0);
    if (a != 0 || b != 0) {
        terminate_test("MockedFn_Tests::fn_test failed. Mock is not mock.");
    }
    if (mock->get_calls_count() != 2) {
        terminate_test("MockedFn_Tests::fn_test failed. Mock is not called 2 times.");
    }
}

void MockedFn_Tests::f_test()
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    int a = mock->f(10.0);
    int b = mock->f(20.0);
    if (a != 0 || b != 0) {
        terminate_test("MockedFn_Tests::f_test failed. Mock is not mock.");
    }
    if (mock->get_calls_count() != 2) {
        terminate_test("MockedFn_Tests::f_test failed. Mock is not called 2 times.");
    }
}

void FnExpectation_Tests::ctor_test()
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    FnExpectation<int, double> exp(2, mock);
    if (exp.m_expected_calls != 2) {
        terminate_test("FnExpectation_Tests::ctor_test failed. m_expected_calls != 2.");
    }
    if (exp.m_function.lock() != mock) {
        terminate_test("FnExpectation_Tests::ctor_test failed. m_function != mock.");
    }
    exp.reset();
}

void FnExpectation_Tests::dtor_test()
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    FnExpectation<int, double> exp(2, mock);
    mock->fn()(10.0);
    mock->fn()(20.0);
    if (exp.m_expected_calls != mock->get_calls_count()) {
        terminate_test("FnExpectation_Tests::dtor_test failed. m_expected_calls != mock->get_calls_count().");
    }
    exp.reset();
}

void FnExpectation_Tests::verify_test()
{
    using Fn = std::function<int(double)>;
    auto mock = MockedFn<Fn>::create();
    FnExpectation<int, double> exp(2, mock);
    mock->fn()(10.0);
    if (exp.m_expected_calls == mock->get_calls_count()) {
        terminate_test("FnExpectation_Tests::verify_test failed. m_expected_calls = mock->get_calls_count().");
    }
    mock->fn()(20.0);
    if (exp.m_expected_calls != mock->get_calls_count()) {
        terminate_test("FnExpectation_Tests::verify_test failed. m_expected_calls != mock->get_calls_count().");
    }
    exp.verify();
    exp.reset();
}

void TestLib_Tests::init_test()
{
    if (TestLib::m_fn_expectations != nullptr) {
        terminate_test("TestLib_Tests::init_test failed. m_fn_expectations != nullptr.");
    }
    TestLib::init();
    if (TestLib::m_fn_expectations == nullptr) {
        terminate_test("TestLib_Tests::init_test failed. m_fn_expectations = nullptr.");
    }
}

void TestLib_Tests::destroy_test()
{
    TestLib::destroy();
    if (TestLib::m_fn_expectations != nullptr) {
        terminate_test("TestLib_Tests::destroy_test failed. m_fn_expectations != nullptr.");
    }
    TestLib::init();
    TestLib::destroy();
    if (TestLib::m_fn_expectations != nullptr) {
        terminate_test("TestLib_Tests::destroy_test failed. m_fn_expectations != nullptr.");
    }
    TestLib::init();
    TestLib::destroy();
    if (TestLib::m_fn_expectations != nullptr) {
        terminate_test("TestLib_Tests::destroy_test failed. m_fn_expectations != nullptr.");
    }
}

void TestLib_Tests::verify_expectations_test()
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
    struct FakeExpectation : public IFnExpectation {
        void verify() const override
        {
            m_is_verified = true;
        }
        void reset() override {}
        mutable bool m_is_verified = false;
    };
#pragma clang diagnostic pop

    TestLib::verify_expectations();
    TestLib::init();
    TestLib::verify_expectations();

    auto exp = std::make_unique<FakeExpectation>();
    auto exp_ptr = exp.get();
    TestLib::m_fn_expectations->emplace_back(std::move(exp));

    TestLib::verify_expectations();
    if (exp_ptr->m_is_verified == false) {
        terminate_test("TestLib_Tests::verify_expectations_test failed. exp.m_is_verified != true.");
    }

    TestLib::destroy();
    TestLib::verify_expectations();
}

void TestLib_Tests::verify_and_clear_expectations_test()
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
    struct FakeExpectation : public IFnExpectation {
        void verify() const override
        {
            m_is_verified = true;
        }
        void reset() override
        {
            m_is_reset = true;
        }
        mutable bool m_is_verified = false;
        bool m_is_reset = false;
    };
#pragma clang diagnostic pop

    TestLib::verify_and_clear_expectations();
    TestLib::init();
    TestLib::verify_and_clear_expectations();

    auto exp = std::make_unique<FakeExpectation>();
    auto exp_ptr = exp.get();
    TestLib::m_fn_expectations->emplace_back(std::move(exp));

    TestLib::verify_and_clear_expectations();
    if (exp_ptr->m_is_verified == false) {
        terminate_test("TestLib_Tests::verify_and_clear_expectations_test failed. exp.m_is_verified != true.");
    }
    if (exp_ptr->m_is_reset == false) {
        terminate_test("TestLib_Tests::verify_and_clear_expectations_test failed. exp.m_is_reset != true.");
    }

    TestLib::destroy();
    TestLib::verify_and_clear_expectations();
}

void TestLib_Tests::fn_expectations_test()
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
    struct FakeExpectation : public IFnExpectation {
        void verify() const override
        {
            m_is_verified = true;
        }
        void reset() override {}
        mutable bool m_is_verified = false;
    };
#pragma clang diagnostic pop

    auto exps_ptr = TestLib::fn_expectations();
    if (exps_ptr != nullptr) {
        terminate_test("TestLib_Tests::fn_expectations_test failed. exp.m_is_verified != nullptr.");
    }
    TestLib::init();
    exps_ptr = TestLib::fn_expectations();
    if (exps_ptr == nullptr) {
        terminate_test("TestLib_Tests::fn_expectations_test failed. exp.m_is_verified = nullptr.");
    }
    TestLib::destroy();
    exps_ptr = TestLib::fn_expectations();
    if (exps_ptr != nullptr) {
        terminate_test("TestLib_Tests::fn_expectations_test failed. exp.m_is_verified != nullptr.");
    }
}

} // namespace psi::test
