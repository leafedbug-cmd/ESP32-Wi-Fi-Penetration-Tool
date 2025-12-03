# Alternative build script using ESP-IDF directly
# This bypasses PlatformIO's problematic download mechanism

$ErrorActionPreference = "Stop"
$projectPath = "c:\Users\atruett\esp32-wifi-pentest"
$espIdfPath = "$env:USERPROFILE\esp-idf"
$pythonExe = "$projectPath\.venv\Scripts\python.exe"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ESP32 Firmware Build (ESP-IDF Method)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Download ESP-IDF if not present
if (!(Test-Path $espIdfPath)) {
    Write-Host "[1/3] Downloading ESP-IDF..." -ForegroundColor Yellow
    cd $env:USERPROFILE
    git clone --depth 1 --branch release/v5.0 https://github.com/espressif/esp-idf.git
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Failed to clone ESP-IDF" -ForegroundColor Red
        exit 1
    }
    Write-Host "✓ ESP-IDF downloaded" -ForegroundColor Green
} else {
    Write-Host "[1/3] ESP-IDF already present" -ForegroundColor Green
}

Write-Host ""

# Step 2: Install ESP-IDF tools
Write-Host "[2/3] Installing ESP-IDF tools..." -ForegroundColor Yellow
cd $espIdfPath
& python install.bat all 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "WARNING: Some tools may not have installed, but continuing..." -ForegroundColor Yellow
}
Write-Host "✓ ESP-IDF tools installed" -ForegroundColor Green

Write-Host ""

# Step 3: Build the project
Write-Host "[3/3] Building firmware..." -ForegroundColor Yellow
cd $projectPath
& "$espIdfPath\export.bat" | Out-Null
idf.py.exe set-target esp32s3
idf.py.exe build

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "✓ Build successful!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Firmware located at:" -ForegroundColor Cyan
    Write-Host "  $projectPath\build\esp32s3\esp32s3.bin" -ForegroundColor White
    Write-Host ""
    Write-Host "To flash to ESP32 on COM3:" -ForegroundColor Yellow
    Write-Host "  idf.py.exe -p COM3 flash" -ForegroundColor White
    Write-Host ""
    Write-Host "To view logs:" -ForegroundColor Yellow
    Write-Host "  idf.py.exe -p COM3 monitor" -ForegroundColor White
} else {
    Write-Host "ERROR: Build failed" -ForegroundColor Red
    exit 1
}
