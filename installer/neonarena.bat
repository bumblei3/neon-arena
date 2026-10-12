@echo off
REM NeonArena Launcher (portable)
REM Starts Quake3e with the NeonArena mod. All paths are relative to
REM this script, so the folder can live anywhere (USB stick, Desktop).
REM The engine is patched with BASEGAME=baseoa, so baseoa content is
REM loaded from the baseoa\ folder next to the exe.

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

REM +set fs_game neonarena    - load the mod (pk3s in neonarena\)
REM +set com_basegame baseoa  - OpenArena base content (maps etc.)
REM +set fs_basepath          - engine dir (baseoa\ lives here)
REM +set fs_homepath          - keep config/screenshots inside the folder
REM +set com_introdisabled 1  - skip intro video
REM +set r_mode -1            - custom resolution (1920x1080 default)

"%QUAKE_EXE%" +set fs_game neonarena +set com_basegame baseoa ^
    +set fs_basepath "%QUAKE_DIR%" +set fs_homepath "%QUAKE_DIR%" ^
    +set com_introdisabled 1 +set r_mode -1 ^
    +set r_customwidth 1920 +set r_customheight 1080
