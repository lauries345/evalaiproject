# ==============================================================================
# EvalAIProject – Top-level CMake build configuration
#
# Standards context : Structured per IEC 62304 software life cycle (evaluation)
# Minimum CMake     : 3.15
# C++ standard      : ISO/IEC 14882:2017 (C++17), GNU extensions disabled
#
# Revision History:
#   1.0.0  2026-05-15  Seth Laurie  Initial release
# ==============================================================================

cmake_minimum_required(VERSION 3.15)

project(
    EvalAIProject
    VERSION     1.0.0
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
# High-warning builds are mandatory for safety-critical software. Warnings are
# treated as errors to prevent silent accumulation of potential defects.
# ------------------------------------------------------------------------------
if(MSVC)
    # /W4          – comprehensive warning level
    # /WX          – treat all warnings as errors
    # /permissive- – enforce strict ISO C++ conformance (disables MSVC extensions)
    add_compile_options(
        /W4
        /WX
        /permissive-
    )
else()
    # -Wall / -Wextra / -Wpedantic cover the majority of MISRA-relevant diagnostics
    # available via GCC/Clang without a dedicated MISRA checker plug-in.
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

# ------------------------------------------------------------------------------
# Source files
# List each translation unit explicitly; globbing can hide unintentional
# inclusions and complicate traceability in a regulated environment.
# ------------------------------------------------------------------------------
set(PROJECT_SOURCES
    src/main.cpp
    src/Item.cpp
    src/Inventory.cpp
)

# ------------------------------------------------------------------------------
# Executable target
# ------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ------------------------------------------------------------------------------
# Unit tests
# Disabled in this evaluation build. Enable by uncommenting the block below
# and providing a test framework (e.g. GoogleTest via FetchContent or vcpkg).
# ------------------------------------------------------------------------------
# enable_testing()
# add_subdirectory(tests)
