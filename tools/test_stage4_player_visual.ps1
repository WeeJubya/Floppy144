param()

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir
$SourceDir = Join-Path $Root "game\src"
$IncludeDir = Join-Path $Root "include"

Write-Host "=== STAGE 4D PLAYER CHARACTER / ANIMATION AUDIT ==="

$Main = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_main.c") -Raw
$Player = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_player_visual.c") -Raw
$PlayerH = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_player_visual.h") -Raw
$Site = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_site_2d.c") -Raw
$SiteH = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_site.h") -Raw
$SiteObjects = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_site_object.c") -Raw
$Timing = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_timing.c") -Raw
$Profile = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_profile.c") -Raw
$Persistence = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_persistence.c") -Raw

foreach($Required in @(
    'FLOPPY144_PLAYER_FACING_DOWN',
    'FLOPPY144_PLAYER_FACING_LEFT',
    'FLOPPY144_PLAYER_FACING_RIGHT',
    'FLOPPY144_PLAYER_FACING_UP',
    'FLOPPY144_PLAYER_WALK_FRAME_MS',
    'Floppy144PlayerVisualSetMovement',
    'Floppy144PlayerVisualAdvance',
    'Floppy144PlayerVisualDraw'
))
{
    if(
        $PlayerH -notmatch [regex]::Escape($Required) -and
        $Player -notmatch [regex]::Escape($Required)
    )
    {
        throw "Directional player contract is missing: $Required"
    }
}

foreach($Required in @(
    'Floppy144Site2DDrawForPlayerState',
    'global_player_visual',
    'events.presentation_elapsed_ms',
    'FLOPPY144_SITE_MOVE_STEP_X16'
))
{
    if($Main -notmatch [regex]::Escape($Required))
    {
        throw "Player presentation coordinator wiring is missing: $Required"
    }
}

foreach($Forbidden in @(
    'GetTickCount64',
    'SetTimer',
    'WM_TIMER'
))
{
    if(
        $Player -match [regex]::Escape($Forbidden) -or
        $Site -match [regex]::Escape($Forbidden)
    )
    {
        throw "Player animation introduced a direct Win32 timing dependency: $Forbidden"
    }
}

if($Timing -notmatch [regex]::Escape('presentation_elapsed_ms'))
{
    throw "Stage 4B timing layer is not supplying player presentation elapsed time."
}

foreach($Invariant in @(
    'FLOPPY144_SITE_PLAYER_VISUAL_WIDTH_X16       56',
    'FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16    32',
    'FLOPPY144_SITE_PLAYER_COLLISION_DEPTH_X16    32',
    'FLOPPY144_SITE_MOVE_STEP_X16              8'
))
{
    if($SiteH -notmatch [regex]::Escape($Invariant))
    {
        throw "Stage 3 player world-space invariant changed: $Invariant"
    }
}

foreach($InteractionInvariant in @(
    'FLOPPY144_SITE_DATA_INTERACTION_RANGE_X16',
    '(FLOPPY144_SITE_FIXED_ONE / 2)'
))
{
    if($SiteObjects -notmatch [regex]::Escape($InteractionInvariant))
    {
        throw "Site interaction-range invariant changed: $InteractionInvariant"
    }
}

foreach($Required in @(
    'FLOPPY144_OPERATOR_BODY_STYLE_B',
    'sprite_width * 7 / 8'
))
{
    if($Player -notmatch [regex]::Escape($Required))
    {
        throw "Profile body style is not mapped into the upgraded player renderer: $Required"
    }
}

foreach($Required in @(
    'profile->body_style',
    'Floppy144DiscoveryProfileBodyStyle'
))
{
    if(
        $Persistence -notmatch [regex]::Escape($Required) -and
        $Profile -notmatch [regex]::Escape($Required)
    )
    {
        throw "Persistent body-style contract is missing: $Required"
    }
}

foreach($Extension in @('.bmp','.png','.jpg','.jpeg','.gif'))
{
    if(
        $Player.ToLowerInvariant().Contains($Extension) -or
        $PlayerH.ToLowerInvariant().Contains($Extension)
    )
    {
        throw "Directional player renderer unexpectedly references external bitmap assets."
    }
}

Write-Host "STAGE 4D PLAYER CHARACTER / ANIMATION AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4D PLAYER VISUAL REGRESSION ==="

if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue))
{
    throw "MSVC compiler is not configured for the Stage 4D player regression."
}

$BuildDir =
    if($env:RUNNER_TEMP)
    {
        Join-Path $env:RUNNER_TEMP "floppy144-stage4d-player"
    }
    else
    {
        Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4d-player"
    }

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
$ExePath = Join-Path $BuildDir "stage4_player_visual_tests.exe"

$CompileArgs = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/O1",
    "/MT",
    "/utf-8",
    (Join-Path $ScriptDir "stage4_player_visual_tests.c"),
    (Join-Path $SourceDir "floppy144_player_visual.c"),
    (Join-Path $SourceDir "floppy144_draw.c"),
    ("/I" + $IncludeDir),
    ("/I" + $SourceDir),
    ("/Fe:" + $ExePath)
)

& cl.exe @CompileArgs

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4D player visual regression build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "=== RUN STAGE 4D PLAYER VISUAL REGRESSION ==="

& $ExePath

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4D player visual regression failed with exit code $LASTEXITCODE."
}

Write-Host "STAGE 4D PLAYER VISUAL REGRESSION: PASS"
