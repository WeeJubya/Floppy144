$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$testSource = Join-Path $PSScriptRoot "stage2_headless_tests.c"
$testObjDir = Join-Path $root "obj\tests\stage2"
$testExe = Join-Path $testObjDir "stage2_headless_tests.exe"

if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw "cl.exe was not found. Run this from a Visual Studio Developer PowerShell/Command Prompt."
}

New-Item -ItemType Directory -Force $testObjDir | Out-Null

$sources = @(
    $testSource,
    "game\src\floppy144_game_data.c",
    "game\src\floppy144_trigger_engine.c",
    "game\src\floppy144_interaction_engine.c",
    "game\src\floppy144_drawing_runtime.c",
    "game\src\floppy144_run_state.c",
    "game\src\floppy144_world.c",
    "game\src\floppy144_draw.c",
    "game\src\floppy144_site.c",
    "game\src\floppy144_site_rooms.c",
    "game\src\floppy144_site_object.c",
    "game\src\floppy144_object_registry.c",
    "game\src\floppy144_collection_registry.c"
)

Push-Location $root
try {
    Write-Host "`n=== BUILD STAGE 2 HEADLESS REGRESSION ==="

    # Compile each source separately so all intermediate objects stay under
    # obj/tests rather than appearing in the source tree.
    $objects = @()
    foreach($source in $sources) {
        $base = [IO.Path]::GetFileNameWithoutExtension($source)
        if($source -eq $testSource) {
            $base = "stage2_headless_tests"
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
            /O2 `
            /Iinclude `
            /Igame\src `
            "/Fo:$object" `
            $source

        if($LASTEXITCODE -ne 0) {
            throw "Stage 2 regression compile failed for $source with exit code $LASTEXITCODE."
        }
    }

    Write-Host "`n=== LINK STAGE 2 HEADLESS REGRESSION ==="
    & link.exe /nologo "/OUT:$testExe" $objects

    if($LASTEXITCODE -ne 0) {
        throw "Stage 2 regression link failed with exit code $LASTEXITCODE."
    }

    Write-Host "`n=== RUN STAGE 2 HEADLESS REGRESSION ==="
    & $testExe

    if($LASTEXITCODE -ne 0) {
        throw "Stage 2 regression failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}
