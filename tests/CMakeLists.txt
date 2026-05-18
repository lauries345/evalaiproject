# ==============================================================================
# EvalAIProject – Tests CMake configuration
#
# Standards context : Unit test build per IEC 62304 §5.5 (software unit testing)
# Framework         : GoogleTest v1.14.0 (fetched via FetchContent)
#
# Revision History:
#   1.1.0  2026-05-15  Seth Laurie  Initial test target
# ==============================================================================

# ------------------------------------------------------------------------------
# Test executable
# Links against eval_lib (the production library) and GTest. The strict
# eval_compile_warnings interface is intentionally NOT applied here so that
# GTest macro expansions do not trigger warnings-as-errors on test code.
# ------------------------------------------------------------------------------
set(TEST_SOURCES
    test_Item.cpp
    test_Inventory.cpp
    test_Command.cpp
)

add_executable(EvalAIProject_tests ${TEST_SOURCES})

target_link_libraries(EvalAIProject_tests
    PRIVATE
        eval_lib
        GTest::gtest_main
)

# ------------------------------------------------------------------------------
# Test discovery
# gtest_discover_tests runs the test executable at configure time to enumerate
# individual test cases and register each one as a separate CTest entry.
# ------------------------------------------------------------------------------
include(GoogleTest)
gtest_discover_tests(EvalAIProject_tests)
