$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot

$gameFacingHeaders = @(
    "include\f144_platform.h",
    "game\src\floppy144_draw.h",
    "game\src\floppy144_cabinet.h",
    "game\src\floppy144_catalogue.h",
    "game\src\floppy144_notebook_view.h",
    "game\src\floppy144_recovery.h",
    "game\src\floppy144_site_2d.h",
    "game\src\floppy144_site_directory.h",
    "game\src\floppy144_site_isometric.h",
    "game\src\floppy144_terminal.h"
)

$renderSources = @(
    "game\src\floppy144_cabinet.c",
    "game\src\floppy144_catalogue.c",
    "game\src\floppy144_notebook_view.c",
    "game\src\floppy144_recovery.c",
    "game\src\floppy144_site_2d.c",
    "game\src\floppy144_site_directory.c",
    "game\src\floppy144_site_isometric.c",
    "game\src\floppy144_terminal.c"
)

$forbiddenPatterns = @(
    '(?i)#\s*include\s*[<"]windows\.h[>"]',
    '#\s*include\s*"f144_runtime\.h"',
    '\bHWND\b',
    '\bHINSTANCE\b',
    '\bHDC\b',
    '\bWPARAM\b',
    '\bLPARAM\b',
    '\bHANDLE\b',
    '\bVK_[A-Z0-9_]+\b'
)

foreach($relative in $gameFacingHeaders)
{
    $path = Join-Path $root $relative
    $text = Get-Content -LiteralPath $path -Raw

    foreach($pattern in $forbiddenPatterns)
    {
        if($text -match $pattern)
        {
            throw "Stage 4 platform boundary leaked Win32/runtime detail into $relative ($pattern)."
        }
    }
}

foreach($relative in $renderSources)
{
    $path = Join-Path $root $relative
    $text = Get-Content -LiteralPath $path -Raw

    if($text -match '\bF144Runtime\b')
    {
        throw "Stage 4 render-surface seam still depends on F144Runtime in $relative."
    }
}

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4-platform"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-platform"
}

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$sourcePath = Join-Path $tempDir "platform_header_compile.c"
$objPath = Join-Path $tempDir "platform_header_compile.obj"

@'
#include "f144_platform.h"
#include "floppy144_draw.h"
#include "floppy144_cabinet.h"
#include "floppy144_catalogue.h"
#include "floppy144_notebook_view.h"
#include "floppy144_recovery.h"
#include "floppy144_site_2d.h"
#include "floppy144_site_directory.h"
#include "floppy144_site_isometric.h"
#include "floppy144_terminal.h"

static Floppy144Surface *TestFramebuffer(F144Platform *platform)
{
    return &platform->surface;
}

static void TestPresent(F144Platform *platform)
{
    (void)platform;
}

static bool TestPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    char *path,
    uint32_t path_capacity
)
{
    (void)platform;
    (void)file;

    if(path == NULL || path_capacity < 2U)
    {
        return false;
    }

    path[0] = 'x';
    path[1] = '\0';
    return true;
}

static bool TestLegacyPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    uint32_t candidate,
    char *path,
    uint32_t path_capacity
)
{
    (void)candidate;

    return TestPersistencePath(
        platform,
        file,
        path,
        path_capacity
    );
}

int Floppy144Stage4PlatformHeaderCompileTest(void)
{
    F144PlatformApi api =
    {
        TestFramebuffer,
        TestPresent,
        TestPersistencePath,
        TestLegacyPersistencePath
    };
    F144Platform platform = {0};

    platform.api = &api;

    if(f144PlatformFramebuffer(&platform) != &platform.surface)
    {
        return 1;
    }

    {
        char path[F144_PLATFORM_PATH_CAPACITY];

        if(
            !f144PlatformPersistencePath(
                &platform,
                F144_PERSISTENCE_MANUAL_SAVE,
                path,
                (uint32_t)sizeof(path)
            ) ||
            path[0] != 'x'
        )
        {
            return 2;
        }

        if(
            !f144PlatformLegacyPersistencePath(
                &platform,
                F144_PERSISTENCE_PROFILE,
                0U,
                path,
                (uint32_t)sizeof(path)
            ) ||
            path[0] != 'x'
        )
        {
            return 3;
        }
    }

    f144PlatformPresent(&platform);
    return 0;
}
'@ | Set-Content -LiteralPath $sourcePath -Encoding UTF8

$includePlatform = "/I" + (Join-Path $root "include")
$includeGame = "/I" + (Join-Path $root "game\src")
$outputObject = "/Fo" + $objPath

& cl.exe /nologo /TC /std:c11 /W4 /WX /c $sourcePath $includePlatform $includeGame $outputObject
if($LASTEXITCODE -ne 0)
{
    throw "Stage 4 platform-neutral header compile failed with exit code $LASTEXITCODE."
}

Write-Host "STAGE 4 PLATFORM BOUNDARY: PASS"
