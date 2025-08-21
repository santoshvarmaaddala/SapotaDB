# Paths
$projectRoot = "C:\Users\KITS\products\SapotaDB"
$srcPath = "$projectRoot\src"
$cliPath = "$srcPath\cli"
$enginePath = "$srcPath\engine"
$outputExe = "$cliPath\main.exe"
$logpath = "$srcPath\log"

# Compile SapotaDB
Write-Host "Compiling SapotaDB..."

g++ -I "$srcPath" -fdiagnostics-color=always -g `
    "$cliPath\main.cpp" `
    "$enginePath\engine.cpp" `
    "$logpath\log_manager.cpp" `
    -o "$outputExe"

# Check if compilation succeeded
if (Test-Path $outputExe) {
    Write-Host "✅ Compilation successful!"
    Write-Host "Running SapotaDB..."
    & $outputExe
} else {
    Write-Error "❌ Compilation failed. Check errors above."
}
