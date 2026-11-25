@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo   RONRUN - Single Standalone Executable Build Script
echo ========================================================
echo.

cd /d "%~dp0"

:: 1. Check / Clone Dear ImGui
if not exist "vendor\imgui\imgui.h" (
    echo [*] Cloning Dear ImGui in vendor/imgui...
    git clone --depth 1 https://github.com/ocornut/imgui.git vendor\imgui
    if errorlevel 1 (
        echo [!] Git clone failed. Assicurati che git sia installato.
        pause
        exit /b 1
    )
    echo [+] ImGui clonato con successo.
) else (
    echo [+] Vendor ImGui presente.
)

:: 2. Find CMake and MSVC
set "CMAKE_BIN="
where cmake >nul 2>nul
if %errorlevel% equ 0 (
    set "CMAKE_BIN=cmake"
)

:: Check standard VS installations if cmake not in PATH
if not defined CMAKE_BIN (
    for %%p in (
        "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    ) do (
        if exist %%p (
            set "CMAKE_BIN=%%~p"
            goto :found_cmake
        )
    )
)

:found_cmake
if not defined CMAKE_BIN (
    echo.
    echo [!] CMake non trovato nel sistema.
    echo [*] Per compilare l'eseguibile unico:
    echo     1. Installa Visual Studio 2022 con 'Desktop development with C++'
    echo     2. Oppure installa CMake: https://cmake.org/download/
    echo.
    pause
    exit /b 1
)

echo [+] CMake trovato: "!CMAKE_BIN!"

:: 3. Configure CMake
echo.
echo [*] Configurazione CMake in corso...
if not exist "build" mkdir build

"!CMAKE_BIN!" -B build -S . -A x64
if errorlevel 1 (
    echo [!] Errore durante la generazione dei file di build CMake.
    pause
    exit /b 1
)

:: 4. Build Release
echo.
echo [*] Compilazione in corso (Release x64)...
"!CMAKE_BIN!" --build build --config Release
if errorlevel 1 (
    echo [!] Errore durante la compilazione.
    pause
    exit /b 1
)

:: 5. Copy executable as single root executable
if exist "build\Release\ronrun.exe" (
    copy /y "build\Release\ronrun.exe" "RONRUN.exe" >nul
    echo.
    echo ========================================================
    echo   [+] BUILD COMPLETATA CON SUCCESSO!
    echo   [+] Eseguibile unico generato: %~dp0RONRUN.exe
    echo ========================================================
    echo.
) else (
    echo [!] Eseguibile non trovato in build\Release\ronrun.exe
)

pause
