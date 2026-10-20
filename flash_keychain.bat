@echo off
setlocal
title SPEARHEAD // Firmware Flasher Studio
cd /d "%~dp0"

:: 1. Check local python in AppData
if exist "%LOCALAPPDATA%\Python\bin\python.exe" (
    "%LOCALAPPDATA%\Python\bin\python.exe" flash_tool.py %*
    goto :DONE
)

:: 2. Check PlatformIO Python
if exist "%USERPROFILE%\.platformio\penv\Scripts\python.exe" (
    "%USERPROFILE%\.platformio\penv\Scripts\python.exe" flash_tool.py %*
    goto :DONE
)

:: 3. Check py launcher
where py >nul 2>&1
if %ERRORLEVEL% equ 0 (
    py -3 flash_tool.py %*
    goto :DONE
)

:: 4. Fallback to python command
python flash_tool.py %*

:DONE
if %ERRORLEVEL% neq 0 (
    echo.
    echo Flasher exited with error code %ERRORLEVEL%.
    pause
)
