$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$premakePath = Join-Path $root "premake5.lua"
$workflowPath = Join-Path $root ".github\workflows\stage3c-ci.yml"

Write-Host "=== STAGE 4 BUILD ARCHITECTURE AUDIT ==="

$premake = Get-Content -LiteralPath $premakePath -Raw
$workflow = Get-Content -LiteralPath $workflowPath -Raw

# Return the text owned by one Premake project block.
function Get-PremakeProjectBlock
{
    param
    (
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $marker = 'project("' + $Name + '")'
    $start = $premake.LastIndexOf($marker)

    if($start -lt 0)
    {
        throw "Premake project is missing: $Name"
    }

    $next = $premake.IndexOf('project("', $start + $marker.Length)

    if($next -lt 0)
    {
        return $premake.Substring($start)
    }

    return $premake.Substring(
        $start,
        $next - $start
    )
}

# Reject one forbidden native dependency from the portable Core project.
function Assert-CorePatternAbsent
{
    param
    (
        [Parameter(Mandatory = $true)]
        [string]$Pattern,

        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    if($coreBlock -match $Pattern)
    {
        throw "Floppy144Core unexpectedly owns $Description."
    }
}

$coreBlock = Get-PremakeProjectBlock -Name "Floppy144Core"
$platformBlock = Get-PremakeProjectBlock -Name "Floppy144PlatformWin32"
$appBlock = Get-PremakeProjectBlock -Name "Floppy144"

if($premake -match 'project\("F144 Runtime"\)')
{
    throw "Legacy F144 Runtime remains a separate build target instead of belonging to PlatformWin32."
}

foreach($required in @(
    './game/src/**.c',
    './src/f144_startup_config.c',
    './src/f144_platform.c',
    './game/src/floppy144_main.c',
    './game/src/floppy144_persistence.c',
    './game/src/floppy144_storage.c'
))
{
    if($premake -notmatch [regex]::Escape($required))
    {
        throw "Build ownership declaration is missing source: $required"
    }
}

foreach($excluded in @(
    './game/src/floppy144_main.c',
    './game/src/floppy144_persistence.c',
    './game/src/floppy144_storage.c'
))
{
    if($coreBlock -notmatch [regex]::Escape($excluded))
    {
        throw "Floppy144Core does not explicitly exclude application/platform-debt source: $excluded"
    }
}

Assert-CorePatternAbsent -Pattern 'defines\s*\(\s*\{\s*"BUILD_WINDOWS"' -Description 'the BUILD_WINDOWS compile definition'
Assert-CorePatternAbsent -Pattern '"user32"' -Description 'user32'
Assert-CorePatternAbsent -Pattern '"gdi32"' -Description 'gdi32'
Assert-CorePatternAbsent -Pattern '"shell32"' -Description 'shell32'
Assert-CorePatternAbsent -Pattern 'f144_win32_' -Description 'Win32 implementation sources'
Assert-CorePatternAbsent -Pattern 'SharedLib' -Description 'a runtime DLL boundary'

foreach($required in @(
    './src/f144_win32_platform.c',
    './src/f144_win32_input.c',
    './src/f144_win32_storage.c',
    './src/f144_win32_timing.c',
    './src/f144_win32_lifecycle.c',
    './src/f144_win32_audio.c',
    './src/f144_win32_single_instance.c',
    './src/f144_win32_startup_config.c',
    './src/f144_runtime.c',
    './src/f144_win32_runtime.c'
))
{
    if($platformBlock -notmatch [regex]::Escape($required))
    {
        throw "Floppy144PlatformWin32 is missing native/runtime source: $required"
    }
}

if($platformBlock -notmatch 'BUILD_WINDOWS')
{
    throw "Floppy144PlatformWin32 does not own BUILD_WINDOWS."
}

foreach($required in @(
    '"Floppy144Core"',
    '"Floppy144PlatformWin32"',
    '"user32"',
    '"gdi32"',
    '"shell32"'
))
{
    if($appBlock -notmatch [regex]::Escape($required))
    {
        throw "Windows application link is missing: $required"
    }
}

if($appBlock -match '"winmm"')
{
    throw "WinMM was linked even though S4B-04 currently has no native audio backend."
}

$coreSources = @(
    Get-ChildItem -Path (Join-Path $root "game\src") -Filter "*.c" -File |
        Where-Object {
            $_.Name -notin @(
                "floppy144_main.c",
                "floppy144_persistence.c",
                "floppy144_storage.c"
            )
        }
)

$coreSources +=
    Get-Item -LiteralPath (Join-Path $root "src\f144_startup_config.c")

$coreSources +=
    Get-Item -LiteralPath (Join-Path $root "src\f144_platform.c")

$coreHeaders = @(
    Get-ChildItem -Path (Join-Path $root "game\src") -Filter "*.h" -File |
        Where-Object {
            $_.Name -notin @(
                "floppy144_persistence.h",
                "floppy144_storage.h"
            )
        }
)

$coreHeaders +=
    Get-Item -LiteralPath (Join-Path $root "include\f144_startup_config.h")

$coreHeaders +=
    Get-Item -LiteralPath (Join-Path $root "include\f144_platform.h")

$coreFiles =
    @($coreSources) +
    @($coreHeaders)

$coreForbiddenPatterns = @(
    '(?i)#\s*include\s*[<"]windows\.h[>"]',
    '(?i)#\s*include\s*"f144_win32_[^"]+"',
    '(?i)#\s*include\s*"f144_runtime\.h"',
    '\bHWND\b',
    '\bHDC\b',
    '\bWPARAM\b',
    '\bLPARAM\b',
    '\bHANDLE\b',
    '\bVK_[A-Z0-9_]+\b',
    '\bGetTickCount(?:64)?\s*\(',
    '\bSetTimer\s*\(',
    '\bKillTimer\s*\(',
    '\bCreateMutexA\s*\(',
    '\bSHGetFolderPathA\s*\(',
    '\bMoveFileExA\s*\('
)

foreach($file in $coreFiles)
{
    $sourceText =
        Get-Content -LiteralPath $file.FullName -Raw

    foreach($pattern in $coreForbiddenPatterns)
    {
        if($sourceText -match $pattern)
        {
            throw "Portable Core file contains a Win32/runtime dependency: $($file.Name) :: $pattern"
        }
    }
}

$persistenceSource = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_persistence.c") -Raw

foreach($debtMarker in @(
    '#include <windows.h>',
    'MoveFileExA',
    'DeleteFileA',
    'GetFileAttributesA'
))
{
    if($persistenceSource -notmatch [regex]::Escape($debtMarker))
    {
        throw "Documented persistence portability debt changed unexpectedly: $debtMarker"
    }
}

$generationIndex = $workflow.IndexOf('Regenerate canonical runtime data')
$premakeIndex = $workflow.IndexOf('Generate Visual Studio 2022 project')

if(
    $generationIndex -lt 0 -or
    $premakeIndex -lt 0 -or
    $generationIndex -gt $premakeIndex
)
{
    throw "Canonical data generation must remain ahead of Visual Studio project generation/build."
}

$resourceFiles = @(
    Get-ChildItem -Path $root -Recurse -File |
        Where-Object {
            $_.Extension -in @(
                ".rc",
                ".ico",
                ".manifest",
                ".res"
            )
        }
)

if($resourceFiles.Count -eq 0)
{
    Write-Host "Resource/icon audit: no Windows resource or icon files are currently present."
}
else
{
    Write-Host "Resource/icon audit: found $($resourceFiles.Count) resource file(s)."
}

Write-Host "Portable Core audit: $($coreSources.Count) C translation unit(s) and $($coreHeaders.Count) header(s), no Win32/runtime dependency found."
Write-Host "Persistence debt audit: mixed codec/Win32 file I/O remains quarantined in the application target."
Write-Host "STAGE 4 BUILD ARCHITECTURE AUDIT: PASS"
