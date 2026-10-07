param()

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir
$SourceDir = Join-Path $Root "game\src"
$IncludeDir = Join-Path $Root "include"

Write-Host "=== STAGE 4D PRESENTATION CONSISTENCY AUDIT ==="

$Sources = @{
    Recovery = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_recovery.c")
    Profile = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_profile_view.c")
    Settings = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_settings_view.c")
    Credits = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_credits_view.c")
    Completion = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_completion_view.c")
    Catalogue = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_catalogue.c")
    Notebook = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_notebook_view.c")
    Cabinet = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_cabinet.c")
    Terminal = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_terminal.c")
    Site = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_site_2d.c")
    Main = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_main.c")
    Draw = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_draw.c")
    DrawHeader = Get-Content -Raw -LiteralPath (Join-Path $SourceDir "floppy144_draw.h")
}

foreach($Name in @('Recovery','Profile','Settings','Credits','Completion'))
{
    if($Sources[$Name] -notmatch 'FLOPPY144_UI_FORMAL_FOOTER_Y')
    {
        throw "$Name no longer uses the common formal-screen footer baseline."
    }
}

$Forbidden = @(
    'UP DOWN SELECT',
    'PGUP PGDN PAGE',
    'BACKSPACE RETURN',
    'BACKSPACE  BACK',
    'ENTER  SAVE',
    'ESC  CANCEL',
    'LEFT/RIGHT  BODY STYLE',
    'ESC RECOVERY',
    'Q: EXIT'
)

foreach($Token in $Forbidden)
{
    foreach($Name in @('Recovery','Profile','Settings','Credits','Completion','Catalogue','Notebook','Cabinet','Terminal','Site'))
    {
        if($Sources[$Name].Contains($Token))
        {
            throw "Legacy presentation wording remains in $($Name): $Token"
        }
    }
}

$RequiredBySource = @{
    Recovery = @('UP/DOWN SELECT   ENTER CONFIRM')
    Profile = @(
        'ENTER SAVE   ESC CANCEL   BACKSPACE DELETE',
        'LEFT/RIGHT BODY STYLE   ENTER EDIT NAME   BACKSPACE BACK'
    )
    Settings = @('UP/DOWN SELECT   LEFT/RIGHT CHANGE   ENTER CHANGE/OPEN   BACKSPACE BACK')
    Credits = @('BACKSPACE BACK')
    Completion = @(
        'UP/DOWN SELECT   ENTER CONFIRM',
        'ENTER/BACKSPACE BACK TO SUMMARY',
        'Floppy144DrawScrollbar'
    )
    Catalogue = @(
        'UP/DOWN SELECT',
        'PGUP/PGDN PAGE',
        'BACKSPACE BACK',
        'Floppy144DrawScrollbar'
    )
    Notebook = @(
        'N/BACKSPACE BACK',
        'Floppy144DrawScrollbar'
    )
    Cabinet = @(
        '0-9 CODE   ENTER SUBMIT   BACKSPACE BACK',
        'UP/DOWN SELECT   ENTER VIEW   BACKSPACE BACK',
        'BACKSPACE BACK TO CONTENTS',
        'Floppy144DrawScrollbar'
    )
    Terminal = @(
        'Q: RETURN',
        '"Q RETURN"'
    )
    Site = @(
        'ESC SESSION CONTROL',
        'Floppy144DrawTextWidth(menu_prompt,1U)'
    )
    Draw = @('void Floppy144DrawScrollbar(')
    DrawHeader = @(
        '#define FLOPPY144_UI_FORMAL_FOOTER_Y 346U',
        'void Floppy144DrawScrollbar('
    )
}

foreach($Entry in $RequiredBySource.GetEnumerator())
{
    foreach($Token in $Entry.Value)
    {
        if(-not $Sources[$Entry.Key].Contains($Token))
        {
            throw "Presentation contract missing from $($Entry.Key): $Token"
        }
    }
}

foreach($Token in @(
    'case FLOPPY144_SCREEN_NOTEBOOK:',
    'case F144_ACTION_NOTEBOOK:',
    'case F144_ACTION_BACK:',
    'case FLOPPY144_SCREEN_COMPLETION:',
    'case F144_ACTION_CONFIRM:',
    'case FLOPPY144_SCREEN_SITE_DIRECTORY'
))
{
    if(-not $Sources.Main.Contains($Token))
    {
        throw "Input/navigation contract missing from coordinator: $Token"
    }
}

$FormalFooters = @(
    'UP/DOWN SELECT   ENTER CONFIRM',
    'ENTER SAVE   ESC CANCEL   BACKSPACE DELETE',
    'LEFT/RIGHT BODY STYLE   ENTER EDIT NAME   BACKSPACE BACK',
    'UP/DOWN SELECT   LEFT/RIGHT CHANGE   ENTER CHANGE/OPEN   BACKSPACE BACK',
    'BACKSPACE BACK',
    'UP/DOWN SCROLL   PGUP/PGDN PAGE   ENTER/BACKSPACE BACK TO SUMMARY'
)

foreach($Footer in $FormalFooters)
{
    $Width = $Footer.Length * 6 - 1

    if($Width -gt 620)
    {
        throw "Formal footer exceeds the safe 620px width: $Footer"
    }
}

Write-Host "STAGE 4D PRESENTATION CONSISTENCY AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4D PRESENTATION CONSISTENCY REGRESSION ==="

if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue))
{
    throw "MSVC compiler is not configured for the presentation-consistency regression."
}

$BuildDir =
    if($env:RUNNER_TEMP)
    {
        Join-Path $env:RUNNER_TEMP "floppy144-stage4d-presentation-consistency"
    }
    else
    {
        Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4d-presentation-consistency"
    }

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
$ExePath = Join-Path $BuildDir "stage4_presentation_consistency_tests.exe"

$CompileArgs = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/O1",
    "/MT",
    "/utf-8",
    (Join-Path $ScriptDir "stage4_presentation_consistency_tests.c"),
    (Join-Path $SourceDir "floppy144_draw.c"),
    ("/I" + $IncludeDir),
    ("/I" + $SourceDir),
    ("/Fe:" + $ExePath)
)

& cl.exe @CompileArgs

if($LASTEXITCODE -ne 0)
{
    throw "Presentation-consistency regression build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "=== RUN STAGE 4D PRESENTATION CONSISTENCY REGRESSION ==="

& $ExePath

if($LASTEXITCODE -ne 0)
{
    throw "Presentation-consistency regression failed with exit code $LASTEXITCODE."
}

Write-Host "STAGE 4D PRESENTATION CONSISTENCY REGRESSION: PASS"
