$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$sourceDir = Join-Path $root "game\src"
$testSource = Join-Path $PSScriptRoot "stage4_persistence_paths_tests.c"

Write-Host "=== STAGE 4 PERSISTENCE PATH BOUNDARY AUDIT ==="

$mainSource = Get-Content -LiteralPath (Join-Path $sourceDir "floppy144_main.c") -Raw
$platformHeader = Get-Content -LiteralPath (Join-Path $root "include\f144_platform.h") -Raw
$win32StorageSource = Get-Content -LiteralPath (Join-Path $root "src\f144_win32_storage.c") -Raw

foreach($leaf in @(
    'floppy144_manual.sav',
    'floppy144_auto.sav',
    'floppy144_profile.dat',
    'floppy144_settings.dat'
))
{
    if($mainSource -match [regex]::Escape($leaf))
    {
        throw "Game coordinator still embeds writable filename: $leaf"
    }
}

foreach($required in @(
    'F144_PERSISTENCE_MANUAL_SAVE',
    'F144_PERSISTENCE_AUTOSAVE',
    'F144_PERSISTENCE_PROFILE',
    'F144_PERSISTENCE_SETTINGS',
    'f144PlatformPersistencePath',
    'f144PlatformLegacyPersistencePath'
))
{
    if($platformHeader -notmatch [regex]::Escape($required))
    {
        throw "Platform persistence-path contract is missing: $required"
    }
}

foreach($required in @(
    'SHGetFolderPathA',
    'CSIDL_APPDATA',
    'CSIDL_FLAG_CREATE',
    '"Floppy144"',
    'CreateDirectoryA',
    'GetCurrentDirectoryA',
    'GetModuleFileNameA'
))
{
    if($win32StorageSource -notmatch [regex]::Escape($required))
    {
        throw "Win32 persistence path provider is missing: $required"
    }
}

if(
    $mainSource -match 'GetFileAttributesA\s*\(' -or
    $mainSource -match '%APPDATA%' -or
    $mainSource -match 'AppData'
)
{
    throw "Game coordinator still knows Windows persistence-path details."
}

Write-Host "STAGE 4 PERSISTENCE PATH BOUNDARY AUDIT: PASS"

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4-persistence"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-persistence"
}

if(Test-Path -LiteralPath $tempDir)
{
    Remove-Item -LiteralPath $tempDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$env:F144_TEST_ROOT = $tempDir

$testObjDir = Join-Path $tempDir "obj"
$testExe = Join-Path $tempDir "stage4_persistence_paths_tests.exe"
New-Item -ItemType Directory -Force -Path $testObjDir | Out-Null

$sources = @(
    $testSource,
    "src\f144_platform.c",
    "src\f144_win32_storage.c",
    "game\src\floppy144_storage.c",
    "game\src\floppy144_game_data.c",
    "game\src\floppy144_trigger_engine.c",
    "game\src\floppy144_interaction_engine.c",
    "game\src\floppy144_run_state.c",
    "game\src\floppy144_world.c",
    "game\src\floppy144_terminal.c",
    "game\src\floppy144_recovery.c",
    "game\src\floppy144_catalogue.c",
    "game\src\floppy144_document.c",
    "game\src\floppy144_variation.c",
    "game\src\floppy144_effect.c",
    "game\src\floppy144_persistence.c",
    "game\src\floppy144_profile.c",
    "game\src\floppy144_profile_edit.c",
    "game\src\floppy144_profile_view.c",
    "game\src\floppy144_player_visual.c",
    "game\src\floppy144_settings.c",
    "game\src\floppy144_draw.c",
    "game\src\floppy144_site.c",
    "game\src\floppy144_site_rooms.c",
    "game\src\floppy144_site_object.c",
    "game\src\floppy144_grey_door.c",
    "game\src\floppy144_grey_encounter.c",
    "game\src\floppy144_object_registry.c",
    "game\src\floppy144_collection_registry.c"
)

Push-Location $root
try
{
    Write-Host ""
    Write-Host "=== BUILD STAGE 4 PERSISTENCE PATH REGRESSION ==="

    $objects = @()

    foreach($source in $sources)
    {
        $base = [IO.Path]::GetFileNameWithoutExtension($source)
        if($source -eq $testSource)
        {
            $base = "stage4_persistence_paths_tests"
        }

        $object = Join-Path $testObjDir ($base + ".obj")
        $objects += $object

        $compileArgs = @(
            "/nologo",
            "/c",
            "/TC",
            "/std:c11",
            "/utf-8",
            "/W4",
            "/WX",
            "/wd4505",
            "/O2",
            "/Iinclude",
            "/Igame\src",
            "/Fo:$object",
            $source
        )

        & cl.exe @compileArgs

        if($LASTEXITCODE -ne 0)
        {
            throw "Stage 4 persistence regression compile failed for $source with exit code $LASTEXITCODE."
        }
    }

    Write-Host ""
    Write-Host "=== LINK STAGE 4 PERSISTENCE PATH REGRESSION ==="

    $linkArgs = @(
        "/nologo",
        "/OUT:$testExe"
    ) + $objects + @(
        "user32.lib",
        "shell32.lib"
    )

    & link.exe @linkArgs

    if($LASTEXITCODE -ne 0)
    {
        throw "Stage 4 persistence regression link failed with exit code $LASTEXITCODE."
    }

    Write-Host ""
    Write-Host "=== RUN STAGE 4 PERSISTENCE PATH REGRESSION ==="

    & $testExe

    if($LASTEXITCODE -ne 0)
    {
        throw "Stage 4 persistence regression failed with exit code $LASTEXITCODE."
    }
}
finally
{
    Pop-Location
    Remove-Item Env:F144_TEST_ROOT -ErrorAction SilentlyContinue
}
