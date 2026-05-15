# ==============================================================================
# run_coverage_msvc.ps1
#
# Generates an HTML code-coverage report for EvalAIProject on Windows using
# OpenCppCoverage (https://github.com/OpenCppCoverage/OpenCppCoverage).
#
# Prerequisites
# -------------
#   1. CMake 3.15 or later in PATH
#   2. A C++17-capable MSVC compiler (Visual Studio 2019 or 2022)
#   3. OpenCppCoverage installed and its install directory in PATH
#      Download: https://github.com/OpenCppCoverage/OpenCppCoverage/releases
#
# Usage
# -----
#   # From the repository root:
#   .\scripts\run_coverage_msvc.ps1
#
#   # Override build directory or report output directory:
#   .\scripts\run_coverage_msvc.ps1 -BuildDir build-cov -ReportDir my_report
#
# Output
# ------
#   <ReportDir>\index.html  – Open this file in any browser to view the report.
#
# Revision History:
#   1.1.0  2026-05-15  Seth Laurie  Initial script
# ==============================================================================

param(
    [string]$BuildDir  = "build-coverage",
    [string]$ReportDir = "coverage_report"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ------------------------------------------------------------------------------
# Step 0 – Verify OpenCppCoverage is available
# ------------------------------------------------------------------------------
$ocpExe = Get-Command "OpenCppCoverage.exe" -ErrorAction SilentlyContinue
if (-not $ocpExe) {
    Write-Error @"
OpenCppCoverage.exe not found in PATH.

Install steps:
  1. Download the latest installer from:
     https://github.com/OpenCppCoverage/OpenCppCoverage/releases
  2. Run the installer and ensure 'Add to PATH' is selected.
  3. Open a new terminal and re-run this script.
"@
    exit 1
}
Write-Host "Found OpenCppCoverage: $($ocpExe.Source)"

# Resolve absolute paths relative to the script's parent (the repo root).
$RepoRoot  = (Resolve-Path "$PSScriptRoot\..").Path
$BuildPath = Join-Path $RepoRoot $BuildDir
$ReportPath = Join-Path $RepoRoot $ReportDir

# ------------------------------------------------------------------------------
# Step 1 – Configure (Debug build; no coverage flags needed for OpenCppCoverage)
# ------------------------------------------------------------------------------
Write-Host "`n[1/4] Configuring CMake (Debug, BUILD_TESTING=ON)..."
cmake -B $BuildPath -S $RepoRoot -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
if ($LASTEXITCODE -ne 0) { Write-Error "CMake configure failed."; exit 1 }

# ------------------------------------------------------------------------------
# Step 2 – Build
# ------------------------------------------------------------------------------
Write-Host "`n[2/4] Building..."
cmake --build $BuildPath --config Debug
if ($LASTEXITCODE -ne 0) { Write-Error "CMake build failed."; exit 1 }

# ------------------------------------------------------------------------------
# Step 3 – Locate test executable
# ------------------------------------------------------------------------------
Write-Host "`n[3/4] Locating test executable..."
$candidates = @(
    (Join-Path $BuildPath "tests\Debug\EvalAIProject_tests.exe"),
    (Join-Path $BuildPath "tests\EvalAIProject_tests.exe"),
    (Join-Path $BuildPath "Debug\EvalAIProject_tests.exe"),
    (Join-Path $BuildPath "EvalAIProject_tests.exe")
)
$testExe = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $testExe) {
    Write-Error "Could not locate EvalAIProject_tests.exe under $BuildPath."
    exit 1
}
Write-Host "Test executable: $testExe"

# ------------------------------------------------------------------------------
# Step 4 – Run OpenCppCoverage and produce HTML report
# ------------------------------------------------------------------------------
Write-Host "`n[4/4] Running coverage analysis..."

# --sources restricts instrumentation to our own source files only, excluding
# GoogleTest internals and system headers from the report.
OpenCppCoverage.exe `
    --sources "$RepoRoot\src" `
    --sources "$RepoRoot\include" `
    --export_type "html:$ReportPath" `
    --working_dir $BuildPath `
    -- $testExe

if ($LASTEXITCODE -ne 0) {
    Write-Error "OpenCppCoverage reported a non-zero exit code."
    exit 1
}

$indexHtml = Join-Path $ReportPath "index.html"
Write-Host "`nCoverage report generated successfully."
Write-Host "Open the report: $indexHtml"
