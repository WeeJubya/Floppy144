$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root =
    Split-Path -Parent $PSScriptRoot

$workflowPath =
    Join-Path $root ".github\workflows\stage3c-ci.yml"

$platformHeaderPath =
    Join-Path $root "include\f144_platform.h"

$mainPath =
    Join-Path $root "game\src\floppy144_main.c"

$premakePath =
    Join-Path $root "premake5.lua"

$completionPath =
    Join-Path $root "docs\stage4\stage4b_completion.md"

Write-Host "=== STAGE 4B INTEGRATION SIGN-OFF AUDIT ==="

$workflow =
    Get-Content -LiteralPath $workflowPath -Raw

$platformHeader =
    Get-Content -LiteralPath $platformHeaderPath -Raw

$mainSource =
    Get-Content -LiteralPath $mainPath -Raw

$premake =
    Get-Content -LiteralPath $premakePath -Raw

# Require every Stage 3 and Stage 4B validation gate in the integration workflow.
$requiredWorkflowCommands = @(
    '.\tools\test_stage2.ps1',
    '.\tools\test_stage3a.ps1',
    '.\tools\test_stage3b.ps1',
    '.\tools\test_stage4_platform.ps1',
    '.\tools\test_stage4_input.ps1',
    '.\tools\test_stage4_persistence.ps1',
    '.\tools\test_stage4_audio.ps1',
    '.\tools\test_stage4_timing_lifecycle.ps1',
    '.\tools\test_stage4_single_instance.ps1',
    '.\tools\test_stage4_config.ps1',
    '.\tools\test_stage4_build_architecture.ps1'
)

foreach($command in $requiredWorkflowCommands)
{
    if($workflow -notmatch [regex]::Escape($command))
    {
        throw "Stage 4B integration workflow is missing validation command: $command"
    }
}

# Require the complete game-facing platform contract assembled during Stage 4B.
$requiredPlatformContract = @(
    'F144ActionEvent',
    'F144TextInputEvent',
    'F144LifecycleEvent',
    'F144PersistenceFile',
    'F144MusicCueId',
    'F144SfxCueId',
    'Floppy144Surface',
    'f144PlatformFramebuffer',
    'f144PlatformPresent',
    'f144PlatformPersistencePath',
    'f144PlatformLegacyPersistencePath',
    'f144PlatformAudioInit',
    'f144PlatformAudioShutdown',
    'f144PlatformPlayMusic',
    'f144PlatformPlaySfx',
    'f144PlatformMonotonicMs',
    'f144PlatformQuit'
)

foreach($symbol in $requiredPlatformContract)
{
    if($platformHeader -notmatch [regex]::Escape($symbol))
    {
        throw "Stage 4B platform contract is missing: $symbol"
    }
}

# The platform-neutral interface itself must never acquire native Windows types.
$platformForbidden = @(
    '(?i)#\s*include\s*[<"]windows\.h[>"]',
    '\bHWND\b',
    '\bHDC\b',
    '\bHANDLE\b',
    '\bWPARAM\b',
    '\bLPARAM\b',
    '\bVK_[A-Z0-9_]+\b',
    '\bWM_[A-Z0-9_]+\b'
)

foreach($pattern in $platformForbidden)
{
    if($platformHeader -match $pattern)
    {
        throw "Win32 detail leaked into f144_platform.h: $pattern"
    }
}

# Verify WinMain/coordinator uses semantic Stage 4B boundaries for migrated work.
$requiredCoordinatorWiring = @(
    'f144Win32TranslateKeyEvent',
    'Floppy144HandleActionEvent',
    'f144Win32TranslateTextEvent',
    'Floppy144HandleTextInput',
    'Floppy144StorageResolve',
    'f144PlatformAudioInit',
    'Floppy144TimingAdvance',
    'f144Win32TranslateLifecycleEvent',
    'f144Win32SingleInstanceAcquire',
    'f144Win32StartupConfigFromCommandLine'
)

foreach($symbol in $requiredCoordinatorWiring)
{
    if($mainSource -notmatch [regex]::Escape($symbol))
    {
        throw "Stage 4B coordinator wiring is missing semantic boundary: $symbol"
    }
}

# Reject the old direct implementations for responsibilities already migrated.
$forbiddenCoordinatorPatterns = @(
    '\bVK_[A-Z0-9_]+\b',
    '\bGetTickCount(?:64)?\s*\(',
    '\bSetTimer\s*\(',
    '\bKillTimer\s*\(',
    '\bGetFileAttributesA\s*\(',
    '\bSHGetFolderPathA\s*\(',
    '\bCreateMutexA\s*\(',
    '\bMoveFileExA\s*\(',
    '\bPlaySound[A-Z]*\s*\(',
    '\bwaveOut[A-Za-z0-9_]*\s*\(',
    '\bmidiOut[A-Za-z0-9_]*\s*\(',
    'Floppy144CommandLineHasSwitch',
    '"-debug"'
)

foreach($pattern in $forbiddenCoordinatorPatterns)
{
    if($mainSource -match $pattern)
    {
        throw "Legacy/direct platform path remains in game coordinator: $pattern"
    }
}

# Confirm the build now expresses one portable Core and one Win32 implementation.
foreach($projectName in @(
    'project("Floppy144Core")',
    'project("Floppy144PlatformWin32")',
    'project("Floppy144")'
))
{
    if($premake -notmatch [regex]::Escape($projectName))
    {
        throw "Stage 4B build project is missing: $projectName"
    }
}

if($premake -match 'project\("F144 Runtime"\)')
{
    throw "Legacy F144 Runtime project unexpectedly returned."
}

if(-not (Test-Path -LiteralPath $completionPath))
{
    throw "Stage 4B completion record is missing."
}

Write-Host "Stage 3 + Stage 4B validation manifest: complete."
Write-Host "F144 platform contract audit: complete."
Write-Host "Coordinator old/new path audit: complete."
Write-Host "Core/Win32 build ownership audit: complete."
Write-Host "STAGE 4B INTEGRATION SIGN-OFF AUDIT: PASS"
