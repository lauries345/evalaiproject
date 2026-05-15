# EvalAIProject

**Version:** 1.0.0
**Author:** Seth Laurie
**Date:** 2026-05-15
**Classification:** Evaluation / Non-production

> **Regulatory Notice:** This software is structured according to the conventions
> required for medical device software development (IEC 62304). It is an
> **evaluation build only** and has not undergone formal verification, validation,
> or risk management activities required for deployment in a regulated medical
> device. All changes to production-bound versions must follow applicable change
> control procedures.

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Repository Structure](#2-repository-structure)
3. [Software Architecture](#3-software-architecture)
4. [Design Rationale](#4-design-rationale)
5. [Build Process](#5-build-process)
6. [Testing](#6-testing)
7. [Code Coverage](#7-code-coverage)
8. [Coding Standards](#8-coding-standards)
9. [Applicable Standards](#9-applicable-standards)
10. [Revision History](#10-revision-history)

---

## 1. Project Overview

EvalAIProject is a C++17 console application that demonstrates a simple
inventory management system. It is used as an evaluation vehicle for
assessing AI-assisted software development practices in the context of
medical device software (MDS) development workflows.

The application manages a collection of named items, each with an associated
quantity. Core operations include:

- Adding items (with automatic merging of duplicate entries)
- Removing items by name
- Sorting items lexicographically
- Querying total quantity across all items

---

## 2. Repository Structure

```
EvalAIProject/
├── CMakeLists.txt          # Top-level CMake build configuration
├── README.md               # This document
├── .gitignore
│
├── include/                # Public header files (interface declarations)
│   ├── Item.h              # Item entity – name + quantity
│   └── Inventory.h         # Inventory container – manages a collection of Items
│
├── src/                    # Translation units (implementations)
│   ├── main.cpp            # Application entry point
│   ├── Item.cpp            # Item member function definitions
│   └── Inventory.cpp       # Inventory member function definitions
│
├── tests/                  # Unit tests (GoogleTest)
│   ├── CMakeLists.txt      # Test target configuration
│   ├── test_Item.cpp       # Unit tests for eval::Item
│   └── test_Inventory.cpp  # Unit tests for eval::Inventory
│
└── scripts/
    └── run_coverage_msvc.ps1  # Windows/MSVC coverage helper (OpenCppCoverage)
```

**Convention:** All public interfaces live under `include/`. No implementation
detail is exposed in headers beyond what is needed to satisfy the compiler
(i.e., private member layout). This separation supports independent review,
traceability, and future mocking/substitution in a test harness.

---

## 3. Software Architecture

### 3.1 Component Diagram

```
┌─────────────────────────────────────────────────┐
│                   main.cpp                      │
│            (application entry point)            │
└──────────────────────┬──────────────────────────┘
                       │ uses
                       ▼
┌─────────────────────────────────────────────────┐
│               eval::Inventory                   │
│  ┌─────────────────────────────────────────┐    │
│  │  add()  remove()  sortByName()  print() │    │
│  │  totalQuantity()  size()                │    │
│  └─────────────────────────────────────────┘    │
│  Internal: std::vector<eval::Item>              │
└──────────────────────┬──────────────────────────┘
                       │ owns (by value)
                       ▼
┌─────────────────────────────────────────────────┐
│                 eval::Item                      │
│  ┌─────────────────────────────────────────┐    │
│  │  getName()  getQuantity()               │    │
│  │  adjustQuantity()                       │    │
│  └─────────────────────────────────────────┘    │
│  Data: std::string m_name, int m_quantity       │
└─────────────────────────────────────────────────┘
```

### 3.2 Class Descriptions

#### `eval::Item`

| Attribute | Detail |
|-----------|--------|
| **Header** | `include/Item.h` |
| **Source** | `src/Item.cpp` |
| **Namespace** | `eval` |
| **Responsibility** | Encapsulates a single inventory record: a name and a mutable integer quantity. |
| **Semantics** | Value type. Copy and move are compiler-generated. |

**Public Interface:**

| Method | Description |
|--------|-------------|
| `Item(std::string, int)` | Construct with name and initial quantity. |
| `getName() const` | Return the item name (const ref, no copy). |
| `getQuantity() const` | Return the current quantity. |
| `adjustQuantity(int)` | Add a signed delta to the quantity. |

---

#### `eval::Inventory`

| Attribute | Detail |
|-----------|--------|
| **Header** | `include/Inventory.h` |
| **Source** | `src/Inventory.cpp` |
| **Namespace** | `eval` |
| **Responsibility** | Manages an ordered, in-memory collection of `Item` objects. |
| **Semantics** | Value type. Not thread-safe; requires external synchronisation for concurrent access. |

**Public Interface:**

| Method | Description |
|--------|-------------|
| `add(name, qty)` | Insert a new item or merge into an existing one. |
| `remove(name)` | Erase by name; returns `false` if not found. |
| `sortByName()` | Lexicographic sort in-place. |
| `totalQuantity() const` | Sum of all quantities. |
| `size() const` | Number of distinct item records. |
| `print() const` | Write all items to `stdout`. |

---

### 3.3 Data Flow

```
main()
  │
  ├─ inv.add("Apples", 10)    → Item{"Apples", 10} appended to m_items
  ├─ inv.add("Bananas", 5)    → Item{"Bananas", 5} appended
  ├─ inv.add("Oranges", 8)    → Item{"Oranges", 8} appended
  ├─ inv.add("Apples", 3)     → existing "Apples" found; quantity 10→13
  │
  ├─ inv.print()              → stdout: Apples:13, Bananas:5, Oranges:8
  ├─ inv.totalQuantity()      → 26
  │
  ├─ inv.sortByName()         → m_items reordered: Apples, Bananas, Oranges
  │
  └─ inv.remove("Bananas")    → m_items: Apples, Oranges
```

---

## 4. Design Rationale

### 4.1 Header / Source Separation

Each class is split into a declaration-only header (`include/`) and a
definition-only source file (`src/`). This pattern:

- Minimises recompilation scope when an implementation changes.
- Provides a clean, reviewable interface boundary.
- Aligns with IEC 62304 traceability requirements (each requirement maps to a
  distinct, identifiable software unit).

### 4.2 `eval` Namespace

All application classes are placed in the `eval` namespace to:

- Prevent name collisions with standard library or third-party symbols.
- Clearly scope evaluation-only code that must not migrate to production
  without formal review.

### 4.3 `[[nodiscard]]` Attributes

Query methods (`getName`, `getQuantity`, `totalQuantity`, `size`) are marked
`[[nodiscard]]` so the compiler emits a diagnostic when a caller silently
discards a return value. This is particularly valuable in safety-critical
code where unintentionally ignored return values can indicate logic errors.

### 4.4 `noexcept` on Accessors and Simple Mutators

Methods that cannot throw (accessors, `adjustQuantity`) are marked `noexcept`.
This enables compiler optimisations and communicates intent clearly in a
codebase where exception specifications are part of the software design
documentation.

### 4.5 Explicit Special Member Functions

All compiler-generated copy, move, and destructor declarations are written
explicitly with `= default`. This makes the intended value semantics visible
to reviewers and prevents accidental resource-ownership bugs if data members
change in future revisions.

### 4.6 No Raw Pointers or Manual Memory Management

The implementation uses only standard containers (`std::vector`, `std::string`)
and value semantics. This eliminates an entire class of memory safety defects
(dangling pointers, double-free, leaks) that are difficult to detect and could
have patient-safety implications in a medical device context.

---

## 5. Build Process

### 5.1 Prerequisites

| Tool | Minimum Version | Notes |
|------|----------------|-------|
| CMake | 3.15 | Cross-platform build system generator |
| C++ compiler | C++17 capable | MSVC 2019+, GCC 9+, or Clang 10+ |
| Git | Any | For source control |

### 5.2 Configure and Build (all platforms)

```sh
# 1. Clone the repository
git clone <repository-url>
cd EvalAIProject

# 2. Create an out-of-source build directory (keeps source tree clean)
cmake -B build -S .

# 3. Build the executable
cmake --build build

# 4. Run the application
./build/EvalAIProject          # Linux / macOS
build\Debug\EvalAIProject.exe  # Windows (MSVC, Debug configuration)
```

### 5.3 Build Configurations

| Configuration | Command | Description |
|---------------|---------|-------------|
| Debug | `cmake -B build -DCMAKE_BUILD_TYPE=Debug` | Includes debug symbols, no optimisation |
| Release | `cmake -B build -DCMAKE_BUILD_TYPE=Release` | Optimised, no debug symbols |
| RelWithDebInfo | `cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo` | Optimised with debug symbols |

### 5.4 Compiler Warning Policy

The build is configured to treat **all warnings as errors** (`/WX` on MSVC,
`-Werror` on GCC/Clang). This is a mandatory practice for safety-critical
software to prevent silent accumulation of potential defects. Do not suppress
warnings without documented justification and change-control approval.

---

## 6. Testing

Unit tests are implemented using **GoogleTest v1.14.0**, fetched automatically
at configure time via CMake `FetchContent`. No manual download is required.

### 6.1 Test Suite Summary

| Test file | Class under test | Tests | Coverage target |
|-----------|-----------------|-------|-----------------|
| `tests/test_Item.cpp` | `eval::Item` | 18 | 100 % lines / branches |
| `tests/test_Inventory.cpp` | `eval::Inventory` | 29 | 100 % lines / branches |

Every public method and every reachable branch in `src/Item.cpp` and
`src/Inventory.cpp` is exercised.

### 6.2 Prerequisites

| Tool | Minimum Version | Notes |
|------|----------------|-------|
| CMake | 3.15 | Build system generator |
| C++ compiler | C++17 capable | MSVC 2019+, GCC 9+, or Clang 10+ |
| Git | Any | Required by FetchContent to clone GoogleTest |
| Internet access | – | Required on first configure to fetch GoogleTest |

### 6.3 Build and Run Tests

```sh
# Configure (tests are ON by default)
cmake -B build -S .

# Build everything, including the test executable
cmake --build build

# Run all tests via CTest
ctest --test-dir build --output-on-failure
```

To disable test compilation (e.g. in a release pipeline):

```sh
cmake -B build -S . -DBUILD_TESTING=OFF
```

### 6.4 Running Tests Directly

After building, the test executable can be run directly for more granular
control. This is useful during development or for integrating with an IDE.

```sh
# Windows (MSVC – Debug configuration)
.\build\tests\Debug\EvalAIProject_tests.exe

# Linux / macOS
./build/tests/EvalAIProject_tests

# Filter to a specific test suite
./build/tests/EvalAIProject_tests --gtest_filter="ItemTest.*"
./build/tests/EvalAIProject_tests --gtest_filter="InventoryTest.*"

# Produce a JUnit XML report for CI integration
./build/tests/EvalAIProject_tests --gtest_output=xml:test_results.xml
```

### 6.5 Expected Output

A passing run produces output similar to:

```
[==========] Running 47 tests from 2 test suites.
[----------] 18 tests from ItemTest
[ RUN      ] ItemTest.ConstructorStoresName
[       OK ] ItemTest.ConstructorStoresName (0 ms)
...
[----------] 29 tests from InventoryTest
[ RUN      ] InventoryTest.DefaultConstructorCreatesEmptyInventory
[       OK ] InventoryTest.DefaultConstructorCreatesEmptyInventory (0 ms)
...
[==========] 47 tests from 2 test suites ran.
[  PASSED  ] 47 tests.
```

---

## 7. Code Coverage

Two coverage workflows are provided: one for GCC/Clang (lcov) and one for
Windows MSVC (OpenCppCoverage). Both restrict the report to project source
files only (`src/`, `include/`), excluding GoogleTest internals.

### 7.1 GCC / Clang – lcov + genhtml

**Prerequisites:** `lcov` and `genhtml` installed (e.g. `apt install lcov` or
`brew install lcov`).

```sh
# 1. Configure with coverage instrumentation
cmake -B build-cov -S . -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug

# 2. Build
cmake --build build-cov

# 3. Generate the HTML report (runs tests automatically)
cmake --build build-cov --target coverage

# 4. Open the report
open build-cov/coverage_report/index.html   # macOS
xdg-open build-cov/coverage_report/index.html  # Linux
```

The `coverage` CMake target performs these steps automatically:

1. Zeroes all `.gcda` counters (clean slate for this run).
2. Runs the full test suite via CTest.
3. Captures coverage data with `lcov`.
4. Filters the report to `src/` and `include/` only.
5. Renders the HTML report to `build-cov/coverage_report/`.

### 7.2 Windows MSVC – OpenCppCoverage

**Prerequisites:** Install
[OpenCppCoverage](https://github.com/OpenCppCoverage/OpenCppCoverage/releases)
and ensure its directory is in `PATH`.

```powershell
# From the repository root:
.\scripts\run_coverage_msvc.ps1

# Override directories if needed:
.\scripts\run_coverage_msvc.ps1 -BuildDir build-cov -ReportDir my_report
```

The script:

1. Verifies `OpenCppCoverage.exe` is in `PATH`.
2. Configures and builds a Debug test binary via CMake.
3. Runs `OpenCppCoverage.exe` against the test executable.
4. Writes an HTML report to `coverage_report\index.html`.

### 7.3 Coverage Report Interpretation

| Column | Meaning |
|--------|---------|
| **Lines** | Percentage of executable source lines reached by at least one test |
| **Branches** | Percentage of conditional branches (both true and false paths) exercised |
| **Functions** | Percentage of functions called at least once |

A target of **100 % line and branch coverage** is met when all entries in the
`src/` section of the report show 100 %. Per IEC 62304, the minimum coverage
threshold for Class B and C software units must be documented in the Software
Development Plan.

---

## 8. Coding Standards

| Aspect | Convention |
|--------|-----------|
| **File headers** | Every file carries a Doxygen-compatible header with author, date, version, revision history, and safety classification. |
| **Documentation** | All public symbols are documented with `@brief`, `@details`, `@param`, `@return`, and `@note` tags as applicable. |
| **Naming** | `PascalCase` for types, `camelCase` for methods, `m_` prefix for private data members, `ALL_CAPS` for macros (none used). |
| **Formatting** | Allman brace style (opening brace on its own line). 4-space indentation. 100-character line limit. |
| **Include guards** | `#pragma once` (supported by all targeted toolchains; preferred over double-underscore macros). |
| **Namespaces** | All application code lives in the `eval` namespace. Closing brace annotated `// namespace eval`. |
| **Exception safety** | No exceptions are thrown by this application. `noexcept` is applied where the guarantee can be upheld. |
| **MISRA alignment** | MISRA C++ 2008 rules are considered during development. A formal MISRA compliance report requires a dedicated static analysis tool. |

---

## 9. Applicable Standards

| Standard | Title | Relevance |
|----------|-------|-----------|
| IEC 62304:2006+AMD1:2015 | Medical device software – Software life cycle processes | Primary SW development standard |
| ISO 14971:2019 | Medical devices – Application of risk management | Risk management framework |
| IEC 62366-1:2015 | Medical devices – Usability engineering | Usability of software UI elements |
| MISRA C++:2008 | Guidelines for the use of the C++ language in critical systems | Coding guidelines |
| ISO/IEC 14882:2017 | Programming languages – C++ (C++17) | Language standard |

---

## 10. Revision History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0.0 | 2026-05-15 | Seth Laurie | Initial release – project structure, Item and Inventory classes, CMake build, README |
| 1.1.0 | 2026-05-15 | Seth Laurie | Add unit-test suite (GoogleTest), code-coverage support (lcov + OpenCppCoverage), updated README |
