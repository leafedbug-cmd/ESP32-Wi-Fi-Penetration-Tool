@echo off
REM ESP32 Build and Flash Script
REM Downloads ESP-IDF and builds the project

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set IDF_PATH=%USERPROFILE%\esp-idf
set PROJECT_PATH=%SCRIPT_DIR%

echo.
echo ========================================
echo ESP32-S3 Build Script
echo ========================================
echo.

REM Check if ESP-IDF is installed
if not exist "%IDF_PATH%" (
    echo [1/3] Downloading ESP-IDF...
    cd /d "%USERPROFILE%"
    git clone --depth 1 --branch release/v5.0 https://github.com/espressif/esp-idf.git
    if errorlevel 1 (
        echo ERROR: Failed to clone ESP-IDF
        exit /b 1
    )
    echo Download complete
) else (
    echo [1/3] ESP-IDF already installed
)

echo.
echo [2/3] Installing ESP-IDF tools...
cd /d "%IDF_PATH%"
call install.bat all
if errorlevel 1 (
    echo WARNING: Some tools may have failed to install
)

echo.
echo [3/3] Building firmware...
cd /d "%PROJECT_PATH%"
call "%IDF_PATH%\export.bat"
idf.py.exe set-target esp32s3
idf.py.exe build

if errorlevel 1 (
    echo.
    echo ERROR: Build failed
    exit /b 1
)

echo.
echo ========================================
echo Build successful!
echo ========================================
echo.
echo Firmware: build\esp32s3\esp32s3.bin
echo.
echo To flash to COM3:
echo   idf.py.exe -p COM3 flash
echo.
echo To view logs:
echo   idf.py.exe -p COM3 monitor
echo.
