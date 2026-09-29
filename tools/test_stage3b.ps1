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
    & $ExePath

    if($LASTEXITCODE -ne 0) {
        throw "$Label failed with exit code $LASTEXITCODE."
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
    (Join-Path $SourceDir "floppy144_settings.c")
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
        $MainSource -notmatch 'FLOPPY144_SITE_ACTION_INSPECT' -or
        $MainSource -notmatch 'Floppy144SiteAvailableActions'
    ) {
        throw "Stage 3C coordinator does not gate blind Inspect keypresses through Site action availability."
    }

    if(
        $MainSource -notmatch 'FLOPPY144_TERMINAL_RESTORE_TIMER_ID' -or
        $MainSource -notmatch 'Floppy144TerminalAdvanceRestore' -or
        $MainSource -notmatch 'Floppy144TerminalRestoreInProgress'
    ) {
        throw "Collection restore progress is not wired into the Win32 terminal coordinator."
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
    $GameData = Get-Content -Raw -Path $GameDataPath | ConvertFrom-Json
    $CabinetSource = Get-Content -Raw -Path $CabinetPath
    $Site2DSource = Get-Content -Raw -Path $Site2DPath

    if($GameData.physical_items.Count -ne 500) {
        throw "Physical-item ledger count is $($GameData.physical_items.Count); expected 500."
    }

    foreach($Item in $GameData.physical_items) {
        if([string]::IsNullOrWhiteSpace($Item.description)) {
            throw "Physical item $($Item.id) has no player-facing description."
        }

        if($Item.description.Length -gt 100) {
            throw "Physical item $($Item.id) description exceeds the 100-character UI contract."
        }
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
            'UP/DOWN SELECT  ENTER VIEW  BACKSPACE SITE' -or
        $CabinetSource -notmatch
            'ITEM INSPECTED - NOTEBOOK UPDATED'
    ) {
        throw "Contents screen player-facing Enter/Notebook wording has regressed."
    }

    if(
        $CabinetSource -notmatch
            'uY = 124U;' -or
        $CabinetSource -notmatch
            'Marker coordinates are deliberately derived from the silhouette'
    ) {
        throw "Desk contents markers are no longer constrained to the parent silhouette."
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
        'Floppy144Site2DRotationIsDiagonal\(rotation\)'
    ) {
        throw "Diagonal desk details no longer inherit the desk orientation."
    }

    Write-Host "PHYSICAL ITEM PLAYER-FACING CONTRACT: PASS"
}

Invoke-Stage3BStep -Label "PHYSICAL ITEM PLAYER-FACING CONTRACT" -Action {
    Test-PhysicalItemPlayerFacingContract
}

$CabinetSources = @(
    (Join-Path $ScriptDir "stage3b_cabinet_tests.c"),
    (Join-Path $SourceDir "floppy144_cabinet.c"),
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
    (Join-Path $SourceDir "floppy144_site_2d.c"),
    (Join-Path $SourceDir "floppy144_site_directory.c"),
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
