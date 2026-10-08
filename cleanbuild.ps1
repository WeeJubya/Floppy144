# FLOPPY//144 clean validation followed by a normal Release build.
# Ordered strictly: Git diff, original cleanup, regressions, build/run.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

Push-Location $PSScriptRoot
try {
    Write-Host ""
    Write-Host "=== FLOPPY//144 GIT DIFFERENCE CHECK ==="
    git diff --check
    if($LASTEXITCODE -ne 0) {
        throw "Git diff --check failed with exit code $LASTEXITCODE."
    }

    Write-Host ""
    Write-Host "=== FLOPPY//144 PREVIOUS BUILD CLEANING ==="
    Remove-Item .\build -Recurse -Force -ErrorAction SilentlyContinue
    Remove-Item .\obj -Recurse -Force -ErrorAction SilentlyContinue
    Remove-Item .\bin -Recurse -Force -ErrorAction SilentlyContinue

    Write-Host ""
    Write-Host "=== FLOPPY//144 TESTS ==="
    $global:LASTEXITCODE = 0
    & (Join-Path $PSScriptRoot 'testbuild.ps1')
    if($LASTEXITCODE -ne 0) {
        throw "Regression suite failed with exit code $LASTEXITCODE. Build was not started."
    }

    Write-Host ""
    Write-Host "=== FLOPPY//144 BUILD/RUN ==="
    $global:LASTEXITCODE = 0
    & (Join-Path $PSScriptRoot 'run.ps1') -build release
    if($LASTEXITCODE -ne 0) {
        throw "Release build/run failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}
