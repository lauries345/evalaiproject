# ==============================================================================
# EvalAIProject – Top-level CMake build configuration
#
# Standards context : Structured per IEC 62304 software life cycle (evaluation)
# Minimum CMake     : 3.15
# C++ standard      : ISO/IEC 14882:2017 (C++17), GNU extensions disabled
#
# Revision History:
#   1.0.0  2026-05-15  Seth Laurie  Initial release
#   1.1.0  2026-05-15  Seth Laurie  Add unit-test target and code-coverage support
# ==============================================================================

cmake_minimum_required(VERSION 3.15)

project(
    EvalAIProject
    VERSION     1.1.0
    DESCRIPTION "Inventory management evaluation application"
    LANGUAGES   CXX
)

# ------------------------------------------------------------------------------
# Language standard
# Enforce strict ISO C++17 with no vendor-specific extensions to maximise
# portability across toolchains used in medical device development.
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)   # Disable GNU extensions (e.g. -std=c++17, not -std=gnu++17)

# ------------------------------------------------------------------------------
# Compiler warnings
# Applied via an INTERFACE library so that third-party dependencies fetched via
# FetchContent inherit only their own warning settings, not ours.
# High-warning builds are mandatory for safety-critical software; warnings are
# treated as errors to prevent silent accumulation of potential defects.
# ------------------------------------------------------------------------------
add_library(eval_compile_warnings INTERFACE)
if(MSVC)
    # /W4          – comprehensive warning level
    # /WX          – treat all warnings as errors
    # /permissive- – enforce strict ISO C++ conformance (disables MSVC extensions)
    target_compile_options(eval_compile_warnings INTERFACE
        /W4
        /WX
        /permissive-
    )
else()
    # -Wall / -Wextra / -Wpedantic cover the majority of MISRA-relevant diagnostics
    # available via GCC/Clang without a dedicated MISRA checker plug-in.
    target_compile_options(eval_compile_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

# ------------------------------------------------------------------------------
# Code coverage
# Enabled via -DCODE_COVERAGE=ON. Supported on GCC and Clang only.
# Must be set before targets are defined so flags apply to all compiled units.
# ------------------------------------------------------------------------------
option(CODE_COVERAGE "Enable code coverage instrumentation (GCC/Clang only)" OFF)

if(CODE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Code coverage instrumentation enabled (--coverage)")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    else()
        message(WARNING
            "CODE_COVERAGE=ON but compiler is ${CMAKE_CXX_COMPILER_ID}; "
            "coverage is only supported with GCC or Clang. Flag ignored.")
    endif()
endif()

# ------------------------------------------------------------------------------
# Library target
# Item and Inventory translation units are compiled into a static library so
# both the application executable and the test executable can link against them
# without duplicating object files or including main().
# ------------------------------------------------------------------------------
add_library(eval_lib STATIC
    src/Item.cpp
    src/Inventory.cpp
    src/Command.cpp
)

target_include_directories(eval_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(eval_lib
    PRIVATE
        eval_compile_warnings
)

# ------------------------------------------------------------------------------
# Application executable
# ------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        eval_lib
        eval_compile_warnings
)

# ------------------------------------------------------------------------------
# Unit tests
# Uses GoogleTest via CMake FetchContent. Requires network access on the first
# configure. Disable with -DBUILD_TESTING=OFF.
# ------------------------------------------------------------------------------
include(CTest)

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        GIT_SHALLOW    TRUE
    )

    # On MSVC, link GTest against the same CRT variant as the project.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Do not install GTest targets alongside the project.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()

# ------------------------------------------------------------------------------
# Coverage report target  (GCC/Clang + lcov + genhtml)
# Usage:
#   cmake -B build -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build
#   cmake --build build --target coverage
# The HTML report is written to <build>/coverage_report/index.html.
# ------------------------------------------------------------------------------
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    find_program(LCOV_PATH    lcov)
    find_program(GENHTML_PATH genhtml)

    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMENT "Running tests and generating HTML coverage report..."

            # Reset counters from any previous run.
            # --ignore-errors empty,empty  – lcov v2 requires this when .gcda files
            #   are absent (e.g. a clean build directory or second invocation after
            #   zerocounters already ran).
            COMMAND ${LCOV_PATH}
                --zerocounters
                --directory ${CMAKE_BINARY_DIR}
                --ignore-errors empty,empty

            # Execute the test suite.
            COMMAND ${CMAKE_CTEST_COMMAND}
                --output-on-failure
                --build-config ${CMAKE_BUILD_TYPE}

            # Capture coverage data for all instrumented translation units.
            # lcov v2 notes:
            #   mismatch,mismatch – fully suppresses the "mismatched end line"
            #     diagnostic that geninfo emits for GTest macro expansions.
            #     Specifying the keyword twice is required by lcov v2 to reach
            #     exit 0 (once only demotes the error to a warning, which still
            #     produces a non-zero exit code).
            #   gcov,gcov         – suppresses "unexecuted block on non-branch
            #     line" warnings emitted for STL headers.
            COMMAND ${LCOV_PATH}
                --capture
                --directory ${CMAKE_BINARY_DIR}
                --output-file ${CMAKE_BINARY_DIR}/coverage_raw.info
                --ignore-errors mismatch,mismatch,gcov,gcov

            # Restrict the report to project source and header files only.
            # --ignore-errors unused,unused – lcov v2 exits non-zero when a
            #   glob pattern matches no entry in the trace file.  src/main.cpp
            #   is compiled into the application executable only, not the test
            #   binary, so it has no .gcda data and triggers this error.
            COMMAND ${LCOV_PATH}
                --extract ${CMAKE_BINARY_DIR}/coverage_raw.info
                "${CMAKE_SOURCE_DIR}/src/*"
                "${CMAKE_SOURCE_DIR}/include/*"
                --output-file ${CMAKE_BINARY_DIR}/coverage.info
                --ignore-errors unused,unused

            # Render the HTML report.
            COMMAND ${GENHTML_PATH}
                ${CMAKE_BINARY_DIR}/coverage.info
                --output-directory ${CMAKE_BINARY_DIR}/coverage_report
                --title "EvalAIProject Coverage"
                --legend

            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        message(STATUS "Coverage target 'coverage' available (lcov + genhtml found)")
    else()
        message(STATUS
            "CODE_COVERAGE=ON but lcov/genhtml not found; "
            "'coverage' target not created. Install lcov to enable HTML reports.")
    endif()
endif()
