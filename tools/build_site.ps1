param(
    [string]$InputFile = "site_layout.jsonc",
    [string]$OutputFile = "game\src\floppy144_site_generated.def"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$compilerSource = Join-Path $PSScriptRoot "site_compiler.c"
$compilerExe = Join-Path $PSScriptRoot "site_compiler.exe"
$compilerObj = Join-Path $PSScriptRoot "site_compiler.obj"
$inputPath = Join-Path $root $InputFile
$outputPath = Join-Path $root $OutputFile

Push-Location $root
try {
    if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        throw "cl.exe was not found. Run this from a Visual Studio Developer PowerShell/Command Prompt."
    }

    Write-Host "`n=== BUILD SITE COMPILER ==="

    & cl.exe `
        /nologo `
        /TC `
        /std:c11 `
        /W4 `
        /WX `
        /O2 `
        "/Fo:$compilerObj" `
        "/Fe:$compilerExe" `
        $compilerSource

    if($LASTEXITCODE -ne 0) {
        throw "Site compiler build failed with exit code $LASTEXITCODE."
    }

    Write-Host "`n=== COMPILE SITE DATA ==="

    & $compilerExe $inputPath $outputPath

    if($LASTEXITCODE -ne 0) {
        throw "Site data compilation failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}
