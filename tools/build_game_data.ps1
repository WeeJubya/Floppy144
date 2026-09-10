param(
    [string]$InputFile = "data\floppy144_game_data.json"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$compilerSource = Join-Path $PSScriptRoot "game_data_compiler.c"
$inputPath = Join-Path $root $InputFile
$outputDir = Join-Path $root "game\src"
$toolObjDir = Join-Path $root "obj\tools"
$compilerExe = Join-Path $toolObjDir "game_data_compiler.exe"
$compilerObj = Join-Path $toolObjDir "game_data_compiler.obj"

if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw "cl.exe was not found. Run this from a Visual Studio Developer PowerShell/Command Prompt."
}

New-Item -ItemType Directory -Force $toolObjDir, $outputDir | Out-Null

Push-Location $root
try {
    Write-Host "`n=== BUILD GAME-DATA COMPILER ==="

    & cl.exe `
        /nologo `
        /TC `
        /std:c11 `
        /utf-8 `
        /W4 `
        /WX `
        /O2 `
        "/Fo:$compilerObj" `
        "/Fe:$compilerExe" `
        $compilerSource

    if($LASTEXITCODE -ne 0) {
        throw "Game-data compiler build failed with exit code $LASTEXITCODE."
    }

    Write-Host "`n=== COMPILE CANONICAL GAME DATA ==="
    & $compilerExe $inputPath $outputDir

    if($LASTEXITCODE -ne 0) {
        throw "Game-data compilation failed with exit code $LASTEXITCODE."
    }

    # Public enum headers include the historical .def names. Keep those files
    # generated too, so stable IDs and the runtime table can never drift apart.
    Copy-Item "$outputDir\floppy144_collections.generated.def" "$outputDir\floppy144_collections.def" -Force
    Copy-Item "$outputDir\floppy144_triggers.generated.def" "$outputDir\floppy144_triggers.def" -Force
    Copy-Item "$outputDir\floppy144_interactions.generated.def" "$outputDir\floppy144_interactions.def" -Force
    Copy-Item "$outputDir\floppy144_evidence.generated.def" "$outputDir\floppy144_evidence.def" -Force

    Write-Host "`n=== VALIDATE GENERATED SITE ==="
    & "$PSScriptRoot\build_site.ps1" `
        -InputFile "game\src\site_layout.generated.jsonc" `
        -OutputFile "game\src\floppy144_site_generated.def"

    if($LASTEXITCODE -ne 0) {
        throw "Generated Site validation failed with exit code $LASTEXITCODE."
    }

    Write-Host "`nGAME DATA: PASS" -ForegroundColor Green
}
finally {
    Pop-Location
}
