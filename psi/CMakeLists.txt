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
    tests/psi_tests.cpp
)
add_executable(PSI_TEST_psi_test ${PROJECT_SOURCE_DIR}/tests/EntryPoint.cpp ${TEST_SOURCES})
target_link_libraries(PSI_TEST_psi_test ${target_lib})
psi_config_target(PSI_TEST_psi_test)