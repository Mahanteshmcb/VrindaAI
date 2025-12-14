@echo off
REM Run VrindaAI in headless mode with workflow
REM This script must be run from the VrindaAI project root directory

echo Starting VrindaAI headless workflow execution...
echo.

REM Run in headless mode
VrindaAI.exe --headless --workflow cinematic_pipeline.json

REM Check exit code
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Headless execution completed successfully!
    echo Check ./output directory for generated files.
) else (
    echo.
    echo Headless execution failed with error code %ERRORLEVEL%
    echo Please check the console output above for details.
)

pause
