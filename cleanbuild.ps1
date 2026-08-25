Remove-Item .\build -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item .\obj -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item .\bin -Recurse -Force -ErrorAction SilentlyContinue

.\run.ps1 -build release
