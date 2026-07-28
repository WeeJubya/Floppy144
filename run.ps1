param
(
    [ValidateSet("debug", "asan", "release")]
    [string]$build = "release",

    [switch]$compile_only
)

$ErrorActionPreference = "Stop"

New-Item `
    -ItemType Directory `
    -Force `
    .\obj, .\build, .\bin |
    Out-Null

Write-Host ""
Write-Host "Generating Floppy144 build..."
Write-Host ""

cmd /d /c "premake5 vs2022 && MSBuild build\river2D.sln /t:Floppy144 /p:Configuration=$build /p:Platform=windows /m"

exit $LASTEXITCODE

