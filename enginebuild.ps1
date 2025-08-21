# Compile SapotaDB

g++ -I "C:\Users\KITS\products\SapotaDB\src" -fdiagnostics-color=always -g `
    "C:\Users\KITS\products\SapotaDB\src\cli\main.cpp" `
    "C:\Users\KITS\products\SapotaDB\src\engine\engine.cpp" `
    -o "C:\Users\KITS\products\SapotaDB\src\cli\main.exe"

# Run the program after compilation

if (Test-Path "C:\Users\KITS\products\SapotaDB\src\cli\main.exe") {
    & "C:\Users\KITS\products\SapotaDB\src\cli\main.exe"
}