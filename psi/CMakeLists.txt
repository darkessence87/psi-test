include_directories(
    include
    src
)

set (SOURCES
    src/psi/test/psi_mock.cpp
    src/psi/test/psi_test.cpp
)

set (target_lib "psi-test")

add_library(${target_lib} ${SOURCES})
target_link_libraries(${target_lib} "clang_rt.asan_dynamic-x86_64;clang_rt.asan_dynamic_runtime_thunk-x86_64")
psi_config_target(${target_lib})

psi_make_examples("1_TestExamples" "examples/1_TestExamples.cpp" "${target_lib}")

set (TEST_SOURCES
    tests/EntryPoint.cpp
)
add_executable(psi_test_tests ${TEST_SOURCES})
target_link_libraries(psi_test_tests ${target_lib})
psi_config_target(psi_test_tests)