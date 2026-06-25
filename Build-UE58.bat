@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%.") do set "PROJECT_DIR=%%~fI"
set "UPROJECT=%PROJECT_DIR%\FortRogue.uproject"
set "UE58_BUILD_BAT=F:\UE_5.8\Engine\Build\BatchFiles\Build.bat"
set "PROJECT_TARGET=FortRogueEditor"
set "PROJECT_PLATFORM=Win64"
set "PROJECT_CONFIG=DebugGame"

if not "%FR_UE58_BUILD_BAT%"=="" set "UE58_BUILD_BAT=%FR_UE58_BUILD_BAT%"
if not "%FR_UE_TARGET%"=="" set "PROJECT_TARGET=%FR_UE_TARGET%"
if not "%FR_UE_PLATFORM%"=="" set "PROJECT_PLATFORM=%FR_UE_PLATFORM%"
if not "%FR_UE_CONFIG%"=="" set "PROJECT_CONFIG=%FR_UE_CONFIG%"

if not exist "%UE58_BUILD_BAT%" (
    echo UE 5.8 Build.bat not found:
    echo "%UE58_BUILD_BAT%"
    exit /b 1
)

if not exist "%UPROJECT%" (
    echo Project file not found:
    echo "%UPROJECT%"
    exit /b 1
)

set "BUILD_TARGET=%PROJECT_TARGET% %PROJECT_PLATFORM% %PROJECT_CONFIG% -Project=""%UPROJECT%"""

set "COMMAND_MODE=build"
set "ALLOW_EDITOR_BUILD=0"
if /I "%~1"=="--raw" (
    set "COMMAND_MODE=raw"
    shift
)
if /I "%~1"=="--print" (
    set "COMMAND_MODE=print"
    shift
)

set "EXTRA_ARGS="
:CollectArgs
if "%~1"=="" goto ArgsCollected
if /I "%~1"=="--allow-editor" (
    set "ALLOW_EDITOR_BUILD=1"
    shift
    goto CollectArgs
)
set "EXTRA_ARGS=%EXTRA_ARGS% %1"
shift
goto CollectArgs

:ArgsCollected

if /I "%COMMAND_MODE%"=="raw" (
    call "%UE58_BUILD_BAT%" %EXTRA_ARGS%
    exit /b %ERRORLEVEL%
)

if /I "%COMMAND_MODE%"=="print" (
    echo "%UE58_BUILD_BAT%" -Target="%BUILD_TARGET%" -WaitMutex -FromMsBuild -architecture=x64%EXTRA_ARGS%
    exit /b 0
)

if "%ALLOW_EDITOR_BUILD%"=="0" (
    tasklist /FI "IMAGENAME eq UnrealEditor-Win64-DebugGame.exe" /NH | find /I "UnrealEditor-Win64-DebugGame.exe" >nul
    if not errorlevel 1 (
        echo Unreal Editor is running. Close it before running a full link build.
        echo This guard prevents DLL lock failures and protects the active editor session.
        echo Re-run with --allow-editor only when you intentionally want to bypass this guard.
        exit /b 3
    )

    tasklist /FI "IMAGENAME eq UnrealEditor.exe" /NH | find /I "UnrealEditor.exe" >nul
    if not errorlevel 1 (
        echo Unreal Editor is running. Close it before running a full link build.
        echo This guard prevents DLL lock failures and protects the active editor session.
        echo Re-run with --allow-editor only when you intentionally want to bypass this guard.
        exit /b 3
    )
)

rem Match the Visual Studio "DebugGame Editor | Win64" NMake build path by default.
call "%UE58_BUILD_BAT%" ^
    -Target="%BUILD_TARGET%" ^
    -WaitMutex -FromMsBuild -architecture=x64%EXTRA_ARGS%

exit /b %ERRORLEVEL%
