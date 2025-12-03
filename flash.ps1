# Flash to ESP32 on COM3
$ErrorActionPreference = "Stop"
$projectPath = "c:\Users\atruett\esp32-wifi-pentest"
$venvPath = "$projectPath\.venv\Scripts\python.exe"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Flashing ESP32 on COM3" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

cd $projectPath
Write-Host "Starting upload..." -ForegroundColor Yellow
& $venvPath -m platformio run -t upload

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "✓ Flash successful!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Viewing serial output (press Ctrl+C to exit):" -ForegroundColor Cyan
    & $venvPath -m platformio device monitor
} else {
    Write-Host "ERROR: Flash failed" -ForegroundColor Red
    exit 1
}
