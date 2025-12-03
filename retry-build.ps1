# Quick build retry script
$projectPath = "c:\Users\atruett\esp32-wifi-pentest"
$pythonExe = "$projectPath\.venv\Scripts\python.exe"

Write-Host "Cleaning up failed downloads..." -ForegroundColor Yellow
Remove-Item -Path "$env:USERPROFILE\.platformio\packages\*" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "$env:USERPROFILE\.platformio\cache" -Recurse -Force -ErrorAction SilentlyContinue

Write-Host "Attempting build with timeout adjustment..." -ForegroundColor Yellow
cd $projectPath

# Set environment variables for longer timeouts
$env:PLATFORMIO_CONNECT_TIMEOUT = 120
$env:PLATFORMIO_UPLOAD_TIMEOUT = 180

Write-Host "Starting build..." -ForegroundColor Cyan
& $pythonExe -m platformio run

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "✓ Build successful!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Flash with: & '$pythonExe' -m platformio run -t upload" -ForegroundColor Yellow
} else {
    Write-Host ""
    Write-Host "Build failed. Run the ESP-IDF method instead:" -ForegroundColor Red
    Write-Host "  & 'c:\Users\atruett\esp32-wifi-pentest\build-espidf.ps1'" -ForegroundColor Yellow
}
