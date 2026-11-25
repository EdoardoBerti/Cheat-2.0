@echo off
setlocal

echo ===================================================
echo   RONRUN - Setup & Build Script
echo ===================================================

cd /d "%~dp0"

:: 1. Download Dear ImGui if not present
if not exist "vendor\imgui\imgui.h" (
    echo [*] Cloning Dear ImGui into vendor/imgui...
    git clone --depth 1 https://github.com/ocornut/imgui.git vendor/imgui
    if errorlevel 1 (
        echo [!] Failed to clone ImGui. Make sure git is installed.
        pause
        exit /b 1
    )
    echo [+] ImGui cloned successfully.
) else (
    echo [+] ImGui already present.
)

:: 2. Create build directory
if not exist "build" (
    mkdir build
)

echo.
echo [*] Run CMake to generate the Visual Studio solution:
echo     cd build
echo     cmake .. -G "Visual Studio 17 2022" -A x64
echo     cmake --build . --config Release
echo.
echo ===================================================
pause
