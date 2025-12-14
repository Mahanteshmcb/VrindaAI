@echo off
setlocal enabledelayedexpansion

echo.
echo ========================================
echo VrindaAI - Headless Workflow Execution
echo ========================================
echo.

REM Get the current directory
set "BASEDIR=%CD%"
cd /d "%~dp0"

echo Current working directory: %CD%
echo.

REM Run the headless workflow and capture all output
echo Running: VrindaAI.exe --headless --workflow %1
echo.

VrindaAI.exe --headless --workflow %1
set "EXITCODE=!ERRORLEVEL!"

echo.
echo ========================================
echo Workflow completed with exit code: !EXITCODE!
echo ========================================
echo.

REM Check for output files
if exist output (
    echo.
    echo Output directory contents:
    dir /s output
) else (
    echo No output directory found.
)

pause
exit /b !EXITCODE!
