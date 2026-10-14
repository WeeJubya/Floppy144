# BF29/BF31/BF32/BF34: source integration checks; executable cases live
# in stage4_persistence_paths_tests.c and run via test_stage4_persistence.ps1.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = Split-Path -Parent $PSScriptRoot
$catalogue = Get-Content (Join-Path $root 'game/src/floppy144_catalogue.c') -Raw
$main = Get-Content (Join-Path $root 'game/src/floppy144_main.c') -Raw
$run = Get-Content (Join-Path $root 'game/src/floppy144_run_state.h') -Raw
$recovery = Get-Content (Join-Path $root 'game/src/floppy144_recovery.c') -Raw
$codec = Get-Content (Join-Path $root 'game/src/floppy144_persistence.c') -Raw
$tests = Get-Content (Join-Path $root 'tools/stage4_persistence_paths_tests.c') -Raw

$expect = @(
    @($catalogue,'Floppy144CatalogueTitleHasRepeatedWord'),
    @($catalogue,'Floppy144CatalogueDocumentSizeBytes'),
    @($catalogue,'Floppy144RunStateRecordOpened'),
    @($catalogue,'SIZE: %u BYTES'),
    @($main,'global_pending_difficulty=FLOPPY144_DIFFICULTY_EASY'),
    @($main,'FLOPPY144_SCREEN_DIFFICULTY'),
    @($main,'global_catalogue.run_state = &global_run_state'),
    @($main,'FLOPPY144_DIFFICULTY_REALLY_SUPER_EASY'),
    @($run,'REALLY SUPER EASY (I MEAN REALLY STUPIDLY EASY)'),
    @($recovery,'Floppy144DifficultyDraw'),
    @($codec,'FLOPPY144_SAVE_PAYLOAD_V6_SIZE'),
    @($codec,'FLOPPY144_SAVE_VERSION_V5'),
    @($tests,'TestBF29BF31BF32BF34(root)')
)
foreach($check in $expect)
{
    if($check[0] -notmatch [regex]::Escape($check[1]))
    {
        throw "Document/difficulty source integration missing: $($check[1])"
    }
}
Write-Host 'BF29/BF31/BF32/BF34 SOURCE INTEGRATION: PASS'
Write-Host 'Run tools/test_stage4_persistence.ps1 for executable generation, save and allocation cases.'
