<#
.SYNOPSIS
    Build EvalAIProject in Debug and produce an HTML coverage report
    using OpenCppCoverage.

.DESCRIPTION
    Coverage is scoped to src/ and include/. GoogleTest internals and
    the tests/ directory are excluded.

.PARAMETER BuildDir
    Out-of-source build directory. Default: build

.PARAMETER OutputDir
    HTML output directory. Default: coverage_html

.EXAMPLE
    .\scripts\run_coverage_msvc.ps1
#>

param(
    [string]$BuildDir  = "build",
    [string]$OutputDir = "coverage_html"
)

$ErrorActionPreference = "Stop"

if (-not (Get-Command OpenCppCoverage.exe -ErrorAction SilentlyContinue)) {
    Write-Error "OpenCppCoverage.exe not found on PATH. Install from https://github.com/OpenCppCoverage/OpenCppCoverage"
}

cmake -S . -B $BuildDir -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build $BuildDir --config Debug

$testExe = Join-Path $BuildDir "tests\Debug\unit_tests.exe"
if (-not (Test-Path $testExe)) {
    Write-Error "Test executable not found at $testExe"
}

$srcRoot = (Resolve-Path "src").Path
$incRoot = (Resolve-Path "include").Path

OpenCppCoverage.exe `
    --sources $srcRoot `
    --sources $incRoot `
    --excluded_sources "*\_deps\*" `
    --excluded_sources "*\tests\*" `
    --export_type "html:$OutputDir" `
    -- $testExe --gtest_output="xml:$BuildDir\test-results.xml"

Write-Host "Coverage report written to $OutputDir\index.html"
