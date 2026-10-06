$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$sourceDir = Join-Path $root "game\src"
$inputSource = Join-Path $root "src\f144_win32_input.c"
$movementSource = Join-Path $sourceDir "floppy144_input.c"
$testSource = Join-Path $PSScriptRoot "stage4_input_tests.c"

Write-Host "=== STAGE 4 LOGICAL INPUT BOUNDARY AUDIT ==="

$gameSourceFiles = Get-ChildItem -Path $sourceDir -File |
    Where-Object { $_.Extension -eq ".c" -or $_.Extension -eq ".h" }

$nativeKeyLeaks = foreach($file in $gameSourceFiles)
{
    Select-String -LiteralPath $file.FullName -Pattern '\bVK_[A-Z0-9_]+\b'
}

if($nativeKeyLeaks)
{
    $nativeKeyLeaks | ForEach-Object {
        Write-Host "$($_.Path):$($_.LineNumber): $($_.Line.Trim())"
    }

    throw "Game source still depends directly on Win32 VK_* constants."
}

$mainPath = Join-Path $sourceDir "floppy144_main.c"
$mainSource = Get-Content -LiteralPath $mainPath -Raw

foreach($required in @(
    'f144Win32TranslateKeyEvent',
    'F144ActionEvent',
    'F144TextInputEvent',
    'f144Win32TranslateTextEvent',
    'Floppy144HandleTextInput',
    'Floppy144MovementInputSetAction',
    'Floppy144MovementInputVector',
    'F144_ACTION_ACCESS',
    'F144_ACTION_INSPECT',
    'case WM_CHAR:',
    'Floppy144TerminalInputCharacter',
    'Floppy144TerminalBackspace',
    'Floppy144TerminalSubmitInput',
    'Floppy144CabinetInputDigit'
))
{
    if($mainSource -notmatch [regex]::Escape($required))
    {
        throw "Logical input coordinator contract is missing: $required"
    }
}

if(
    $mainSource -match 'w_param\s*==\s*VK_' -or
    $mainSource -match 'case\s+VK_' -or
    $mainSource -match 'switch\s*\(\s*w_param\s*\)'
)
{
    throw "Win32 physical keys are still driving game-screen policy directly."
}

$platformHeader = Get-Content -LiteralPath (Join-Path $root "include\f144_platform.h") -Raw
if($platformHeader -match 'F144_ACTION_MOVE_(UP_LEFT|UP_RIGHT|DOWN_LEFT|DOWN_RIGHT)')
{
    throw "Diagonal movement must be derived from simultaneous cardinal actions, not exposed as platform actions."
}

foreach($contract in @(
    'case F144_ACTION_ACCESS:[\s\S]*?FLOPPY144_OFFICE_INTERACTION_ACCESS',
    'case F144_ACTION_INSPECT:[\s\S]*?FLOPPY144_OFFICE_INTERACTION_INSPECT',
    'case F144_ACTION_MOVE_LEFT:[\s\S]*?Floppy144MovementInputVector',
    'case F144_ACTION_PAGE_UP:[\s\S]*?Floppy144CataloguePage',
    'case F144_ACTION_PAGE_DOWN:[\s\S]*?Floppy144CataloguePage',
    'case F144_ACTION_BACK:[\s\S]*?Floppy144CabinetBackspace',
    'case F144_ACTION_BACK:[\s\S]*?Floppy144CatalogueCloseDocument',
    'case F144_ACTION_MOVE_UP:[\s\S]*?Floppy144TerminalMoveHistory',
    'case F144_ACTION_NAV_UP:[\s\S]*?Floppy144CatalogueMove'
))
{
    if($mainSource -notmatch $contract)
    {
        throw "Logical action behaviour contract is missing: $contract"
    }
}

Write-Host "STAGE 4 LOGICAL INPUT BOUNDARY AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4 LOGICAL INPUT REGRESSION ==="

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4-input"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-input"
}

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exePath = Join-Path $tempDir "stage4_input_tests.exe"
$includePlatform = "/I" + (Join-Path $root "include")
$includeGame = "/I" + $sourceDir

& cl.exe /nologo /TC /std:c11 /W4 /WX /utf-8 $testSource $inputSource $movementSource $includePlatform $includeGame "/Fe$exePath"

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4 logical-input regression build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "=== RUN STAGE 4 LOGICAL INPUT REGRESSION ==="
& $exePath

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4 logical-input regression failed with exit code $LASTEXITCODE."
}
