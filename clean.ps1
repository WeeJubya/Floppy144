$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$toDelete = @(
    ".\build",
    ".\bin",
    ".\obj",
    ".\log"
)

Write-Host "Cleaning generated build output..."
foreach($folder in $toDelete) {
    if(Test-Path $folder) {
        Remove-Item $folder -Recurse -Force
    }
}
Write-Host "Clean complete." -ForegroundColor Green
