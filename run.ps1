param
(
    [ValidateSet("debug", "asan", "release")]
    [string]$build = "release"
)

& {
    $ErrorActionPreference = "Stop"
    Set-StrictMode -Version Latest

    $maximumBytes = 1474560
    $solutionPath = ".\build\Floppy144.sln"
    $releaseExe = ".\bin\release\Floppy144.exe"

    New-Item -ItemType Directory -Force .\obj, .\build, .\bin | Out-Null

    Write-Host ""
    Write-Host "=== FLOPPY//144 STAGE 2 DATA BUILD ==="
    & .\tools\build_game_data.ps1

    if($LASTEXITCODE -ne 0) {
        throw "Game-data build failed with exit code $LASTEXITCODE."
    }

    Write-Host ""
    Write-Host "=== FLOPPY//144 STAGE 2 REGRESSION ==="
    & .\tools\test_stage2.ps1

    if($LASTEXITCODE -ne 0) {
        throw "Stage 2 regression failed with exit code $LASTEXITCODE."
    }

    Write-Host ""
    Write-Host "=== FLOPPY//144 STAGE 3A PROLOGUE REGRESSION ==="
    & .\tools\test_stage3a.ps1

    if($LASTEXITCODE -ne 0) {
        throw "Stage 3A Prologue regression failed with exit code $LASTEXITCODE."
    }

    & .\tools\test_stage3b.ps1

    if(-not (Get-Command premake5 -ErrorAction SilentlyContinue)) {
        throw "premake5 was not found on PATH."
    }

    if(-not (Get-Command MSBuild -ErrorAction SilentlyContinue)) {
        throw "MSBuild was not found. Run this from a Visual Studio Developer PowerShell/Command Prompt."
    }

    Write-Host ""
    Write-Host "=== GENERATE VISUAL STUDIO SOLUTION ==="
    & premake5 vs2022

    if($LASTEXITCODE -ne 0) {
        throw "Premake generation failed with exit code $LASTEXITCODE."
    }

    Write-Host ""
    Write-Host "=== BUILD FLOPPY//144 ($build) ==="
    & MSBuild `
        $solutionPath `
        /t:Floppy144 `
        /p:Configuration=$build `
        /p:Platform=windows `
        /m

    if($LASTEXITCODE -ne 0) {
        throw "MSBuild failed with exit code $LASTEXITCODE."
    }

    if($build -eq "release") {
        if(-not (Test-Path -LiteralPath $releaseExe)) {
            throw "Release executable was not produced: $releaseExe"
        }

        $usedBytes = (Get-Item -LiteralPath $releaseExe).Length
        $remainingBytes = $maximumBytes - $usedBytes

        Write-Host ""
        Write-Host "FLOPPY//144 RELEASE SIZE"
        Write-Host ""
        Write-Host ("Used:      {0:N0} bytes" -f $usedBytes)
        Write-Host ("Remaining: {0:N0} bytes" -f $remainingBytes)
        Write-Host ("Maximum:   {0:N0} bytes" -f $maximumBytes)
        Write-Host ""

        if($usedBytes -gt $maximumBytes) {
            Write-Host "SIZE GATE: FAIL"
            throw "Floppy//144 exceeds the 1.44 MB submission limit by $(-$remainingBytes) bytes."
        }

        Write-Host "SIZE GATE: PASS" -ForegroundColor Green
    }
}
