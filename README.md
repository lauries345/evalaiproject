# EvalAIProject

Inventory management exemplar in modern C++17, structured as an
IEC 62304 / ISO 14971 demonstrator: layered architecture, GoF Command
(undo/redo) and Observer patterns, smart pointers throughout, Doxygen
documentation, and exhaustive GoogleTest coverage.

## Architecture

Three layers, all in `namespace eval`:

| Layer                    | Class(es)                                 |
|--------------------------|-------------------------------------------|
| Value type               | `eval::Item`                              |
| Container / manager      | `eval::Inventory`                         |
| Command abstraction      | `eval::ICommand`                          |
| Concrete commands        | `eval::AddCommand`, `eval::RemoveCommand` |
| Undo / redo orchestrator | `eval::CommandHistory`                    |
| Observer interface       | `eval::IInventoryObserver`                |

Directory layout:

    include/   public headers (.h)
    src/       implementation (.cpp)
    tests/     GoogleTest suites (one per class)
    scripts/   coverage helpers (PowerShell + bash)
    build/     out-of-source CMake output (gitignored)

## Build & run

Configure and build:

    cmake -S . -B build
    cmake --build build --parallel

Run the demo executable:

    # Linux / macOS
    ./build/EvalAIProject

    # Windows (MSVC, multi-config generator default)
    .\build\Debug\EvalAIProject.exe

The demo wires up an `Inventory`, attaches a `ConsoleLogger`
(`IInventoryObserver` implementation), runs four commands through a
`CommandHistory`, exercises an undo/redo cycle, and prints the result.
You should see a `[LOW STOCK]` notification fire for Oranges.

## Tests

GoogleTest is fetched automatically at configure time via `FetchContent`.

Run all unit tests:

    cd build
    ctest --output-on-failure

Emit JUnit XML for CI:

    ctest --output-on-failure --output-junit test-results.xml

Tests live one-class-per-file:

    tests/test_Item.cpp
    tests/test_Inventory.cpp
    tests/test_AddCommand.cpp
    tests/test_RemoveCommand.cpp
    tests/test_CommandHistory.cpp

## Coverage

Linux / macOS (lcov + genhtml):

    ./scripts/run_coverage_lcov.sh
    # open build/coverage_html/index.html

Windows (OpenCppCoverage; install from
https://github.com/OpenCppCoverage/OpenCppCoverage):

    pwsh -File .\scripts\run_coverage_msvc.ps1
    # open .\coverage_html\index.html

Reports are scoped to `src/` and `include/`; GoogleTest internals and the
test sources themselves are excluded so the percentages reflect only
production code. The target is 100% line and branch coverage.

## CMake options

| Option           | Default | Purpose                              |
|------------------|---------|--------------------------------------|
| `BUILD_TESTING`  | `ON`    | Build the `unit_tests` target        |
| `CODE_COVERAGE`  | `OFF`   | Add `--coverage` flags (GCC/Clang)   |

## Coding conventions

- C++17, enforced via `CMAKE_CXX_STANDARD 17` (no compiler extensions).
- Warnings-as-errors: `/W4 /WX /permissive-` on MSVC,
  `-Wall -Wextra -Wpedantic -Werror` on GCC / Clang.
- Allman braces, 4-space indent, 100-character line limit.
- PascalCase types, camelCase methods, `m_` prefix on private members.
- `[[nodiscard]]` on every query method; `noexcept` on every non-throwing one.
- Explicit `= default` / `= delete` on every special member.
- `#pragma once` guard in every header.

## Standards

| Standard           | Role                                                |
|--------------------|-----------------------------------------------------|
| IEC 62304          | SW lifecycle (classification, traceability, change) |
| ISO 14971          | Risk management                                     |
| MISRA C++ 2008     | Coding guidelines (static analyser required)        |
| ISO/IEC 14882:2017 | C++17 language standard                             |

Every header and source file carries a Doxygen file block with revision
history, safety classification (IEC 62304 Class A), and applicable
standards. Class invariants are documented with `@invariant`.
