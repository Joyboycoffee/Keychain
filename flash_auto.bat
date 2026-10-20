@echo off
setlocal
title SPEARHEAD // Auto-Flash Utility
cd /d "%~dp0"

echo ========================================================
echo    SPEARHEAD // AUTO-DETECTING HARDWARE FLASHER
echo ========================================================
echo.

:: 1. Check local python in AppData
if exist "%LOCALAPPDATA%\Python\bin\python.exe" (
    "%LOCALAPPDATA%\Python\bin\python.exe" flash_tool.py --auto %*
    goto :DONE
)

:: 2. Check PlatformIO Python
if exist "%USERPROFILE%\.platformio\penv\Scripts\python.exe" (
    "%USERPROFILE%\.platformio\penv\Scripts\python.exe" flash_tool.py --auto %*
    goto :DONE
)

:: 3. Check py launcher
where py >nul 2>&1
if %ERRORLEVEL% equ 0 (
    py -3 flash_tool.py --auto %*
    goto :DONE
)

:: 4. Fallback to python command
python flash_tool.py --auto %*

:DONE
echo.
echo Operation finished.
pause
