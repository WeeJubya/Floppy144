$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$testSource = Join-Path $PSScriptRoot "stage3a_prologue_tests.c"
$testObjDir = Join-Path $root "obj\tests\stage3a"
$testExe = Join-Path $testObjDir "stage3a_prologue_tests.exe"

if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw "cl.exe was not found. Run this from a Visual Studio Developer PowerShell/Command Prompt."
}

New-Item -ItemType Directory -Force $testObjDir | Out-Null

$sources = @(
    $testSource,
    "game\src\floppy144_game_data.c",
    "game\src\floppy144_trigger_engine.c",
    "game\src\floppy144_interaction_engine.c",
    "game\src\floppy144_run_state.c",
    "game\src\floppy144_world.c",
    "game\src\floppy144_terminal.c",
    "game\src\floppy144_recovery.c",
    "game\src\floppy144_catalogue.c",
    "game\src\floppy144_document.c",
    "game\src\floppy144_effect.c",
    "game\src\floppy144_persistence.c",
    "game\src\floppy144_profile.c",
    "game\src\floppy144_settings.c",
    "game\src\floppy144_draw.c",
    "game\src\floppy144_site.c",
    "game\src\floppy144_site_rooms.c",
    "game\src\floppy144_site_object.c",
    "game\src\floppy144_object_registry.c",
    "game\src\floppy144_collection_registry.c"
)

Push-Location $root
try {
    Write-Host "`n=== BUILD STAGE 3A PROLOGUE REGRESSION ==="

    $objects = @()
    foreach($source in $sources) {
        $base = [IO.Path]::GetFileNameWithoutExtension($source)
        if($source -eq $testSource) {
            $base = "stage3a_prologue_tests"
        }
        $object = Join-Path $testObjDir ($base + ".obj")
        $objects += $object

        & cl.exe `
            /nologo `
            /c `
            /TC `
            /std:c11 `
            /utf-8 `
            /W4 `
            /WX `
            /wd4505 `
            /O2 `
            /Iinclude `
            /Igame\src `
            "/Fo:$object" `
            $source

        if($LASTEXITCODE -ne 0) {
            throw "Stage 3A regression compile failed for $source with exit code $LASTEXITCODE."
        }
    }

    Write-Host "`n=== LINK STAGE 3A PROLOGUE REGRESSION ==="
    & link.exe /nologo "/OUT:$testExe" $objects user32.lib

    if($LASTEXITCODE -ne 0) {
        throw "Stage 3A regression link failed with exit code $LASTEXITCODE."
    }

    Write-Host "`n=== RUN STAGE 3A PROLOGUE REGRESSION ==="
    & $testExe

    if($LASTEXITCODE -ne 0) {
        throw "Stage 3A regression failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}
