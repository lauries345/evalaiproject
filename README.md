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
7. [Coding Standards](#7-coding-standards)
8. [Applicable Standards](#8-applicable-standards)
9. [Revision History](#9-revision-history)

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
└── tests/                  # Unit and integration test stubs (currently empty)
    └── .gitkeep
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

A `tests/` directory is reserved for unit and integration tests. The
`CMakeLists.txt` includes a commented-out block for enabling CTest and a test
subdirectory. Recommended frameworks:

- **GoogleTest** – widely used, integrates cleanly with CMake via `FetchContent`
- **Catch2** – header-only option, lower integration overhead

Per IEC 62304, Class B and C software units require documented unit test
coverage. Before promoting this evaluation build to a regulated context:

1. Enable `enable_testing()` in `CMakeLists.txt`.
2. Add `tests/CMakeLists.txt` with test targets.
3. Write unit tests for each public method of `Item` and `Inventory`.
4. Establish and document a minimum coverage threshold.

---

## 7. Coding Standards

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

## 8. Applicable Standards

| Standard | Title | Relevance |
|----------|-------|-----------|
| IEC 62304:2006+AMD1:2015 | Medical device software – Software life cycle processes | Primary SW development standard |
| ISO 14971:2019 | Medical devices – Application of risk management | Risk management framework |
| IEC 62366-1:2015 | Medical devices – Usability engineering | Usability of software UI elements |
| MISRA C++:2008 | Guidelines for the use of the C++ language in critical systems | Coding guidelines |
| ISO/IEC 14882:2017 | Programming languages – C++ (C++17) | Language standard |

---

## 9. Revision History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0.0 | 2026-05-15 | Seth Laurie | Initial release – project structure, Item and Inventory classes, CMake build, README |
