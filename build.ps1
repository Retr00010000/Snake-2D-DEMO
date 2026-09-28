# Build and run script for Snake 2D using MinGW-w64 (GCC / g++)
param(
    [switch]$NoRun
)

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot

Write-Host "===> Building Snake 2D with GCC (g++)..." -ForegroundColor Cyan

# 1. Locate g++.exe
$gpp = (Get-Command g++ -ErrorAction SilentlyContinue).Source
if (-not $gpp) {
    $wingetCandidate = Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Packages" -Recurse -Filter "g++.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($wingetCandidate) {
        $gpp = $wingetCandidate.FullName
    }
}

if (-not $gpp) {
    Write-Host "Error: g++ compiler not found! Please ensure MinGW-w64 is installed." -ForegroundColor Red
    exit 1
}

Write-Host "Using compiler: $gpp" -ForegroundColor Green

# 2. Output directory setup
$outDir = Join-Path $root "Debug"
if (-not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir | Out-Null
}

# 3. Source files
$sources = @(
    "source/Snake.cpp",
    "source/engine/glad.c",
    "source/engine/EBO.cpp",
    "source/engine/ShaderClass.cpp",
    "source/engine/stb.cpp",
    "source/engine/Texture.cpp",
    "source/engine/VAO.cpp",
    "source/engine/VBO.cpp"
)

# 4. Compile with g++
$exe = Join-Path $outDir "Snake.exe"
$compileArgs = @(
    "-std=c++20",
    "-static",
    "-static-libgcc",
    "-static-libstdc++",
    "-Isource",
    "-Isource/engine",
    "-IDependencies/GLAD/include",
    "-IDependencies/GLFW/include",
    "-IDependencies/stb",
    "-LDependencies/GLFW/lib-mingw-w64",
    "-lglfw3",
    "-lopengl32",
    "-lgdi32",
    "-o", $exe
)

& $gpp @sources @compileArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit 1
}

# 5. Copy assets
if (Test-Path "$root\assets") {
    Copy-Item "$root\assets" -Destination "$outDir" -Recurse -Force
}

Write-Host "===> Build successful! Binary: $exe" -ForegroundColor Green

# 6. Run
if (-not $NoRun) {
    Write-Host "===> Launching Snake 2D..." -ForegroundColor Cyan
    Start-Process -FilePath $exe -WorkingDirectory $root
}
