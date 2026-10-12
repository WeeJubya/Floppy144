# Independent Stage 4D-and-later source regression sweep.
# All tests run even if one fails; this surfaces the complete fault list
# rather than letting the sequential release gate mask later failures.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = Split-Path -Parent $PSScriptRoot
$tests = @(
    @{ Label='Stage 4D Help presentation'; Path='test_stage4_help_presentation.ps1' },
    @{ Label='Stage 4D office camera'; Path='test_stage4_office_camera.ps1' },
    @{ Label='Stage 4D player visual'; Path='test_stage4_player_visual.ps1' },
    @{ Label='Stage 4D inspection 2.5D'; Path='test_stage4_inspection_25d.ps1' },
    @{ Label='Stage 4D presentation consistency'; Path='test_stage4_presentation_consistency.ps1' },
    @{ Label='Stage 4D integration'; Path='test_stage4d_integration.ps1' },
    @{ Label='Stage 4 architecture'; Path='test_stage4_build_architecture.ps1' },
    @{ Label='Stage 4B integration'; Path='test_stage4b_integration.ps1' },
    @{ Label='Stage 4C integration'; Path='test_stage4c_integration.ps1' },
    @{ Label='Stage 4E integration'; Path='test_stage4e_integration.ps1' },
    @{ Label='Stage 4F icon source'; Path='test_stage4f_application_icon.ps1'; Args=@{Phase='Source'} },
    @{ Label='Stage 4F intro'; Path='test_stage4f_intro.ps1' },
    @{ Label='Stage 4F journey'; Path='test_stage4f_presentation_journey.ps1' },
    @{ Label='Stage 4G Grey Door'; Path='test_stage4g_grey_door.ps1' },
    @{ Label='Stage 4 final source'; Path='test_stage4_final_gate.ps1'; Args=@{Phase='Source'} }
)
$failures = [System.Collections.Generic.List[string]]::new()
$passed = 0
Push-Location $root
try {
    foreach($test in $tests) {
        $scriptPath = Join-Path $PSScriptRoot $test.Path
        $parameters = if($test.ContainsKey('Args')) { $test.Args } else { @{} }
        Write-Host ""
        Write-Host ("=== INDEPENDENT AUDIT: {0} ===" -f $test.Label)
        try {
            if(-not (Test-Path -LiteralPath $scriptPath -PathType Leaf)) {
                throw ("Regression script missing: {0}" -f $test.Path)
            }
            $global:LASTEXITCODE = 0
            & $scriptPath @parameters
            if($LASTEXITCODE -ne 0) {
                throw ("Exited with code {0}" -f $LASTEXITCODE)
            }
            ++$passed
            Write-Host ("AUDIT PASS: {0}" -f $test.Label)
        }
        catch {
            $message = "{0}: {1}" -f $test.Label,$_.Exception.Message
            $failures.Add($message)
            Write-Host ("AUDIT FAIL: {0}" -f $message)
        }
    }
}
finally {
    Pop-Location
}
Write-Host ""
Write-Host ("STAGE 4D+ INDEPENDENT AUDIT: {0}/{1} PASSED" -f $passed,$tests.Count)
foreach($message in $failures) {
    Write-Host (" - FAIL: {0}" -f $message)
}
if($failures.Count -gt 0) {
    throw ("Stage 4D+ audit has {0} failing regressions." -f $failures.Count)
}
Write-Host "STAGE 4D+ INDEPENDENT AUDIT: PASS"
