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
Test-PlayerFacingActLabels

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

Invoke-Stage3BRegression `
    -Label "STAGE 3B.1 TERMINAL REGRESSION" `
    -BuildFolder "stage3b_terminal_tests" `
    -ExecutableName "stage3b_terminal_tests.exe" `
    -Sources $TerminalSources

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

Invoke-Stage3BRegression `
    -Label "STAGE 3B.2 SITE INTERACTION REGRESSION" `
    -BuildFolder "stage3b_site_interaction_tests" `
    -ExecutableName "stage3b_site_interaction_tests.exe" `
    -Sources $SiteInteractionSources

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

Invoke-Stage3BRegression `
    -Label "STAGE 3B.3 RECONSTRUCTION REGRESSION" `
    -BuildFolder "stage3b_reconstruction_tests" `
    -ExecutableName "stage3b_reconstruction_tests.exe" `
    -Sources $ReconstructionSources


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

Test-Stage3B4CoordinatorWiring

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

Invoke-Stage3BRegression `
    -Label "STAGE 3B.4 DOOR / ACCESS REGRESSION" `
    -BuildFolder "stage3b_door_access_tests" `
    -ExecutableName "stage3b_door_access_tests.exe" `
    -Sources $DoorAccessSources

function Test-Stage3B5CoordinatorWiring {
    Write-Host ""
    Write-Host "=== STAGE 3B.5 CABINET COORDINATOR WIRING AUDIT ==="

    $MainPath = Join-Path $SourceDir "floppy144_main.c"
    $RunStateHeaderPath = Join-Path $SourceDir "floppy144_run_state.h"
    $PersistenceHeaderPath = Join-Path $SourceDir "floppy144_persistence.h"
    $Site2DPath = Join-Path $SourceDir "floppy144_site_2d.c"
    $SiteIsoPath = Join-Path $SourceDir "floppy144_site_isometric.c"

    foreach($Path in @($MainPath, $RunStateHeaderPath, $PersistenceHeaderPath, $Site2DPath, $SiteIsoPath)) {
        if(-not (Test-Path $Path)) {
            throw "Stage 3B.5 wiring source is missing: $Path"
        }
    }

    $MainSource = Get-Content -Raw -Path $MainPath
    $RunStateHeader = Get-Content -Raw -Path $RunStateHeaderPath
    $PersistenceHeader = Get-Content -Raw -Path $PersistenceHeaderPath
    $Site2DSource = Get-Content -Raw -Path $Site2DPath
    $SiteIsoSource = Get-Content -Raw -Path $SiteIsoPath

    foreach($Required in @(
        'floppy144_cabinet.h',
        'FLOPPY144_SCREEN_CABINET',
        'Floppy144CabinetOpenNearby',
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

    if($PersistenceHeader -notmatch 'secure_cabinets_unlocked') {
        throw "Stage 3B.5 per-cabinet unlock state is not included in persistence payload sizing."
    }

    foreach($Renderer in @($Site2DSource, $SiteIsoSource)) {
        if($Renderer -notmatch 'Floppy144CabinetOpenNearby') {
            throw "Stage 3B.5 Site prompt does not advertise A=Access near secure cabinets."
        }
    }

    Write-Host "STAGE 3B.5 CABINET COORDINATOR WIRING AUDIT: PASS"
}

Test-Stage3B5CoordinatorWiring

$CabinetSources = @(
    (Join-Path $ScriptDir "stage3b_cabinet_tests.c"),
    (Join-Path $SourceDir "floppy144_cabinet.c"),
    (Join-Path $SourceDir "floppy144_game_data.c"),
    (Join-Path $SourceDir "floppy144_trigger_engine.c"),
    (Join-Path $SourceDir "floppy144_interaction_engine.c"),
    (Join-Path $SourceDir "floppy144_run_state.c"),
    (Join-Path $SourceDir "floppy144_world.c"),
    (Join-Path $SourceDir "floppy144_draw.c"),
    (Join-Path $SourceDir "floppy144_site.c"),
    (Join-Path $SourceDir "floppy144_site_rooms.c"),
    (Join-Path $SourceDir "floppy144_site_object.c"),
    (Join-Path $SourceDir "floppy144_object_registry.c"),
    (Join-Path $SourceDir "floppy144_collection_registry.c"),
    (Join-Path $SourceDir "floppy144_persistence.c"),
    (Join-Path $SourceDir "floppy144_profile.c"),
    (Join-Path $SourceDir "floppy144_settings.c")
)

Invoke-Stage3BRegression `
    -Label "STAGE 3B.5 SECURE CABINET / INTERIOR REGRESSION" `
    -BuildFolder "stage3b_cabinet_tests" `
    -ExecutableName "stage3b_cabinet_tests.exe" `
    -Sources $CabinetSources
