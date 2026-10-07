$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root =
    Split-Path -Parent $PSScriptRoot

$mainPath =
    Join-Path $root "game\src\floppy144_main.c"

$profileHeaderPath =
    Join-Path $root "game\src\floppy144_profile.h"

$profileSourcePath =
    Join-Path $root "game\src\floppy144_profile.c"

$editorPath =
    Join-Path $root "game\src\floppy144_profile_edit.c"

$viewPath =
    Join-Path $root "game\src\floppy144_profile_view.c"

$persistenceHeaderPath =
    Join-Path $root "game\src\floppy144_persistence.h"

$testSource =
    Join-Path $PSScriptRoot "stage4_profile_name_tests.c"

Write-Host "=== STAGE 4C OPERATOR NAME BOUNDARY AUDIT ==="

$mainSource =
    Get-Content -LiteralPath $mainPath -Raw

$profileHeader =
    Get-Content -LiteralPath $profileHeaderPath -Raw

$profileSource =
    Get-Content -LiteralPath $profileSourcePath -Raw

$editorSource =
    Get-Content -LiteralPath $editorPath -Raw

$viewSource =
    Get-Content -LiteralPath $viewPath -Raw

$persistenceHeader =
    Get-Content -LiteralPath $persistenceHeaderPath -Raw

foreach($required in @(
    'FLOPPY144_PROFILE_NAME_CAPACITY          32U',
    'FLOPPY144_PROFILE_NAME_MAX_LENGTH',
    'Floppy144DiscoveryProfileOperatorNameValid',
    'Floppy144DiscoveryProfileOperatorNameCharacterSupported',
    'Floppy144DiscoveryProfileOperatorName'
))
{
    if(
        $profileHeader -notmatch
        [regex]::Escape($required)
    )
    {
        throw "Operator-name storage/validation contract is missing: $required"
    }
}

if(
    $persistenceHeader -notmatch
        'FLOPPY144_PROFILE_VERSION\s+1U' -or
    $persistenceHeader -notmatch
        'FLOPPY144_PROFILE_PAYLOAD_V1_SIZE\s+64U'
)
{
    throw "S4C-02 unexpectedly changed the existing profile V1 persistence format."
}

foreach($required in @(
    'Floppy144ProfileNameEditBegin',
    'Floppy144ProfileNameEditInputCodepoint',
    'Floppy144ProfileNameEditBackspace',
    'Floppy144ProfileNameEditReadyToSave',
    'Floppy144ProfileNameEditCancel'
))
{
    if(
        $editorSource -notmatch
        [regex]::Escape($required)
    )
    {
        throw "Portable profile-name editor is missing: $required"
    }
}

if(
    $editorSource -match
        '(?i)windows\.h|WM_CHAR|VK_|HWND|WPARAM|LPARAM'
)
{
    throw "Portable profile-name editor contains Win32 input details."
}

foreach($required in @(
    'F144TextInputEvent',
    'Floppy144ProfileNameEditInputCodepoint',
    'Floppy144ProfileNameEditBackspace',
    'Floppy144CommitProfileNameEdit',
    'Floppy144PersistenceSaveProfile',
    'Floppy144ProfileNameEditCancel'
))
{
    if(
        $mainSource -notmatch
        [regex]::Escape($required)
    )
    {
        throw "Profile-name coordinator wiring is missing: $required"
    }
}

if(
    $mainSource -notmatch
        'global_screen\s*==\s*FLOPPY144_SCREEN_PROFILE[\s\S]*?Floppy144ProfileNameEditActive[\s\S]*?Floppy144ProfileNameEditInputCodepoint'
)
{
    throw "Profile name text does not route through F144TextInputEvent handling."
}

if(
    $mainSource -match
        'case\s+WM_CHAR:[\s\S]{0,600}operator_name'
)
{
    throw "Operator-name handling was added directly to the Win32 message path."
}

if(
    $mainSource -notmatch
        '!Floppy144DiscoveryProfileHasOperatorName[\s\S]*?Floppy144ProfileNameEditBegin[\s\S]*?true'
)
{
    throw "Fresh Profile does not enter the optional first-time setup flow."
}

if(
    $mainSource -notmatch
        'F144_ACTION_MENU[\s\S]*?FLOPPY144_SCREEN_PROFILE[\s\S]*?Floppy144ProfileNameEditCancel'
)
{
    throw "Escape does not cancel operator-name editing."
}

if(
    $viewSource -notmatch
        'ENTER  EDIT NAME' -or
    $viewSource -notmatch
        'ENTER  SAVE' -or
    $viewSource -notmatch
        'BACKSPACE  DELETE'
)
{
    throw "Profile screen does not explain edit controls."
}

if(
    $profileSource -match
        'toupper|_strupr|CharUpper|CharUpperBuff'
)
{
    throw "Operator-name storage must not be forced to uppercase."
}

Write-Host "STAGE 4C OPERATOR NAME BOUNDARY AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4C OPERATOR NAME REGRESSION ==="

$tempDir =
    if($env:RUNNER_TEMP)
    {
        Join-Path $env:RUNNER_TEMP "floppy144-stage4-profile-name"
    }
    else
    {
        Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-profile-name"
    }

New-Item -ItemType Directory -Force -Path $tempDir |
    Out-Null

$exePath =
    Join-Path $tempDir "stage4_profile_name_tests.exe"

$includePlatform =
    "/I" +
    (Join-Path $root "include")

$includeGame =
    "/I" +
    (Join-Path $root "game\src")

$compileArgs = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/utf-8",
    $testSource,
    (Join-Path $root "game\src\floppy144_profile.c"),
    (Join-Path $root "game\src\floppy144_profile_edit.c"),
    $includePlatform,
    $includeGame,
    "/Fe$exePath"
)

& cl.exe @compileArgs

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4C operator-name regression build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "=== RUN STAGE 4C OPERATOR NAME REGRESSION ==="

& $exePath

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4C operator-name regression failed with exit code $LASTEXITCODE."
}
