Write-Host ""
Write-Host "=== FLOPPY//144 PREVIOUS BUILD CLEANING ==="
Remove-Item .\build -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item .\obj -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item .\bin -Recurse -Force -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "=== FLOPPY//144 GIT DIFFERENCE CHECK ==="
git diff --check
.\run.ps1 -build release
