include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
    test_Item.cpp
    test_Inventory.cpp
    test_AddCommand.cpp
    test_RemoveCommand.cpp
    test_CommandHistory.cpp)

target_link_libraries(unit_tests
    PRIVATE
        eval_core
        GTest::gtest_main
        GTest::gmock)

include(GoogleTest)
gtest_discover_tests(unit_tests
    PROPERTIES LABELS "unit"
    XML_OUTPUT_DIR ${CMAKE_BINARY_DIR}/test-results)
