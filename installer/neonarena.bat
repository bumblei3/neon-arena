@echo off
REM NeonArena Launcher
REM Startet Quake3e mit dem NeonArena-Mod

set QUAKE_DIR=%~dp0
set QUAKE_EXE=%QUAKE_DIR%quake3e.exe

echo Starting NeonArena...
echo.

if not exist "%QUAKE_EXE%" (
    echo ERROR: quake3e.exe not found in %QUAKE_DIR%
    echo Please reinstall NeonArena.
    pause
    exit /b 1
)

REM Start Quake3e with NeonArena mod
REM +set fs_game neonarena — loads the mod
REM +set com_introdisabled 1 — skip intro video
REM +set r_mode -1 — custom resolution
REM +set r_customwidth 1920
REM +set r_customheight 1080

"%QUAKE_EXE%" +set fs_game neonarena +set com_introdisabled 1 +set r_mode -1 +set r_customwidth 1920 +set r_customheight 1080
