set (SOURCES
    src/psi/test/psi_mock.cpp
    src/psi/test/psi_test.cpp
)

set (target_lib "psi-test")

add_library(${target_lib} ${SOURCES})
add_library(psi::test ALIAS ${target_lib})

target_include_directories(${target_lib}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

psi_config_target(${target_lib})

psi_make_examples("1_TestExamples" "examples/1_TestExamples.cpp" "${target_lib}")

set (TEST_SOURCES
    tests/EntryPoint.cpp
)
add_executable(psi_test_tests ${TEST_SOURCES})
target_link_libraries(psi_test_tests ${target_lib})
psi_config_target(psi_test_tests)