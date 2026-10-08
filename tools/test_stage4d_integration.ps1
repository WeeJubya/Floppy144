$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot

function Read-Stage4DSource([string]$relative)
{
    return Get-Content -LiteralPath (Join-Path $root $relative) -Raw
}

$player = Read-Stage4DSource "game\src\floppy144_player_visual.c"
$playerH = Read-Stage4DSource "game\src\floppy144_player_visual.h"
$camera = Read-Stage4DSource "game\src\floppy144_site_2d_camera.c"
$cameraH = Read-Stage4DSource "game\src\floppy144_site_2d_camera.h"
$site = Read-Stage4DSource "game\src\floppy144_site_2d.c"
$siteH = Read-Stage4DSource "game\src\floppy144_site.h"
$siteObject = Read-Stage4DSource "game\src\floppy144_site_object.c"
$cabinet = Read-Stage4DSource "game\src\floppy144_cabinet.c"
$cabinet25d = Read-Stage4DSource "game\src\floppy144_cabinet_25d.c"
$cabinet25dH = Read-Stage4DSource "game\src\floppy144_cabinet_25d.h"
$terminal = Read-Stage4DSource "game\src\floppy144_terminal.c"
$draw = Read-Stage4DSource "game\src\floppy144_draw.c"
$drawH = Read-Stage4DSource "game\src\floppy144_draw.h"
$profile = Read-Stage4DSource "game\src\floppy144_profile.h"
$timing = Read-Stage4DSource "game\src\floppy144_timing.c"
$timingH = Read-Stage4DSource "game\src\floppy144_timing.h"
$main = Read-Stage4DSource "game\src\floppy144_main.c"
$workflow = Read-Stage4DSource ".github\workflows\stage3c-ci.yml"
$helpTests = Read-Stage4DSource "tools\stage4_help_presentation_tests.c"
$cameraTests = Read-Stage4DSource "tools\stage4_office_camera_tests.c"
$playerTests = Read-Stage4DSource "tools\stage4_player_visual_tests.c"
$inspectionTests = Read-Stage4DSource "tools\stage4_inspection_25d_tests.c"
$consistencyTests = Read-Stage4DSource "tools\stage4_presentation_consistency_tests.c"
$stage4cGate = Read-Stage4DSource "tools\test_stage4c_integration.ps1"

Write-Host "=== STAGE 4D INTEGRATION / VISUAL-ACCEPTANCE AUDIT ==="

$portableRenderSources = @{
    "floppy144_player_visual.c" = $player
    "floppy144_player_visual.h" = $playerH
    "floppy144_site_2d_camera.c" = $camera
    "floppy144_site_2d_camera.h" = $cameraH
    "floppy144_site_2d.c" = $site
    "floppy144_cabinet_25d.c" = $cabinet25d
    "floppy144_cabinet_25d.h" = $cabinet25dH
    "floppy144_terminal.c" = $terminal
    "floppy144_draw.c" = $draw
    "floppy144_draw.h" = $drawH
}

$win32LeakPattern = '(?i)#\s*include\s*[<"]windows\.h[>"]|\bHWND\b|\bHDC\b|\bGetTickCount\w*\b|\bQueryPerformanceCounter\b|\bSetTimer\b|\bKillTimer\b|\bBitBlt\b|\bStretchDIBits\b|\bCreateWindow\w*\b|\bGetAsyncKeyState\b|\bWM_TIMER\b'

foreach($entry in $portableRenderSources.GetEnumerator())
{
    if($entry.Value -match $win32LeakPattern)
    {
        throw "Stage 4D portable renderer acquired a direct Win32 dependency: $($entry.Key)"
    }
}

$stage4dImplementation = ($portableRenderSources.Values -join [Environment]::NewLine)

if($stage4dImplementation -match '(?im)\b(TODO|FIXME|HACK)\b')
{
    throw "Stage 4D rendering implementation contains a TODO/FIXME/HACK marker."
}

foreach($entry in @{
    "player" = $player
    "camera" = $camera
    "inspection" = $cabinet25d
}.GetEnumerator())
{
    if($entry.Value -match '(?i)\b(malloc|calloc|realloc|free)\s*\(')
    {
        throw "Stage 4D $($entry.Key) renderer unexpectedly performs heap allocation."
    }
}

if(
    $cameraH -notmatch '#define\s+FLOPPY144_SITE_2D_PIXELS_PER_UNIT\s+12\b' -or
    $camera -notmatch 'Floppy144Site2DBuildCameraAtScale' -or
    $camera -notmatch 'FLOPPY144_SITE_2D_PIXELS_PER_UNIT'
)
{
    throw "The selected 12 px/Site-unit camera scale is no longer the single production camera contract."
}

if(
    $camera -match 'run_state->\w+\s*=(?!=)' -or
    $camera -match 'run_state->\w+\s*(\+\+|--)' -or
    $camera -match 'Floppy144RunState(Reconstruct|Move|BitSet|Apply)'
)
{
    throw "Camera presentation code is mutating gameplay/world state."
}

foreach($required in @(
    'FLOPPY144_SITE_PLAYER_VISUAL_WIDTH_X16',
    'FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16',
    'FLOPPY144_SITE_PLAYER_COLLISION_DEPTH_X16',
    'FLOPPY144_SITE_MOVE_STEP_X16'
))
{
    if($siteH -notmatch [regex]::Escape($required))
    {
        throw "Stage 3 player world-space invariant is missing: $required"
    }
}

if(
    $player -match 'FLOPPY144_SITE_PLAYER_COLLISION' -or
    $playerH -match 'floppy144_run_state\.h' -or
    $player -match 'Floppy144RunState'
)
{
    throw "Player presentation acquired collision or persistent RunState ownership."
}

foreach($required in @(
    'FLOPPY144_OPERATOR_BODY_STYLE_A',
    'FLOPPY144_OPERATOR_BODY_STYLE_B'
))
{
    if($profile -notmatch [regex]::Escape($required))
    {
        throw "Supported cosmetic body style disappeared: $required"
    }
}

if(
    $main -notmatch 'events\.presentation_elapsed_ms' -or
    $main -notmatch 'Floppy144PlayerVisualAdvance' -or
    $timing -notmatch 'presentation_elapsed_ms' -or
    $playerH -notmatch 'FLOPPY144_PLAYER_WALK_FRAME_MS\s+160U'
)
{
    throw "Player animation is no longer driven through the Stage 4B timing abstraction."
}

if(
    $cabinet25d -match 'Floppy144CabinetMoveSelection' -or
    $cabinet25d -match 'Floppy144CabinetInspectSelected' -or
    $cabinet25d -match 'Floppy144RunStateBitSet' -or
    $cabinet25d -match 'Floppy144RunStateReconstructRoom'
)
{
    throw "2.5D Inspection rendering acquired item-selection or gameplay mutation responsibility."
}

foreach($hardCodedId in @(
    '"MAIN_OFFICE_DESK_01"',
    '"IT_SUPPORT_BOOKCASE"',
    '"SECRETARY_OFFICE_DESK"',
    '"IT_SUPPORT_PATCH_PANEL"',
    '"STAFF_ROOM_WORKTOP"'
))
{
    if($cabinet25d.Contains($hardCodedId))
    {
        throw "Production 2.5D renderer contains a test/object-specific parent ID: $hardCodedId"
    }
}

foreach($required in @(
    'Floppy144CabinetEnhancedPresentationUnlocked',
    'FLOPPY144_ROOM_MAIN_OFFICE',
    'Floppy144CabinetDrawContainerBody',
    'Floppy144Cabinet25DDraw',
    'Floppy144CabinetVisibleContentCount'
))
{
    if(
        $cabinet -notmatch [regex]::Escape($required)
    )
    {
        throw "Inspection pre/post-unlock integration contract is missing: $required"
    }
}

if(
    $site -notmatch '(?s)void\s+Floppy144Site2DDrawForBodyStyle\s*\(.*?Floppy144Site2DDrawForPlayerState\s*\('
)
{
    throw "Legacy body-style Site draw entry point is no longer a thin compatibility wrapper."
}

if(
    $draw -notmatch 'void\s+Floppy144DrawScrollbar\s*\(' -or
    $consistencyTests -notmatch 'TestScrollbarContract'
)
{
    throw "Shared Stage 4D scrollbar grammar is missing or unprotected."
}

foreach($required in @(
    'PAGE 1 OF 3',
    'PAGE 2 OF 3',
    'PAGE 3 OF 3',
    'PageFitsLayout'
))
{
    if($helpTests -notmatch [regex]::Escape($required))
    {
        throw "Help visual acceptance no longer covers all pages/layout: $required"
    }
}

foreach($required in @(
    'FLOPPY144_ROOM_COUNT',
    'CheckSelectedScaleTransform'
))
{
    if($cameraTests -notmatch [regex]::Escape($required))
    {
        throw "Camera visual acceptance no longer covers every room/selected transform: $required"
    }
}

foreach($required in @(
    'TestDirectionAndIdle',
    'TestQuickDirectionTransitions',
    'TestAnimation',
    'TestBodyStylesAndRoundHead',
    'FLOPPY144_OPERATOR_BODY_STYLE_B'
))
{
    if($playerTests -notmatch [regex]::Escape($required))
    {
        throw "Player visual acceptance coverage is missing: $required"
    }
}

foreach($required in @(
    'TestEveryCanonicalParentVariant',
    'TestDimensionsOrientationAndWallFixtures',
    'TestContentMarkerStates',
    'TestUnknownFallback'
))
{
    if($inspectionTests -notmatch [regex]::Escape($required))
    {
        throw "Inspection visual acceptance coverage is missing: $required"
    }
}

if(
    $stage4cGate -notmatch 'BUILD STAGE 4C FRESH-PROFILE INTEGRATION SMOKE' -or
    $stage4cGate -notmatch 'RUN STAGE 4C FRESH-PROFILE INTEGRATION SMOKE'
)
{
    throw "Fresh-profile cross-surface smoke coverage is no longer present in the inherited Stage 4C integration gate."
}

foreach($requiredStep in @(
    'Stage 2 regression',
    'Stage 3A regression',
    'Stage 3B regression',
    'Stage 4 platform boundary',
    'Stage 4 logical input',
    'Stage 4 persistence paths',
    'Stage 4 audio contract',
    'Stage 4 timing and lifecycle',
    'Stage 4 single-instance protection',
    'Stage 4 developer configuration',
    'Stage 4C operator profile screen',
    'Stage 4C operator name entry',
    'Stage 4C terminal authentication',
    'Stage 4C persistent settings',
    'Stage 4C credits and attribution',
    'Stage 4C restored-session presentation',
    'Stage 4C bespoke completion presentation',
    'Stage 4D widened Help presentation',
    'Stage 4D office camera scale',
    'Stage 4D directional player character',
    'Stage 4D pseudo-isometric Inspection containers',
    'Stage 4D presentation consistency',
    'Stage 4D integration sign-off audit',
    'Stage 4 build architecture',
    'Stage 4B integration sign-off audit',
    'Stage 4C integration sign-off audit'
))
{
    if($workflow -notmatch [regex]::Escape($requiredStep))
    {
        throw "Stage 4D sign-off workflow is missing required gate: $requiredStep"
    }
}

# The Win32 coordinator remains an inherited Stage 4B architecture exception.
# Stage 4D is accepted only if native presentation stays outside its renderers.
if($main -notmatch '#include\s*<windows\.h>')
{
    throw "Expected inherited Win32 coordinator boundary changed; re-audit architecture before sign-off."
}

Write-Host "STAGE 4D INTEGRATION / VISUAL-ACCEPTANCE AUDIT: PASS"
Write-Host "STAGE 4D INTEGRATION GATE: PASS"
