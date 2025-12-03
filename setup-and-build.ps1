# ESP32 Setup and Build Script
# This script downloads all necessary tools and builds the firmware

$ErrorActionPreference = "Stop"
$projectPath = "c:\Users\atruett\esp32-wifi-pentest"
$venvPath = "$projectPath\.venv\Scripts\python.exe"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ESP32 Firmware Build & Preparation" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Verify Python
Write-Host "[1/4] Checking Python installation..." -ForegroundColor Yellow
if (!(Test-Path $venvPath)) {
    Write-Host "ERROR: Virtual environment not found at $venvPath" -ForegroundColor Red
    exit 1
}
Write-Host "✓ Python found" -ForegroundColor Green

# Step 2: Install/Verify PlatformIO
Write-Host "[2/4] Installing PlatformIO..." -ForegroundColor Yellow
& $venvPath -m pip install --quiet --upgrade platformio
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to install PlatformIO" -ForegroundColor Red
    exit 1
}
Write-Host "✓ PlatformIO ready" -ForegroundColor Green

# Step 3: Download build tools and framework
Write-Host "[3/4] Downloading ESP-IDF framework and build tools..." -ForegroundColor Yellow
Write-Host "This may take 5-10 minutes on first run..." -ForegroundColor Cyan
cd $projectPath
& $venvPath -m platformio run --target clean 2>&1 | Out-Null
$buildOutput = & $venvPath -m platformio run 2>&1
Write-Host $buildOutput
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed" -ForegroundColor Red
    Write-Host "Full output:" -ForegroundColor Yellow
    Write-Host $buildOutput
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "✓ Build successful!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Next step: Flash to ESP32 on COM3" -ForegroundColor Cyan
Write-Host ""
Write-Host "Run this command to flash:" -ForegroundColor Yellow
Write-Host "  & '$venvPath' -m platformio run -t upload" -ForegroundColor White
Write-Host ""
Write-Host "To view serial output:" -ForegroundColor Yellow
Write-Host "  & '$venvPath' -m platformio device monitor" -ForegroundColor White
