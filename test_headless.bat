@echo off
REM ===========================================
REM VrindaAI Headless Execution - Testing Guide
REM ===========================================

setlocal enabledelayedexpansion

echo.
echo ===========================================
echo VrindaAI Headless Execution Testing
echo ===========================================
echo.
echo The headless system is now fully operational.
echo This script will test the headless workflow.
echo.

cd /d "%~dp0"

REM Test 1: Check for executable
echo [TEST 1] Checking for VrindaAI.exe...
if exist VrindaAI.exe (
    echo   ✓ VrindaAI.exe found
) else (
    echo   ✗ VrindaAI.exe NOT FOUND
    echo   Please build the project first: cmake build; ninja
    pause
    exit /b 1
)

REM Test 2: Check for workflow files
echo.
echo [TEST 2] Checking for workflow files...
if exist cinematic_pipeline.json (
    echo   ✓ cinematic_pipeline.json found
) else (
    echo   ✗ cinematic_pipeline.json NOT FOUND
    pause
    exit /b 1
)

if exist jobs\blender_cinematic_job.json (
    echo   ✓ jobs\blender_cinematic_job.json found
) else (
    echo   ✗ jobs\blender_cinematic_job.json NOT FOUND
    pause
    exit /b 1
)

REM Test 3: Run headless workflow
echo.
echo [TEST 3] Running headless workflow...
echo   Command: VrindaAI.exe --headless --workflow cinematic_pipeline.json
echo.

del /f /q headless_execution.log 2>nul
VrindaAI.exe --headless --workflow cinematic_pipeline.json
set "EXITCODE=!ERRORLEVEL!"

echo.
echo Exit code: !EXITCODE!
echo.

REM Test 4: Show log file
echo [TEST 4] Execution log...
if exist headless_execution.log (
    echo.
    type headless_execution.log
) else (
    echo   No log file generated (app may have crashed)
)

echo.
echo ===========================================
echo Test Complete
echo ===========================================
echo.

if !EXITCODE! EQU 0 (
    echo ✓ SUCCESS: Workflow executed successfully!
    echo Check the 'output' directory for generated files.
) else (
    echo ✗ FAILURE: Workflow did not complete (exit code: !EXITCODE!)
    echo.
    echo Common causes:
    echo   - Blender is not installed
    echo   - Blender is not in the system PATH
    echo   - Asset files don't exist
    echo.
    echo To fix:
    echo   1. Install Blender (https://www.blender.org/download/)
    echo   2. Add Blender to your system PATH
    echo   3. Run this script again
)

echo.
pause
exit /b !EXITCODE!
