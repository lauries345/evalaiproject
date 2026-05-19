#!/usr/bin/env bash
# Build EvalAIProject and produce an HTML coverage report via lcov + genhtml.
# Coverage is scoped to src/ and include/.
#
# Usage: ./scripts/run_coverage_lcov.sh [build_dir]
set -euo pipefail

BUILD=${1:-build}

cmake -S . -B "$BUILD" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCODE_COVERAGE=ON \
    -DBUILD_TESTING=ON
cmake --build "$BUILD" --parallel
cmake --build "$BUILD" --target coverage

echo
echo "Coverage report: $BUILD/coverage_html/index.html"
