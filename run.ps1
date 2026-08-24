param
(
    [ValidateSet("debug", "asan", "release")]
    [string]$build = "release"
)

& {
    $ErrorActionPreference = "Stop"
    Set-StrictMode -Version Latest

    $maximumBytes = 1474560
    $solutionPath = ".\build\Floppy144.sln"
    $releaseExe = ".\bin\release\Floppy144.exe"

    New-Item `
        -ItemType Directory `
        -Force `
        .\obj, .\build, .\bin |
        Out-Null

    Write-Host ""
    Write-Host "Generating Floppy144 build..."
    Write-Host ""

    & premake5 vs2022

    if ($LASTEXITCODE -ne 0)
    {
        throw "Premake generation failed with exit code $LASTEXITCODE."
    }

    & MSBuild `
        $solutionPath `
        /t:Floppy144 `
        /p:Configuration=$build `
        /p:Platform=windows `
        /m

    if ($LASTEXITCODE -ne 0)
    {
        throw "MSBuild failed with exit code $LASTEXITCODE."
    }

    if ($build -eq "release")
    {
        if (-not (Test-Path -LiteralPath $releaseExe))
        {
            throw "Release executable was not produced: $releaseExe"
        }

        $usedBytes = (Get-Item -LiteralPath $releaseExe).Length
        $remainingBytes = $maximumBytes - $usedBytes

        Write-Host ""
        Write-Host "FLOPPY//144 RELEASE SIZE"
        Write-Host ""
        Write-Host ("Used:      {0:N0} bytes" -f $usedBytes)
        Write-Host ("Remaining: {0:N0} bytes" -f $remainingBytes)
        Write-Host ("Maximum:   {0:N0} bytes" -f $maximumBytes)
        Write-Host ""

        if ($usedBytes -gt $maximumBytes)
        {
            Write-Host "SIZE GATE: FAIL"
            throw "Floppy//144 exceeds the 1.44 MB submission limit by $(-$remainingBytes) bytes."
        }

        Write-Host "SIZE GATE: PASS"
    }
}
