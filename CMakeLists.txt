cmake_minimum_required(VERSION 3.15)
project(EvalAIProject
        VERSION     1.0.0
        DESCRIPTION "Inventory management exemplar (IEC 62304 / ISO 14971 demonstrator)"
        LANGUAGES   CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build unit tests"      ON)
option(CODE_COVERAGE "Enable coverage flags" OFF)

if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

if(CODE_COVERAGE AND NOT MSVC)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

add_library(eval_core
    src/Item.cpp
    src/Inventory.cpp
    src/AddCommand.cpp
    src/RemoveCommand.cpp
    src/CommandHistory.cpp)
target_include_directories(eval_core PUBLIC include)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE eval_core)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(CODE_COVERAGE AND NOT MSVC)
    find_program(LCOV    lcov)
    find_program(GENHTML genhtml)
    if(LCOV AND GENHTML)
        add_custom_target(coverage
            COMMAND ${LCOV} --directory . --zerocounters
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LCOV} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/_deps/*' '*/tests/*'
                              --output-file coverage.info
            COMMAND ${GENHTML} coverage.info --output-directory coverage_html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating lcov HTML coverage report")
    endif()
endif()
