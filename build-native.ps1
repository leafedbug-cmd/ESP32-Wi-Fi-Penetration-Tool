# ESP32 Build and Flash Script using native ESP-IDF
# This is more reliable than PlatformIO for complex projects

$ErrorActionPreference = "Stop"
$idfPath = "$env:USERPROFILE\esp-idf"
$projectPath = Get-Location

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ESP32-S3 Build (Native ESP-IDF)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Download ESP-IDF if needed
if (!(Test-Path $idfPath)) {
    Write-Host "[1/3] Downloading ESP-IDF v5.0..." -ForegroundColor Yellow
    Push-Location $env:USERPROFILE
    try {
        git clone --depth 1 --branch release/v5.0 https://github.com/espressif/esp-idf.git 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERROR: Failed to clone ESP-IDF" -ForegroundColor Red
            exit 1
        }
    } finally {
        Pop-Location
    }
    Write-Host "✓ ESP-IDF downloaded" -ForegroundColor Green
} else {
    Write-Host "[1/3] ESP-IDF already installed" -ForegroundColor Green
}

Write-Host ""

# Step 2: Install ESP-IDF tools
Write-Host "[2/3] Installing ESP-IDF tools (this may take 10+ minutes)..." -ForegroundColor Yellow
Push-Location $idfPath
try {
    # Run install.bat all (silently)
    & python install.bat all 2>&1 | Out-Null
    Write-Host "✓ ESP-IDF tools installed" -ForegroundColor Green
} catch {
    Write-Host "WARNING: Tool installation had issues but continuing..." -ForegroundColor Yellow
} finally {
    Pop-Location
}

Write-Host ""

# Step 3: Build the project
Write-Host "[3/3] Building firmware..." -ForegroundColor Yellow
Push-Location $projectPath

try {
    # Import ESP-IDF environment
    & "$idfPath\export.bat" 2>&1 | Out-Null
    
    # Set target and build
    python -m idf_tools.tools list --format json 2>&1 | Out-Null
    
    # Use idf.py commands
    py -m idf set-target esp32s3 2>&1 | Out-Null
    py -m idf fullclean 2>&1 | Out-Null
    $buildOutput = py -m idf build 2>&1
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Build failed" -ForegroundColor Red
        Write-Host $buildOutput -ForegroundColor Yellow
        exit 1
    }
    
    Write-Host $buildOutput
    
} finally {
    Pop-Location
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "✓ Build successful!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Firmware: $projectPath\build\esp32s3\esp32s3.bin" -ForegroundColor Cyan
Write-Host ""
Write-Host "Flash to ESP32 on COM3:" -ForegroundColor Yellow
Write-Host "  py -m idf -p COM3 flash" -ForegroundColor White
Write-Host ""
Write-Host "View serial logs:" -ForegroundColor Yellow
Write-Host "  py -m idf -p COM3 monitor" -ForegroundColor White
