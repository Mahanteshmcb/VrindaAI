@echo off
REM Launch VrindaAI from the build directory with all dependencies available
cd /d "%~dp0build\Desktop_Qt_6_9_1_MinGW_64_bit-Debug"
start VrindaAI.exe %*
