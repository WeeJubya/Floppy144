param()

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir
$SourceDir = Join-Path $Root "game\src"
$IncludeDir = Join-Path $Root "include"

function Import-VcEnvironment {
    if(Get-Command cl.exe -ErrorAction SilentlyContinue) {
        return
    }

    $VsWhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if(-not (Test-Path $VsWhere)) {
        throw "MSVC compiler not found and vswhere.exe is unavailable."
    }

    $InstallPath = & $VsWhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath

    if(-not $InstallPath) {
        throw "Visual Studio C++ build tools were not found."
    }

    $VcVars = Join-Path $InstallPath "VC\Auxiliary\Build\vcvars64.bat"
    if(-not (Test-Path $VcVars)) {
        throw "vcvars64.bat was not found under $InstallPath."
    }

    $EnvironmentLines = & cmd.exe /s /c "`"$VcVars`" >nul && set"
    foreach($Line in $EnvironmentLines) {
        if($Line -match '^([^=]+)=(.*)$') {
            Set-Item -Path ("Env:" + $Matches[1]) -Value $Matches[2]
        }
    }

    if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        throw "MSVC environment initialisation completed but cl.exe is still unavailable."
    }
}

function Invoke-Stage3BRegression {
    param(
        [Parameter(Mandatory=$true)]
        [string]$Label,

        [Parameter(Mandatory=$true)]
        [string]$BuildFolder,

        [Parameter(Mandatory=$true)]
        [string]$ExecutableName,

        [Parameter(Mandatory=$true)]
        [string[]]$Sources
    )

    $BuildDir = Join-Path $Root ("build\" + $BuildFolder)
    $ExePath = Join-Path $BuildDir $ExecutableName

    foreach($Source in $Sources) {
        if(-not (Test-Path $Source)) {
            throw "Required $Label test source is missing: $Source"
        }
    }

    New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

    Write-Host ""
    Write-Host "=== BUILD $Label ==="

    Push-Location $BuildDir
    try {
        & cl.exe `
            /nologo `
            /std:c11 `
            /W4 `
            /O1 `
            /MT `
            /D_CRT_SECURE_NO_WARNINGS `
            "/I$SourceDir" `
            "/I$IncludeDir" `
            $Sources `
            "/Fe:$ExePath"

        if($LASTEXITCODE -ne 0) {
            throw "$Label build failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        Pop-Location
    }

    Write-Host ""
    Write-Host "=== RUN $Label ==="

    $TestOutput = @(
        & $ExePath 2>&1
    )
    $TestExitCode = $LASTEXITCODE

    foreach($Line in $TestOutput) {
        Write-Host $Line
    }

    if($TestExitCode -ne 0) {
        $AssertionFailures = @(
            $TestOutput |
                Where-Object {
                    $_ -match '^FAIL:'
                } |
                ForEach-Object {
                    $_.ToString().Trim()
                }
        )

        if($AssertionFailures.Count -gt 0) {
            throw (
                "$Label failed with exit code $TestExitCode. " +
                "Assertions: " +
                ($AssertionFailures -join " | ")
            )
        }

        throw "$Label failed with exit code $TestExitCode."
    }
}

$script:Stage3BFailures = @()

function Invoke-Stage3BStep {
    param(
        [Parameter(Mandatory=$true)]
        [string]$Label,

        [Parameter(Mandatory=$true)]
        [scriptblock]$Action
    )

    try {
        & $Action
    }
    catch {
        $Message = $_.Exception.Message
        $script:Stage3BFailures += "$Label :: $Message"
        Write-Host ""
        Write-Host "=== $Label FAILED; CONTINUING ==="
        Write-Host $Message
    }
}

function Test-PlayerFacingActLabels {
    Write-Host ""
    Write-Host "=== PLAYER-FACING ACT LABEL AUDIT ==="

    $SourceExtensions = @(".c", ".h", ".def", ".inc")
    $SourcePattern = '"[^"]*(?:ACT I|ACT II|ACT III|PROLOGUE|RECORDED SESSION:\s*ACT)[^"]*"'

    <#
        Generated runtime records may legitimately preserve internal progression
        metadata such as evidence "act": "Act I". Those values are not rendered
        to the player and must not be treated as presentation strings.

        Player-facing generated document prose is audited from the canonical JSON
        below, so excluding *.generated.* here does not create a blind spot.
    #>
    $SourceMatches = Get-ChildItem -Path $SourceDir -Recurse -File |
        Where-Object {
            $SourceExtensions -contains $_.Extension -and
            $_.Name -notmatch '\.generated\.'
        } |
        Select-String -Pattern $SourcePattern

    if($SourceMatches) {
        $FirstMatch = $SourceMatches | Select-Object -First 1
        throw "Player-facing Act/Prologue literal remains in source: $($FirstMatch.Path):$($FirstMatch.LineNumber)"
    }

    $GameDataPath = Join-Path $Root "data\floppy144_game_data.json"
    if(-not (Test-Path $GameDataPath)) {
        throw "Canonical game data is missing: $GameDataPath"
    }

    $GameData = Get-Content -Raw -Path $GameDataPath | ConvertFrom-Json

    $PlayerFacingPattern = '\bAct\s+(?:I|II|III)\b|\bPrologue\b'

    foreach($Document in $GameData.documents) {
        foreach($FieldName in @("title", "body")) {
            $Value = $Document.$FieldName
            if($null -ne $Value -and $Value -match $PlayerFacingPattern) {
                throw "Player-facing Act/Prologue wording remains in document $($Document.id) field $FieldName."
            }
        }
    }

    foreach($Collection in $GameData.collections) {
        foreach($FieldName in @("name", "notebook_entry")) {
            $Value = $Collection.$FieldName
            if($null -ne $Value -and $Value -match $PlayerFacingPattern) {
                throw "Player-facing Act/Prologue wording remains in collection $($Collection.id) field $FieldName."
            }
        }
    }

    foreach($Evidence in $GameData.evidence) {
        foreach($FieldName in @("statement", "notebook_entry")) {
            $Value = $Evidence.$FieldName
            if($null -ne $Value -and $Value -match $PlayerFacingPattern) {
                throw "Player-facing Act/Prologue wording remains in evidence $($Evidence.id) field $FieldName."
            }
        }
    }

    foreach($Ambient in $GameData.ambient_interactions) {
        foreach($FieldName in @("title", "text")) {
            $Value = $Ambient.$FieldName
            if($null -ne $Value -and $Value -match $PlayerFacingPattern) {
                throw "Player-facing Act/Prologue wording remains in ambient interaction $($Ambient.id) field $FieldName."
            }
        }
    }

    Write-Host "PLAYER-FACING ACT LABEL AUDIT: PASS"
}

Import-VcEnvironment
Invoke-Stage3BStep -Label "PLAYER-FACING ACT LABEL AUDIT" -Action {
    Test-PlayerFacingActLabels
}

Write-Host ""
Write-Host "=== FLOPPY//144 STAGE 3B REGRESSION ==="

$TerminalSources = @(
    (Join-Path $ScriptDir "stage3b_terminal_tests.c"),
    (Join-Path $SourceDir "floppy144_game_data.c"),
    (Join-Path $SourceDir "floppy144_trigger_engine.c"),
    (Join-Path $SourceDir "floppy144_interaction_engine.c"),
    (Join-Path $SourceDir "floppy144_run_state.c"),
    (Join-Path $SourceDir "floppy144_world.c"),
    (Join-Path $SourceDir "floppy144_terminal.c"),
    (Join-Path $SourceDir "floppy144_recovery.c"),
    (Join-Path $SourceDir "floppy144_catalogue.c"),
    (Join-Path $SourceDir "floppy144_document.c"),
    (Join-Path $SourceDir "floppy144_variation.c"),
    (Join-Path $SourceDir "floppy144_persistence.c"),
    (Join-Path $SourceDir "floppy144_profile.c"),
    (Join-Path $SourceDir "floppy144_settings.c"),
    (Join-Path $SourceDir "floppy144_effect.c"),
    (Join-Path $SourceDir "floppy144_draw.c"),
    (Join-Path $SourceDir "floppy144_site.c"),
    (Join-Path $SourceDir "floppy144_site_rooms.c"),
    (Join-Path $SourceDir "floppy144_site_object.c"),
    (Join-Path $SourceDir "floppy144_object_registry.c"),
    (Join-Path $SourceDir "floppy144_collection_registry.c")
)

Invoke-Stage3BStep -Label "STAGE 3B.1 TERMINAL REGRESSION" -Action {
    Invoke-Stage3BRegression `
        -Label "STAGE 3B.1 TERMINAL REGRESSION" `
        -BuildFolder "stage3b_terminal_tests" `
        -ExecutableName "stage3b_terminal_tests.exe" `
        -Sources $TerminalSources
}

$SiteInteractionSources = @(
    (Join-Path $ScriptDir "stage3b_site_interaction_tests.c"),
    (Join-Path $SourceDir "floppy144_game_data.c"),
    (Join-Path $SourceDir "floppy144_trigger_engine.c"),
    (Join-Path $SourceDir "floppy144_interaction_engine.c"),
    (Join-Path $SourceDir "floppy144_run_state.c"),
    (Join-Path $SourceDir "floppy144_world.c"),
    (Join-Path $SourceDir "floppy144_site.c"),
    (Join-Path $SourceDir "floppy144_site_rooms.c"),
    (Join-Path $SourceDir "floppy144_site_object.c"),
    (Join-Path $SourceDir "floppy144_object_registry.c"),
    (Join-Path $SourceDir "floppy144_collection_registry.c")
)

Invoke-Stage3BStep -Label "STAGE 3B.2 SITE INTERACTION REGRESSION" -Action {
    Invoke-Stage3BRegression `
        -Label "STAGE 3B.2 SITE INTERACTION REGRESSION" `
        -BuildFolder "stage3b_site_interaction_tests" `
        -ExecutableName "stage3b_site_interaction_tests.exe" `
        -Sources $SiteInteractionSources
}

$ReconstructionSources = @(
    (Join-Path $ScriptDir "stage3b_reconstruction_tests.c"),
    (Join-Path $SourceDir "floppy144_game_data.c"),
    (Join-Path $SourceDir "floppy144_trigger_engine.c"),
    (Join-Path $SourceDir "floppy144_interaction_engine.c"),
    (Join-Path $SourceDir "floppy144_run_state.c"),
    (Join-Path $SourceDir "floppy144_world.c"),
    (Join-Path $SourceDir "floppy144_site.c"),
    (Join-Path $SourceDir "floppy144_site_rooms.c"),
    (Join-Path $SourceDir "floppy144_site_object.c"),
    (Join-Path $SourceDir "floppy144_object_registry.c"),
    (Join-Path $SourceDir "floppy144_collection_registry.c"),
    (Join-Path $SourceDir "floppy144_persistence.c"),
    (Join-Path $SourceDir "floppy144_profile.c"),
    (Join-Path $SourceDir "floppy144_settings.c"),
    (Join-Path $SourceDir "floppy144_variation.c"),
    (Join-Path $SourceDir "floppy144_document.c"),
    (Join-Path $SourceDir "floppy144_effect.c")
)

Invoke-Stage3BStep -Label "STAGE 3B.3 RECONSTRUCTION REGRESSION" -Action {
    Invoke-Stage3BRegression `
        -Label "STAGE 3B.3 RECONSTRUCTION REGRESSION" `
        -BuildFolder "stage3b_reconstruction_tests" `
        -ExecutableName "stage3b_reconstruction_tests.exe" `
        -Sources $ReconstructionSources
}


function Test-Stage3B4CoordinatorWiring {
    Write-Host ""
    Write-Host "=== STAGE 3B.4 COORDINATOR WIRING AUDIT ==="

    $MainPath = Join-Path $SourceDir "floppy144_main.c"
    if(-not (Test-Path $MainPath)) {
        throw "Main coordinator source is missing: $MainPath"
    }

    $MainSource = Get-Content -Raw -Path $MainPath
    $MoveStart = $MainSource.IndexOf("static void Floppy144MovePlayer(")
    $MoveEnd = $MainSource.IndexOf("typedef enum Floppy144OfficeInteractionMode", $MoveStart)

    if($MoveStart -lt 0 -or $MoveEnd -le $MoveStart) {
        throw "Could not isolate Floppy144MovePlayer for the Stage 3B.4 audit."
    }

    $MoveSource = $MainSource.Substring($MoveStart, $MoveEnd - $MoveStart)

    if($MoveSource -notmatch 'Floppy144RunStateWouldExitSite') {
        throw "Floppy144MovePlayer does not query unlocked exterior Site exits."
    }

    if($MoveSource -notmatch 'Floppy144OpenMainMenu') {
        throw "Floppy144MovePlayer does not hand an exterior exit back to GDR session control."
    }

    if($MoveSource -notmatch 'Floppy144RunStateMovePlayerSite') {
        throw "Floppy144MovePlayer no longer routes ordinary movement through RunState."
    }

    Write-Host "STAGE 3B.4 COORDINATOR WIRING AUDIT: PASS"
}

Invoke-Stage3BStep -Label "STAGE 3B.4 COORDINATOR WIRING AUDIT" -Action {
    Test-Stage3B4CoordinatorWiring
}

function Test-MainMenuRecordFeedback {
    Write-Host ""
    Write-Host "=== MAIN MENU RECORD FEEDBACK AUDIT ==="

    $MainPath = Join-Path $SourceDir "floppy144_main.c"
    $RecoveryPath = Join-Path $SourceDir "floppy144_recovery.c"

    if(-not (Test-Path $MainPath) -or -not (Test-Path $RecoveryPath)) {
        throw "Main-menu record feedback sources are missing."
    }

    $MainSource = Get-Content -Raw -Path $MainPath
    $RecoverySource = Get-Content -Raw -Path $RecoveryPath

    if($MainSource -notmatch 'global_main_menu_notice\s*=\s*"CURRENT SESSION RECORDED"') {
        throw "Successful manual recording no longer produces player-visible confirmation."
    }

    if($MainSource -notmatch 'global_main_menu_notice\s*=\s*"CURRENT SESSION COULD NOT BE RECORDED"') {
        throw "Failed manual recording no longer produces player-visible feedback."
    }

    if($RecoverySource -notmatch 'menu_notice\s*!=\s*NULL' -or
       $RecoverySource -notmatch 'status_colour\s*=') {
        throw "Session Control no longer renders the transient record feedback."
    }

    Write-Host "MAIN MENU RECORD FEEDBACK AUDIT: PASS"
}

Invoke-Stage3BStep -Label "MAIN MENU RECORD FEEDBACK AUDIT" -Action {
    Test-MainMenuRecordFeedback
}

function Test-MainMenuReinstateFlow {
    Write-Host ""
    Write-Host "=== MAIN MENU REINSTATE FLOW AUDIT ==="

    $MainPath = Join-Path $SourceDir "floppy144_main.c"
    $RecoveryPath = Join-Path $SourceDir "floppy144_recovery.c"

    if(-not (Test-Path $MainPath) -or -not (Test-Path $RecoveryPath)) {
        throw "Main-menu reinstate sources are missing."
    }

    $MainSource = Get-Content -Raw -Path $MainPath
    $RecoverySource = Get-Content -Raw -Path $RecoveryPath

    $ReinstateStart =
        $MainSource.IndexOf(
            "case FLOPPY144_MAIN_MENU_REINSTATE_SESSION:"
        )

    $ReinstateEnd =
        $MainSource.IndexOf(
            "case FLOPPY144_MAIN_MENU_TERMINATE:",
            $ReinstateStart
        )

    if($ReinstateStart -lt 0 -or $ReinstateEnd -le $ReinstateStart) {
        throw "Could not isolate the main-menu reinstate action."
    }

    $ReinstateSource =
        $MainSource.Substring(
            $ReinstateStart,
            $ReinstateEnd - $ReinstateStart
        )

    if(
        $ReinstateSource -notmatch
            'global_reinstate_confirmation_pending\s*=\s*true' -or
        $ReinstateSource -notmatch
            'global_screen\s*=\s*FLOPPY144_SCREEN_MAIN_MENU'
    ) {
        throw "Successful reinstatement no longer pauses on Session Control for confirmation."
    }

    if(
        $ReinstateSource -notmatch
            'global_terminal\.suppress_next_character\s*=\s*false'
    ) {
        throw "Reinstated terminal will discard the first real command character."
    }

    if(
        $MainSource -notmatch 'case WM_KEYUP:' -or
        $MainSource -notmatch
            'global_reinstate_continue_on_keyup' -or
        $MainSource -notmatch
            'pEvent->physical_token == global_reinstate_continue_key' -or
        $MainSource -notmatch
            'f144Win32TranslateKeyEvent' -or
        $MainSource -notmatch
            'UpdateWindow\s*\(\s*window\s*\)'
    ) {
        throw "Reinstate confirmation no longer reveals one painted progress frame before entering gameplay."
    }

    if(
        $RecoverySource -notmatch
            '(?s)const Floppy144RunState \*display_state\s*=\s*active_session\s*\?\s*run_state\s*:\s*NULL\s*;' 
    ) {
        throw "Session Control can expose recorded-save reconstruction progress before reinstatement."
    }

    if(
        $RecoverySource -notmatch '"SESSION RESTORED"' -or
        $RecoverySource -notmatch '"PRESS ANY KEY TO CONTINUE"' -or
        $RecoverySource -notmatch 'if\(reinstate_confirmation\)'
    ) {
        throw "The reinstated-session confirmation box is missing."
    }

    Write-Host "MAIN MENU REINSTATE FLOW AUDIT: PASS"
}

Invoke-Stage3BStep -Label "MAIN MENU REINSTATE FLOW AUDIT" -Action {
    Test-MainMenuReinstateFlow
}

$DoorAccessSources = @(
    (Join-Path $ScriptDir "stage3b_door_access_tests.c"),
    (Join-Path $SourceDir "floppy144_game_data.c"),
    (Join-Path $SourceDir "floppy144_trigger_engine.c"),
    (Join-Path $SourceDir "floppy144_interaction_engine.c"),
    (Join-Path $SourceDir "floppy144_run_state.c"),
    (Join-Path $SourceDir "floppy144_world.c"),
    (Join-Path $SourceDir "floppy144_site.c"),
    (Join-Path $SourceDir "floppy144_site_rooms.c"),
    (Join-Path $SourceDir "floppy144_site_object.c"),
    (Join-Path $SourceDir "floppy144_object_registry.c"),
    (Join-Path $SourceDir "floppy144_collection_registry.c")
)

Invoke-Stage3BStep -Label "STAGE 3B.4 DOOR / ACCESS REGRESSION" -Action {
    Invoke-Stage3BRegression `
        -Label "STAGE 3B.4 DOOR / ACCESS REGRESSION" `
        -BuildFolder "stage3b_door_access_tests" `
        -ExecutableName "stage3b_door_access_tests.exe" `
        -Sources $DoorAccessSources
}

function Test-Stage3B5CoordinatorWiring {
    Write-Host ""
    Write-Host "=== STAGE 3B.5 CABINET COORDINATOR WIRING AUDIT ==="

    $MainPath = Join-Path $SourceDir "floppy144_main.c"
    $RunStateHeaderPath = Join-Path $SourceDir "floppy144_run_state.h"
    $PersistenceHeaderPath = Join-Path $SourceDir "floppy144_persistence.h"
    $PersistenceSourcePath = Join-Path $SourceDir "floppy144_persistence.c"
    $Site2DPath = Join-Path $SourceDir "floppy144_site_2d.c"
    $SiteIsoPath = Join-Path $SourceDir "floppy144_site_isometric.c"
    $SiteDirectoryPath = Join-Path $SourceDir "floppy144_site_directory.c"
    $CataloguePath = Join-Path $SourceDir "floppy144_catalogue.c"
    $NotebookViewPath = Join-Path $SourceDir "floppy144_notebook_view.c"
    $DrawingRuntimeHeaderPath = Join-Path $SourceDir "floppy144_drawing_runtime.h"
    $DrawingRuntimeSourcePath = Join-Path $SourceDir "floppy144_drawing_runtime.c"

    foreach($Path in @(
        $MainPath,
        $RunStateHeaderPath,
        $PersistenceHeaderPath,
        $PersistenceSourcePath,
        $Site2DPath,
        $SiteIsoPath,
        $SiteDirectoryPath,
        $CataloguePath,
        $NotebookViewPath,
        $DrawingRuntimeHeaderPath,
        $DrawingRuntimeSourcePath
    )) {
        if(-not (Test-Path $Path)) {
            throw "Stage 3B.5 wiring source is missing: $Path"
        }
    }

    $MainSource = Get-Content -Raw -Path $MainPath
    $RunStateHeader = Get-Content -Raw -Path $RunStateHeaderPath
    $PersistenceHeader = Get-Content -Raw -Path $PersistenceHeaderPath
    $PersistenceSource = Get-Content -Raw -Path $PersistenceSourcePath
    $Site2DSource = Get-Content -Raw -Path $Site2DPath
    $SiteIsoSource = Get-Content -Raw -Path $SiteIsoPath
    $SiteDirectorySource = Get-Content -Raw -Path $SiteDirectoryPath
    $CatalogueSource = Get-Content -Raw -Path $CataloguePath
    $NotebookViewSource = Get-Content -Raw -Path $NotebookViewPath
    $DrawingRuntimeHeader = Get-Content -Raw -Path $DrawingRuntimeHeaderPath
    $DrawingRuntimeSource = Get-Content -Raw -Path $DrawingRuntimeSourcePath

    foreach($Required in @(
        'floppy144_cabinet.h',
        'FLOPPY144_SCREEN_CABINET',
        'Floppy144CabinetOpenNearby',
        'Floppy144CabinetOpenParent',
        'Floppy144CabinetDraw',
        'Floppy144CabinetSubmitCode',
        'Floppy144CabinetInspectSelected',
        'Floppy144CabinetBackspace'
    )) {
        if($MainSource -notmatch [regex]::Escape($Required)) {
            throw "Stage 3B.5 main coordinator is missing: $Required"
        }
    }

    if($RunStateHeader -notmatch 'secure_cabinets_unlocked') {
        throw "Stage 3B.5 per-cabinet unlock state is not present in RunState."
    }

    if(
        $PersistenceHeader -notmatch 'FLOPPY144_SAVE_PAYLOAD_V2_SIZE' -or
        $PersistenceSource -notmatch 'secure_cabinets_unlocked'
    ) {
        throw "Stage 3B.5 per-cabinet unlock state is not included in the Stage 3C V2 persistence payload."
    }

    foreach($Renderer in @($Site2DSource, $SiteIsoSource)) {
        if($Renderer -notmatch 'Floppy144CabinetOpenNearby') {
            throw "Stage 3B.5 Site prompt does not advertise A=Access near secure cabinets."
        }
    }

    if(
        $MainSource -notmatch
            'case FLOPPY144_SCREEN_OFFICE:[\s\S]*?Floppy144Site2DDraw' -or
        $MainSource -match
            'Floppy144RunStateIsIsometric' -or
        $MainSource -match
            'Floppy144SiteIsometricDraw\('
    ) {
        throw "Stage 3 release must keep live Site exploration on the proven 2D renderer."
    }

    foreach($RequiredDirectoryIsoToken in @(
        'FM-23_ISOMETRIC_DIRECTORY',
        'Floppy144GameDataFactRecorded',
        'Floppy144SiteIsometricDirectoryDraw'
    )) {
        if($SiteDirectorySource -notmatch [regex]::Escape($RequiredDirectoryIsoToken)) {
            throw "FM-23 Site Directory upgrade contract is missing: $RequiredDirectoryIsoToken"
        }
    }

    foreach($RequiredDirectoryOverviewToken in @(
        'Floppy144IsometricConfigureDirectoryProjection',
        'Floppy144IsometricDrawPrismWireX16',
        'Floppy144IsometricDirectoryRectVisible',
        'Floppy144IsometricDirectoryDrawRoomLabels',
        'GDR SITE DIRECTORY // ISOMETRIC ACCOMMODATION PLAN',
        'g_nIsoHalfTileX=3',
        'g_nIsoHalfTileY=1',
        'g_nIsoHeightScale=2'
    )) {
        if($SiteIsoSource -notmatch [regex]::Escape($RequiredDirectoryOverviewToken)) {
            throw "FM-23 recovered accommodation-plan overview is missing: $RequiredDirectoryOverviewToken"
        }
    }

    if(
        $CatalogueSource -notmatch
            'Floppy144DrawScrollbar' -or
        $CatalogueSource -notmatch
            'catalogue->document_scroll_line' -or
        $CatalogueSource -notmatch
            'FLOPPY144_DOCUMENT_BODY_MAX_LINES\*\s*FLOPPY144_DOCUMENT_BODY_LINE_HEIGHT' -or
        $NotebookViewSource -notmatch
            'Floppy144DrawScrollbar' -or
        $NotebookViewSource -notmatch
            'FLOPPY144_NOTEBOOK_VISIBLE_LINES\*\s*FLOPPY144_NOTEBOOK_LINE_HEIGHT'
    ) {
        throw "Document Viewer or Notebook proportional scrollbar contract is missing."
    }

    if(
        $MainSource -notmatch 'FLOPPY144_SITE_ACTION_INSPECT' -or
        $MainSource -notmatch 'Floppy144SiteAvailableActions'
    ) {
        throw "Stage 3C coordinator does not gate blind Inspect keypresses through Site action availability."
    }

    if(
        $MainSource -notmatch 'Floppy144TimingAdvance' -or
        $MainSource -notmatch 'terminal_restore_elapsed_ms' -or
        $MainSource -notmatch 'Floppy144TerminalAdvanceRestore' -or
        $MainSource -notmatch 'Floppy144TerminalRestoreInProgress'
    ) {
        throw "Collection restore progress is not wired into the deterministic terminal coordinator."
    }

    if(
        $MainSource -match
        'UNAUTHORISED ACCESS - SECURE CABINET LOCKED'
    ) {
        throw "Stage 3C still gives a hidden Inspect response beside locked secure cabinets."
    }

    foreach($RequiredRendererToken in @(
        'Floppy144Site2DWallFixtureAttachment',
        'FLOPPY144_SITE_2D_WALL_LEFT',
        'FLOPPY144_SITE_2D_WALL_BOTTOM'
    )) {
        if($Site2DSource -notmatch [regex]::Escape($RequiredRendererToken)) {
            throw "Stage 3C 2D wall-fixture anchoring is missing: $RequiredRendererToken"
        }
    }

    <#
        Stage 3C camera clipping contract:

        Do NOT clip visual.x/visual.y/visual.width/visual.height before handing
        the object to the drawing recipe. That old behaviour changed the authored
        dimensions as an object crossed the viewport edge and made wall-hangings
        appear to shrink/grow.

        A disposable visible_probe may be clipped for visibility rejection, while
        authored drawing must use Floppy144DrawingRuntimeDrawClipped so primitive
        output is clipped without changing the object's geometry.
    #>
    if(
        $Site2DSource -match
        'Floppy144Site2DClipRect\s*\(\s*surface,\s*&visual\.x'
    ) {
        throw "Stage 3C wall fixtures are again clipping object geometry before authored drawing."
    }

    if(
        $SiteDirectorySource -match 'FLOPPY144_DIRECTORY_FOOTER_' -or
        $SiteDirectorySource -notmatch
            'FLOPPY144_DIRECTORY_MAP_SIZE\s+320U'
    ) {
        throw "Site Directory no longer owns the former footer area as part of its full-screen map."
    }

    if(
        $Site2DSource -notmatch 'Floppy144Site2DWallAttachmentToView' -or
        $Site2DSource -notmatch 'Floppy144DrawingRuntimeDrawClipped' -or
        $DrawingRuntimeHeader -notmatch 'Floppy144DrawingRuntimeDrawClipped' -or
        $DrawingRuntimeSource -notmatch 'Floppy144DrawingRuntimeDrawClipped'
    ) {
        throw "Stage 3C wall-hanging camera clipping contract is incomplete."
    }

    $AttachmentStart =
        $Site2DSource.IndexOf(
            "Floppy144Site2DWallAttachmentToView("
        )

    if($AttachmentStart -lt 0) {
        throw "Could not locate the wall-attachment view mapping."
    }

    $AttachmentSource =
        $Site2DSource.Substring(
            $AttachmentStart,
            [Math]::Min(
                900,
                $Site2DSource.Length - $AttachmentStart
            )
        )

    if(
        $AttachmentSource -notmatch 'return\s+attachment\s*;' -or
        $AttachmentSource -match
            'FLOPPY144_SITE_2D_WALL_LEFT\s*:\s*\r?\n\s*return\s+FLOPPY144_SITE_2D_WALL_TOP'
    ) {
        throw "Wall attachments are being rotated even though Site view coordinates are already player-facing."
    }

    $Z1 = $Site2DSource.IndexOf("Z1: room floor.")
    $Z2 = $Site2DSource.IndexOf("Z2: furniture.")
    $Z3 = $Site2DSource.IndexOf("Z3: wall-hangings")
    $Z4 = $Site2DSource.IndexOf("Z4: camera view.")

    if(
        $Z1 -lt 0 -or
        $Z2 -le $Z1 -or
        $Z3 -le $Z2 -or
        $Z4 -le $Z3
    ) {
        throw "Stage 3C 2D renderer no longer preserves Floor > Furniture > Wall-Hangings > Camera View ordering."
    }

    $WallFixtureStart =
        $Site2DSource.IndexOf("static void Floppy144Site2DDrawWallFixture(")
    $WallFixtureEnd =
        $Site2DSource.IndexOf("static void Floppy144Site2DDrawFurnitureDetails(", $WallFixtureStart)

    if($WallFixtureStart -lt 0 -or $WallFixtureEnd -le $WallFixtureStart) {
        throw "Could not isolate the Stage 3C wall-fixture renderer."
    }

    $WallFixtureSource =
        $Site2DSource.Substring(
            $WallFixtureStart,
            $WallFixtureEnd - $WallFixtureStart
        )

    if(
        $WallFixtureSource -notmatch 'visible_probe' -or
        $WallFixtureSource -notmatch 'Floppy144DrawingRuntimeDrawClipped'
    ) {
        throw "Stage 3C wall fixtures can again be resized by camera clipping."
    }

    Write-Host "STAGE 3B.5 CABINET COORDINATOR WIRING AUDIT: PASS"
}

Invoke-Stage3BStep -Label "STAGE 3B.5 CABINET COORDINATOR WIRING AUDIT" -Action {
    Test-Stage3B5CoordinatorWiring
}

function Test-PhysicalItemPlayerFacingContract {
    Write-Host ""
    Write-Host "=== PHYSICAL ITEM PLAYER-FACING CONTRACT ==="

    $GameDataPath = Join-Path $Root "data\floppy144_game_data.json"
    $CabinetPath = Join-Path $SourceDir "floppy144_cabinet.c"
    $Site2DPath = Join-Path $SourceDir "floppy144_site_2d.c"
    $BuildSitePath = Join-Path $Root "tools\build_site.ps1"
    $GameData = Get-Content -Raw -Path $GameDataPath | ConvertFrom-Json
    $CabinetSource = Get-Content -Raw -Path $CabinetPath
    $Site2DSource = Get-Content -Raw -Path $Site2DPath
    $BuildSiteSource = Get-Content -Raw -Path $BuildSitePath

    $ServerRoomSource =
        $GameData.site_layout_source.rooms |
        Where-Object { $_.PSObject.Properties['id'] -and $_.id -eq 'SERVER_ROOM' }

    $ServerTerminalSource =
        $ServerRoomSource.geometry |
        Where-Object { $_.PSObject.Properties['id'] -and $_.id -eq 'SERVER_ROOM_TERMINAL_DESK' }

    $ServerCableRiserSource =
        $ServerRoomSource.geometry |
        Where-Object { $_.PSObject.Properties['id'] -and $_.id -eq 'SERVER_ROOM_CABLE_RISER' }

    if(
        $null -eq $ServerTerminalSource -or
        $ServerTerminalSource.variant -ne 'IT_TERMINAL'
    ) {
        throw "Canonical Server Room terminal is no longer classified as IT_TERMINAL."
    }

    if(
        $null -eq $ServerCableRiserSource -or
        $ServerCableRiserSource.variant -ne 'CABLE_RISER' -or
        $ServerCableRiserSource.x -ne 33 -or
        $ServerCableRiserSource.y -ne 74 -or
        $ServerCableRiserSource.width -ne 1 -or
        $ServerCableRiserSource.height -ne 8
    ) {
        throw "Canonical Server Room cable-riser geometry is missing or has drifted."
    }

    if($GameData.physical_items.Count -lt 633) {
        throw "Physical-item ledger count is $($GameData.physical_items.Count); Stage 3C baseline is 633."
    }

    if([int]$GameData.counts.physical_items -ne $GameData.physical_items.Count) {
        throw "Physical-item metadata count is $($GameData.counts.physical_items); ledger contains $($GameData.physical_items.Count)."
    }

    foreach($Item in $GameData.physical_items) {
        if([string]::IsNullOrWhiteSpace($Item.description)) {
            throw "Physical item $($Item.id) has no player-facing description."
        }

        if($Item.description.Length -gt 150) {
            throw "Physical item $($Item.id) description exceeds the 150-character three-line UI contract."
        }
    }

    $RecordsRoom =
        $GameData.rooms |
        Where-Object {
            $_.id -eq 'RECORDS_OFFICE'
        }

    $RecordsFloors = @($RecordsRoom.floor_geometry)

    if(
        $RecordsFloors.Count -ne 2 -or
        -not (
            $RecordsFloors |
            Where-Object {
                $_.type -eq 'FLOOR_C' -and
                $_.x -eq 66 -and
                $_.y -eq 1 -and
                $_.width -eq 33 -and
                $_.height -eq 32
            }
        ) -or
        -not (
            $RecordsFloors |
            Where-Object {
                $_.type -eq 'FLOOR_C' -and
                $_.x -eq 56 -and
                $_.y -eq 1 -and
                $_.width -eq 10 -and
                $_.height -eq 45
            }
        )
    ) {
        throw "Records Office floor metadata has regressed from the corrected L-shaped footprint."
    }

    if(
        $BuildSiteSource -notmatch
            '\[string\]\$InputFile\s*=\s*"game\\src\\site_layout\.generated\.jsonc"'
    ) {
        throw "build_site.ps1 no longer defaults to the runtime-authoritative generated Site layout."
    }

    foreach($Furniture in $GameData.furniture) {
        $Children =
            @(
                $GameData.physical_items |
                Where-Object {
                    $_.parent_id -eq $Furniture.id
                }
            )

        if($Children.Count -lt 1) {
            throw "Furniture $($Furniture.id) has no physical-item context."
        }

        if(
            $Furniture.element_type -eq 'CHAIR' -and
            ($Children.Count -lt 1 -or $Children.Count -gt 2)
        ) {
            throw "Chair $($Furniture.id) has $($Children.Count) physical items; chairs must have one or two."
        }
    }

    if(
        $CabinetSource -notmatch 'Floppy144CabinetDrawChairBody' -or
        $CabinetSource -notmatch 'Floppy144CabinetTypeIs\(pCabinet,"CHAIR"\)' -or
        $CabinetSource -notmatch 'Floppy144CabinetVisibleContentLimit'
    ) {
        throw "Chair inspection has lost its dedicated chair silhouette or two-item runtime cap."
    }

    if(
        $CabinetSource -notmatch 'EMPTY STORAGE' -or
        $CabinetSource -notmatch 'strcmp\(pParent->pszB, "BOOKCASE"\)' -or
        $CabinetSource -notmatch 'strcmp\(pParent->pszB, "NONSECURE_CABINET"\)' -or
        $CabinetSource -notmatch 'strcmp\(pParent->pszB, "SHELVING_FULL"\)'
    ) {
        throw "Cabinet Contents no longer preserves access to empty non-secure storage."
    }

    $RestoredPhysicalNames = @{
        'P-065' = 'Legacy support sticker'
        'P-082' = 'Meeting notes card'
        'P-089' = 'Evacuation roll-call pencil'
        'P-125' = 'Stationery requisition pad'
        'P-126' = 'Calculator tape'
        'P-127' = 'Closure milestone board card'
    }

    foreach($Pair in $RestoredPhysicalNames.GetEnumerator()) {
        $Item = $GameData.physical_items | Where-Object { $_.id -eq $Pair.Key }
        if($null -eq $Item -or $Item.name -ne $Pair.Value) {
            throw "Physical item $($Pair.Key) no longer preserves its canonical identity."
        }
    }

    $ExpectedNameplates = @(
        'Priya Patel desk nameplate',
        'Daniel Price desk nameplate',
        'Andrew Collins desk nameplate',
        'Claire Hughes desk nameplate',
        'Imran Shah desk nameplate',
        'Martin Webb desk nameplate',
        'Helen Cartwright desk nameplate',
        'Rachel Morgan desk nameplate'
    )

    foreach($Nameplate in $ExpectedNameplates) {
        if(-not ($GameData.physical_items | Where-Object { $_.name -eq $Nameplate })) {
            throw "Canonical staff nameplate is missing: $Nameplate"
        }
    }

    foreach($Interaction in $GameData.interactions) {
        if(
            $Interaction.physical_source -match '^P-' -and
            $Interaction.type -in @('Inspect','Synthesis','Capability') -and
            [string]::IsNullOrWhiteSpace($Interaction.notebook)
        ) {
            throw "Physical inspection $($Interaction.id) can complete without a Notebook observation."
        }
    }

    $Fm13ContractRecord =
        $GameData.documents |
        Where-Object {
            $_.collection_id -eq 'FM-13' -and
            $_.record_index -eq 1
        }

    if(
        $null -eq $Fm13ContractRecord -or
        $Fm13ContractRecord.body -notmatch 'Daniel Mercer' -or
        $Fm13ContractRecord.body -notmatch 'Alderwick Fire & Safety Ltd'
    ) {
        throw "FM-13-RS-0076 no longer resolves to the named external contractor record."
    }

    if(
        $CabinetSource -notmatch 'Floppy144CabinetDrawWrappedText' -or
        $CabinetSource -notmatch '54U,\s*3U,\s*20U'
    ) {
        throw "Physical-item detail view no longer supports the three-line flavour-text contract."
    }

    if($CabinetSource -match '"ID: %s"' -or $CabinetSource -match '"ROLE:') {
        throw "Cabinet Interior still exposes internal physical-item metadata."
    }

    if(
        $CabinetSource -match
            '(?s)Floppy144DrawText\s*\([^;]*"RECOVERED PHYSICAL ITEM"'
    ) {
        throw "Contents detail still renders the backend-style Recovered Physical Item heading."
    }

    if(
        $CabinetSource -notmatch
            'UP/DOWN SELECT   ENTER VIEW   BACKSPACE BACK' -or
        $CabinetSource -notmatch
            'ITEM INSPECTED - NOTEBOOK UPDATED'
    ) {
        throw "Contents screen player-facing Enter/Notebook wording has regressed."
    }

    if(
        $CabinetSource -notmatch
            'Floppy144CabinetContentMarkerRegion' -or
        $CabinetSource -notmatch
            'uGridWidth=uColumns\*uMarkerWidth' -or
        $CabinetSource -notmatch
            'uGridHeight=uRows\*uMarkerHeight' -or
        $CabinetSource -notmatch
            'uBaseX=uRegionX\+\(uRegionWidth>uGridWidth\?\(uRegionWidth-uGridWidth\)/2U:0U\)' -or
        $CabinetSource -notmatch
            'uBaseY=uRegionY\+\(uRegionHeight>uGridHeight\?\(uRegionHeight-uGridHeight\)/2U:0U\)'
    ) {
        throw "Contents markers are no longer centred as an adaptive grid inside the parent silhouette."
    }

    if(
        $CabinetSource -notmatch
            'Door between %s and %s' -or
        $CabinetSource -notmatch
            'FLOPPY144_DATA_CONNECTION'
    ) {
        throw "Door Contents titles are no longer derived from player-facing connection names."
    }

    if(
        $CabinetSource -notmatch
            'Floppy144CabinetTypeIs\(pCabinet,"SHELVING_FULL"\)' -or
        $CabinetSource -notmatch
            'Floppy144CabinetContentMarkerRegion' -or
        $CabinetSource -notmatch
            'Floppy144CabinetPhysicalItemRevealControlled' -or
        $CabinetSource -notmatch
            'Floppy144CabinetContentShuffleKey' -or
        $CabinetSource -notmatch
            'auShelfY\[4\]=\{130U,184U,238U,292U\}' -or
        $CabinetSource -notmatch
            'uY=\s*auShelfY\[uShelf\]-\s*uShelfMarkerHeight'
    ) {
        throw "Shelving no longer has its own open-shelf presentation, seeded ordering, or PI markers resting on authored shelf surfaces."
    }

    if(
        $Site2DSource -notmatch
            'const int32_t mount_pixels' -or
        $Site2DSource -notmatch
            'visual_rect->y -= mount_pixels' -or
        $Site2DSource -notmatch
            'visual_rect->x -= mount_pixels'
    ) {
        throw "Wall hangings are no longer visually mounted onto the wall plane."
    }

    if(
        $Site2DSource -notmatch
            'Floppy144Site2DRotationIsDiagonal\(rotation\)' -or
        $Site2DSource -notmatch
            'Floppy144Site2DDiagonalAxes' -or
        $Site2DSource -notmatch
            'authored_width16' -or
        $Site2DSource -notmatch
            'Floppy144Site2DFillQuad' -or
        $Site2DSource -notmatch
            'FLOPPY144_SITE_2D_PIXELS_PER_UNIT \*' -or
        $Site2DSource -notmatch
            'Floppy144Site2DNormalisedRotation'
    ) {
        throw "Diagonal furniture no longer preserves authored size, aspect ratio and signed orientation."
    }

    $SiteCompilerSource =
        Get-Content -Raw -Path (Join-Path $Root "tools\site_compiler.c")

    if(
        $SiteCompilerSource -notmatch 'rotation_authored' -or
        $SiteCompilerSource -notmatch
            'placement->rotation_authored'
    ) {
        throw "Site compiler no longer preserves authored signed rotations."
    }

    Write-Host "PHYSICAL ITEM PLAYER-FACING CONTRACT: PASS"
}

Invoke-Stage3BStep -Label "PHYSICAL ITEM PLAYER-FACING CONTRACT" -Action {
    Test-PhysicalItemPlayerFacingContract
}

$CabinetSources = @(
    (Join-Path $ScriptDir "stage3b_cabinet_tests.c"),
    (Join-Path $SourceDir "floppy144_cabinet.c"),
    (Join-Path $SourceDir "floppy144_cabinet_25d.c"),
    (Join-Path $SourceDir "floppy144_noticeboard.c"),
    (Join-Path $SourceDir "floppy144_takeaway.c"),
    (Join-Path $SourceDir "floppy144_crossword.c"),
    (Join-Path $SourceDir "floppy144_paperback.c"),
    (Join-Path $SourceDir "floppy144_variation.c"),
    (Join-Path $Root "src\f144_startup_config.c"),
    (Join-Path $SourceDir "floppy144_game_data.c"),
    (Join-Path $SourceDir "floppy144_trigger_engine.c"),
    (Join-Path $SourceDir "floppy144_interaction_engine.c"),
    (Join-Path $SourceDir "floppy144_run_state.c"),
    (Join-Path $SourceDir "floppy144_world.c"),
    (Join-Path $SourceDir "floppy144_draw.c"),
    (Join-Path $SourceDir "floppy144_drawing_runtime.c"),
    (Join-Path $SourceDir "floppy144_site.c"),
    (Join-Path $SourceDir "floppy144_site_rooms.c"),
    (Join-Path $SourceDir "floppy144_site_object.c"),
    (Join-Path $SourceDir "floppy144_site_view.c"),
    (Join-Path $SourceDir "floppy144_site_2d_camera.c"),
    (Join-Path $SourceDir "floppy144_site_2d.c"),
    (Join-Path $SourceDir "floppy144_grey_door.c"),
    (Join-Path $SourceDir "floppy144_player_visual.c"),
    (Join-Path $SourceDir "floppy144_site_directory.c"),
    (Join-Path $SourceDir "floppy144_site_isometric.c"),
    (Join-Path $SourceDir "floppy144_object_registry.c"),
    (Join-Path $SourceDir "floppy144_collection_registry.c"),
    (Join-Path $SourceDir "floppy144_persistence.c"),
    (Join-Path $SourceDir "floppy144_profile.c"),
    (Join-Path $SourceDir "floppy144_settings.c")
)

Invoke-Stage3BStep -Label "STAGE 3B.5 PARENT CONTENTS / SECURE CABINET REGRESSION" -Action {
    Invoke-Stage3BRegression `
        -Label "STAGE 3B.5 PARENT CONTENTS / SECURE CABINET REGRESSION" `
        -BuildFolder "stage3b_cabinet_tests" `
        -ExecutableName "stage3b_cabinet_tests.exe" `
        -Sources $CabinetSources
}
Write-Host ""
if($script:Stage3BFailures.Count -gt 0) {
    Write-Host "=== STAGE 3B REGRESSION SUMMARY: FAIL ==="
    foreach($Failure in $script:Stage3BFailures) {
        Write-Host (" - " + $Failure)
    }
    throw "Stage 3B regression completed with $($script:Stage3BFailures.Count) failing step(s)."
}

Write-Host "=== STAGE 3B REGRESSION SUMMARY: PASS ==="
