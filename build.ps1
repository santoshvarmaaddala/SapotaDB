# Paths
$projectRoot = "C:\Users\KITS\products\SapotaDB"
$srcPath = "$projectRoot\src"
$cliPath = "$srcPath\cli"
$enginePath = "$srcPath\engine"
$logPath = "$srcPath\log"
$txPath = "$srcPath\tx"
$storagePath = "$srcPath\storage"
$outputExe = "$projectRoot\sapotaDB.exe"

Write-Host "Compiling SapotaDB (MVCC)..."

g++ -std=c++17 -I "$srcPath" -fdiagnostics-color=always -g `
    "$cliPath\main.cpp" `
    "$enginePath\engine.cpp" `
    "$logPath\log_manager.cpp" `
    "$txPath/tx.cpp" `
    "$storagePath/mvcc.cpp" `
    -o "$outputExe"

if (Test-Path $outputExe) {
    Write-Host "✅ Compilation successful!"
    & $outputExe
} else {
    Write-Error "❌ Compilation failed. Check errors above."
}
